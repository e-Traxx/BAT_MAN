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

static const char *TAG = "ISO_SPI";
spi_device_handle_t spi_cs1, spi_cs2;
const uint16_t crc15Table[256];

#define DATA_PEC_POLY 0x22D
#define DATA_PEC_INIT 0x0008

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
						 .spics_io_num = -1,
						 .queue_size = 7,
						 .flags = 0 };

    ESP_ERROR_CHECK (spi_bus_add_device (SPI_HOST_D, &devcfg_cs1, &spi_cs1));
    ESP_LOGI (TAG, "CS1 flags = 0x%08x", ((unsigned)devcfg_cs1.flags));

    // Set CS High (active = Low)
    gpio_set_level (SPI_CS1, 1);
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
 *
 * The ADBMS uses a CRC15 algorithm for calculating a 16-Bit PEC,
 * The last bit is 0 and ignored in the 16-Bit
 */
uint16_t
pec15_calc (const uint8_t *data, uint8_t len)
{
    const uint16_t crc15Table[256] = {
	0x0000, 0xC599, 0xCEAB, 0x0B32, 0xD8CF, 0x1D56, 0x1664, 0xD3FD, 0xF407, 0x319E, 0x3AAC, 0xFF35, 0x2CC8, 0xE951,
	0xE263, 0x27FA, 0xAD97, 0x680E, 0x633C, 0xA6A5, 0x7558, 0xB0C1, 0xBBF3, 0x7E6A, 0x5990, 0x9C09, 0x973B, 0x52A2,
	0x815F, 0x44C6, 0x4FF4, 0x8A6D, 0x5B2E, 0x9EB7, 0x9585, 0x501C, 0x83E1, 0x4678, 0x4D4A, 0x88D3, 0xAF29, 0x6AB0,
	0x6182, 0xA41B, 0x77E6, 0xB27F, 0xB94D, 0x7CD4, 0xF6B9, 0x3320, 0x3812, 0xFD8B, 0x2E76, 0xEBEF, 0xE0DD, 0x2544,
	0x02BE, 0xC727, 0xCC15, 0x098C, 0xDA71, 0x1FE8, 0x14DA, 0xD143, 0xF3C5, 0x365C, 0x3D6E, 0xF8F7, 0x2B0A, 0xEE93,
	0xE5A1, 0x2038, 0x07C2, 0xC25B, 0xC969, 0x0CF0, 0xDF0D, 0x1A94, 0x11A6, 0xD43F, 0x5E52, 0x9BCB, 0x90F9, 0x5560,
	0x869D, 0x4304, 0x4836, 0x8DAF, 0xAA55, 0x6FCC, 0x64FE, 0xA167, 0x729A, 0xB703, 0xBC31, 0x79A8, 0xA8EB, 0x6D72,
	0x6640, 0xA3D9, 0x7024, 0xB5BD, 0xBE8F, 0x7B16, 0x5CEC, 0x9975, 0x9247, 0x57DE, 0x8423, 0x41BA, 0x4A88, 0x8F11,
	0x057C, 0xC0E5, 0xCBD7, 0x0E4E, 0xDDB3, 0x182A, 0x1318, 0xD681, 0xF17B, 0x34E2, 0x3FD0, 0xFA49, 0x29B4, 0xEC2D,
	0xE71F, 0x2286, 0xA213, 0x678A, 0x6CB8, 0xA921, 0x7ADC, 0xBF45, 0xB477, 0x71EE, 0x5614, 0x938D, 0x98BF, 0x5D26,
	0x8EDB, 0x4B42, 0x4070, 0x85E9, 0x0F84, 0xCA1D, 0xC12F, 0x04B6, 0xD74B, 0x12D2, 0x19E0, 0xDC79, 0xFB83, 0x3E1A,
	0x3528, 0xF0B1, 0x234C, 0xE6D5, 0xEDE7, 0x287E, 0xF93D, 0x3CA4, 0x3796, 0xF20F, 0x21F2, 0xE46B, 0xEF59, 0x2AC0,
	0x0D3A, 0xC8A3, 0xC391, 0x0608, 0xD5F5, 0x106C, 0x1B5E, 0xDEC7, 0x54AA, 0x9133, 0x9A01, 0x5F98, 0x8C65, 0x49FC,
	0x42CE, 0x8757, 0xA0AD, 0x6534, 0x6E06, 0xAB9F, 0x7862, 0xBDFB, 0xB6C9, 0x7350, 0x51D6, 0x944F, 0x9F7D, 0x5AE4,
	0x8919, 0x4C80, 0x47B2, 0x822B, 0xA5D1, 0x6048, 0x6B7A, 0xAEE3, 0x7D1E, 0xB887, 0xB3B5, 0x762C, 0xFC41, 0x39D8,
	0x32EA, 0xF773, 0x248E, 0xE117, 0xEA25, 0x2FBC, 0x0846, 0xCDDF, 0xC6ED, 0x0374, 0xD089, 0x1510, 0x1E22, 0xDBBB,
	0x0AF8, 0xCF61, 0xC453, 0x01CA, 0xD237, 0x17AE, 0x1C9C, 0xD905, 0xFEFF, 0x3B66, 0x3054, 0xF5CD, 0x2630, 0xE3A9,
	0xE89B, 0x2D02, 0xA76F, 0x62F6, 0x69C4, 0xAC5D, 0x7FA0, 0xBA39, 0xB10B, 0x7492, 0x5368, 0x96F1, 0x9DC3, 0x585A,
	0x8BA7, 0x4E3E, 0x450C, 0x8095,
    };

    uint16_t remainder = 16; // initialize the PEC
    uint16_t addr;

    /* loops for each byte in data array */
    for (uint8_t i = 0; i != len; i++)
	{
	    /* calculate PEC table address */
	    addr = ((remainder >> 7) ^ data[i]) & 0xff;
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

    gpio_set_level (SPI_CS1, 0);
    uint8_t wake_signal[] = { 0xFF, 0x22 };
    spi_transaction_t wake = {
	.length = sizeof (wake_signal) * 8,
	.tx_buffer = &wake_signal,
    };

    /*
     * For about 2ms, continously send the wake up signal (64 times in total)
     */
    for (int i = 0; i < 64; i++)
	{
	    spi_device_transmit (dev, &wake);
	}

    // release CS
    // gpio_set_level (SPI_CS1, 1);
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

    // CS Active
    gpio_set_level (SPI_CS1, 0);

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

    // CS Inactive
    gpio_set_level (SPI_CS1, 0);

    return err;
}
