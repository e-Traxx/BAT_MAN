# BAT-MAN 🦇🔋

## Battery Management System – Formula Student

Quick setup guide for integrating a **Battery Management System (BMS)** using ADBMS6830 (Robin Slaves), LTC6820 as isoSPI translator, and ESP32-S3 as Master MCU.

## 🚀 Quick Start

### Hardware Components:
- **MCU:** ESP32-S3 (Master)
- **isoSPI Translator:** LTC6820
- **BMS Slaves:** ADBMS6830 (Robin Modules)

### Connections:
- **ESP32-S3 ↔ LTC6820:** SPI interface
- **LTC6820 ↔ Robin Modules:** isoSPI communication (twisted pair cables)

### Software Requirements:
- ESP-IDF environment
- SPI and isoSPI driver implementation

## 🔌 System Setup
1. Configure ESP32-S3 SPI interface.
2. Set up LTC6820 for isoSPI communication.
3. Initialize and configure Robin (ADBMS6830) modules.
4. Establish full-duplex isoSPI data exchange between Master and Robin slaves.

## 🛠 Troubleshooting
- **isoSPI issues:** Verify cable integrity and correct termination resistors (100Ω differential).
- **Communication failures:** Ensure correct SPI timing and polarity.
- **Slave response problems:** Confirm wake-up signals and module addressing.

## 💡 Best Practices
- Regularly validate communication and balancing routines.
- Use clear logging for diagnostics.
- Document hardware and software revisions meticulously.

## 📞 Need Support?
Contact your Electrical Lead / Ashitosh Ubdhoot or refer to datasheets:
- [ADBMS6830 Datasheet](https://www.analog.com/)
- [LTC6820 Datasheet](https://www.analog.com/)
- [ESP32-S3 Documentation](https://docs.espressif.com/projects/esp-idf/)

---

© BAT-MAN 🦇 
@ASH is voll krass
