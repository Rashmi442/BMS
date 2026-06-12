🔋 Battery Monitoring System using STM32F411RE

A bare-metal embedded application for real-time monitoring of Lithium-Ion battery parameters using the STM32F411RE Nucleo board. The system measures battery voltage and load current through the STM32's internal ADC, estimates battery state of charge, and displays the results on a 16×2 I2C LCD.

📖 Overview

This project was developed to explore low-level peripheral programming on STM32 microcontrollers without relying on vendor-provided abstraction libraries. ADC, GPIO, clock configuration, and peripheral control are implemented through direct register manipulation, providing a deeper understanding of the STM32F4 architecture.

The system continuously acquires sensor data, processes the measurements, and provides real-time feedback through an LCD interface.

✨ Features

* Real-time battery voltage monitoring
* Current measurement using the ACS712 current sensor
* Battery percentage estimation
* 16×2 I2C LCD display interface
* 12-bit ADC-based signal acquisition
* Bare-metal register-level programming
* Lightweight and low-overhead firmware

🛠️ Hardware Used

* STM32F411RE Nucleo Board
* ACS712 Current Sensor
* Voltage Divider Circuit
* 16×2 I2C LCD Module
* Lithium-Ion Battery
* Passive Components (Resistors and Capacitors)

💻 Software & Development Tools

* Embedded C
* STM32CubeIDE
* STM32F411RE Reference Manual
* STM32F411RE Datasheet

🏗️ System Architecture
Battery
   │
   ├── Voltage Divider ──► ADC
   │
   └── ACS712 Sensor ────► ADC
                             │
                             ▼
                      STM32F411RE
                             │
                             ▼
                      Data Processing
                             │
                             ▼
                        I2C LCD


📊 Parameters Monitored

| Parameter    | Description                          |
| ------------ | ------------------------------------ |
| Voltage      | Battery terminal voltage             |
| Current      | Load current measured through ACS712 |
| Charge Level | Estimated battery percentage         |

 ⚙️ Implementation Highlights

* Direct register-level configuration of STM32 peripherals
* ADC-based acquisition of voltage and current signals
* Conversion of raw ADC values into engineering units
* Real-time LCD updates using I2C communication
* Modular firmware structure for future enhancements
* Bare-metal approach without STM32 HAL libraries

📚 Learning Outcomes

Through this project, I gained practical experience in:

* STM32F4 microcontroller architecture
* Bare-metal embedded C programming
* ADC configuration and analog signal acquisition
* I2C communication protocol
* Sensor interfacing and calibration
* Register-level peripheral control
* Embedded firmware debugging
* Real-time data processing
* Hardware-software integration

🚀 Future Enhancements

* ESP8266 (ESP-01) Wi-Fi integration
* Cloud-based monitoring dashboard
* MQTT/Blynk connectivity
* Low-battery alert notifications
* Battery temperature monitoring
* Data logging and analytics
* Advanced battery health estimation

📷 Project Demonstration

👨‍💻 Author

Rashmi R
Electronics and Communication Engineering
Interested in Embedded Systems, Firmware Development, STM32 Microcontrollers, and IoT Technologies.
