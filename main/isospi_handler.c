#include "Robin_handler.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "hal/spi_types.h"
#include "string.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "spi_handler.h"

#define SPI_HOST_D SPI2_HOST
#define MISO_NUM 11
#define MOSI_NUM 12
#define CLK_NUM 10
#define SPI_CS1 13
#define SPI_CS2 9

#define DATA_PEC_POLY 0x22D
#define DATA_PEC_INIT 0x0008
#define COMMAND_PEC_POLY 0b110001011001101
#define COMMAND_PEC_INIT 0b0110001011001101

static const char *TAG = "ISO_SPI";
spi_device_handle_t spi_cs1, spi_cs2;
const uint16_t crc15Table[256];

void
ISOSPI_Setup ()
{
    esp_err_t err;

    spi_bus_config_t bus_conf = { .miso_io_num = MISO_NUM,
				  .mosi_io_num = MOSI_NUM,
				  .sclk_io_num = CLK_NUM,
				  .quadhd_io_num = -1,
				  .quadwp_io_num = -1,
				  .max_transfer_sz = 4096 };

    err = spi_bus_initialize (SPI_HOST_D, &bus_conf, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
	ESP_LOGE (TAG, "[-] Failed to Initialise SPI");

    spi_device_interface_config_t devcfg_cs1 = { .clock_speed_hz = 5 * 1000 * 100, // 500KHz
						 .mode = 3,
						 .spics_io_num = SPI_CS1,
						 .queue_size = 7,
						 .flags = 0 };

    ESP_ERROR_CHECK (spi_bus_add_device (SPI_HOST_D, &devcfg_cs1, &spi_cs1));
    ESP_LOGI (TAG, "CS1 flags = 0x%08x", ((unsigned)devcfg_cs1.flags));

    ESP_LOGI (TAG, "SPI Sender and Receiver are Initilialised");
};

/*
 *
 *
 *
 *
 *
 *
 */

/* ---- task-context hex dump (uses snprintf, OK outside ISRs) ---- */
static void
print_hex (const char *tag, const uint8_t *buf, size_t len)
{
    char line[3 * 16 + 1]; // “AA ” × 16 + NUL
    size_t idx = 0;

    for (size_t i = 0; i < len; ++i)
	{
	    idx += snprintf (&line[idx], sizeof (line) - idx, "%02X ", buf[i]);

	    if ((i & 0x0F) == 0x0F || i == len - 1)
		{ // wrap every 16 bytes
		    line[idx] = '\0';
		    ESP_LOGI (tag, "%s", line);
		    idx = 0; // reset line buffer
		}
	}
}

/* ---- ISR-safe hex dump (prints byte-by-byte from ROM) ---------- */
IRAM_ATTR static void
print_hex_isr (const char *tag, const uint8_t *buf, size_t len)
{
    esp_rom_printf ("%s: ", tag);
    for (size_t i = 0; i < len; ++i)
	{
	    esp_rom_printf ("%02X ", buf[i]);
	    if ((i & 0x0F) == 0x0F)
		esp_rom_printf ("\n%s: ", tag);
	}
    esp_rom_printf ("\n");
} /////////////////////////////////////////////

/*
 * The Data PEC is a 10 bit Polynomial Error Check placed after every register data packet.
 *
 */

uint16_t
Compute_Data_PEC (const uint8_t *data, size_t response_len)
{
    // Remainder uses bits [9:0]. We will store it in a 16-bit variable
    uint16_t rem = DATA_PEC_INIT; // 15-bit register

    for (size_t i = 0; i < response_len; ++i)
	{
	    rem ^= (uint16_t)data[i] << 7; // align byte to MSB-1

	    for (uint8_t bit = 0; bit < 8; ++bit)
		{
		    if (rem & 0x4000) // test bit 14
			rem = (rem << 1) ^ DATA_PEC_POLY;
		    else
			rem <<= 1;

		    rem &= 0x7FFF; // keep to 15 bits
		}
	}
    return rem;
}

/*
 * using a lookup table for the PEC 15-Bit
 */
uint16_t
pec15_calc (const uint8_t *data, size_t len)
{
    uint16_t remainder, addr;
    remainder = 16; // initialise PEC

    for (uint8_t i = 0; i < len; i++)
	{
	    addr = ((remainder >> 7) ^ data[i]) & 0xFF; // calculate PEC table
	    remainder = (remainder << 8) ^ crc15Table[addr];
	}

    return (remainder * 2);
}

/*
 * ISO SPI Controllers
 *
 * How it works:
 *
 * - Send a wake up signal:
 *      This wake up signal is just swinging the CS and clocking dummy bytes showing activity on the connection.
 *      This will wake up the ADBMS and allow the MCU to send Commands to the Module.
 *      The Module will fall asleep after 2 - 5ms of inactivity on the connection.
 *
 *  - Send Command:
 *      The Comand Sending comprises of 2 phases namely:
 *          TX PHASE:
 *            The MCU clocks out the command including the PEC.
 *            The CS is kept low after the transaction using the flag.
 *
 *          RX PHASE:
 *            The CS is kept low from the previous transaction.
 *            The MCU clocks out the amount amount of bytes it should be receiving.
 *            The number of bytes it should be receiving is based on the number of Stacks present,
 *
 */

void
wake_tone (spi_device_handle_t dev)
{
    esp_err_t err;

    /*
     * Generate a signal of alternating values to make sure that there is enough swinging of the data lines to wake up
     * the module
     *
     * The wake_signal at 500KHz only takes up (16-Bits / 500KHz) 32us, which is not enough to wake up the module.
     * Keep CS low and blast the wake tone until t >
     */

    uint8_t wake_signal[] = { 0xFF, 0x22 };
    spi_transaction_t wake = {
	.length = sizeof (wake_signal) * 8,
	.tx_buffer = &wake_signal,
    };

    /*
     * For 240us, continously send the wake up signal (8 times in total)
     */
    for (int i = 0; i < 8; i++)
	{
	    spi_device_polling_transmit (dev, &wake);
	}
    esp_rom_delay_us (10);
}

/*
 * This separates the transaction into 2 different parts.
 */
esp_err_t
isoSPI_tx_rx (const spi_device_handle_t dev, const uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len)
{
    esp_err_t err;

    /*
     * Wake up signal
     */
    spi_device_acquire_bus (dev, portMAX_DELAY); // lock bus for this transaction

    wake_tone (dev);

    /*
     * PHASE 1: TX Command
     * Clocks out (tx_len * 8) command in Bytes
     */
    spi_transaction_t t_cmd
	= { .length = tx_len * 8, .tx_buffer = tx_data, .rxlength = 0, .flags = SPI_TRANS_CS_KEEP_ACTIVE };
    err = spi_device_polling_transmit (dev, &t_cmd);

    ESP_ERROR_CHECK (err);

    if (rx_len > 0)
	{
	    /*
	     * PHASE 2: RX Command
	     * Clocks out (rx_len * 8) command in Bytes
	     * Awaits the arrival of RX_data and collects them in the rx_buffer.
	     */
	    spi_transaction_t t_rx = { .length = rx_len * 8, .rx_buffer = rx_data, .rxlength = rx_len * 8 };
	    err = spi_device_polling_transmit (dev, &t_rx);
	    print_hex ("RX", rx_data, rx_len);
	}

    spi_device_release_bus (dev); // Release the Bus

    return err;
}

// CRC lookup table
const uint16_t crc15Table[256] = {
    0x0,    0xc599, 0xceab, 0xb32,  0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, // precomputed CRC15 Table
    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990,
    0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a,
    0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76,
    0xebef, 0xe0dd, 0x2544, 0x2be,  0xc727, 0xcc15, 0x98c,  0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e,
    0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2,  0xc25b, 0xc969, 0xcf0,  0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52,
    0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31,
    0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423,
    0x41ba, 0x4a88, 0x8f11, 0x57c,  0xc0e5, 0xcbd7, 0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0,
    0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614,
    0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9, 0xf84,  0xca1d, 0xc12f, 0x4b6,  0xd74b, 0x12d2, 0x19e0,
    0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2,
    0xe46b, 0xef59, 0x2ac0, 0xd3a,  0xc8a3, 0xc391, 0x608,  0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
    0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350, 0x51d6,
    0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5,
    0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x846,  0xcddf, 0xc6ed, 0x374,  0xd089,
    0x1510, 0x1e22, 0xdbbb, 0xaf8,  0xcf61, 0xc453, 0x1ca,  0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054,
    0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368,
    0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
};
