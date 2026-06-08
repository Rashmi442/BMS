Hardware Used: STM32F411RE Nucleo, ACS712 Current Sensor, Voltage Divider Circuit, 16×2 I2C LCD, Lithium-Ion Battery, Passive Components (Resistors/Capacitors).


IoT Battery Monitoring System using STM32

This project is a battery monitoring system developed using the STM32F411RE Nucleo board to measure and display important battery parameters in real time. The system uses the microcontroller's built-in 12-bit ADC to acquire analog signals from a voltage divider circuit for battery voltage measurement and an ACS712 current sensor for current measurement. The measured values are processed by the STM32 to calculate the actual battery voltage, load current, and an approximate battery charge percentage, which are then displayed on a 16×2 I2C LCD module.

The firmware is written in Embedded C using the STM32 HAL libraries and developed in STM32CubeIDE. The project focuses on learning and implementing fundamental embedded concepts such as ADC configuration, sensor interfacing, I2C communication, data processing, and peripheral initialization. It provides a simple and low-cost approach for monitoring Lithium-Ion battery parameters in small embedded and IoT applications.

At present, the system performs local monitoring through the LCD interface. Future improvements include integrating an ESP-01 (ESP8266) Wi-Fi module for cloud connectivity, enabling remote battery monitoring through IoT platforms such as Blynk, adding low-battery alert notifications, and incorporating temperature sensing and advanced battery health estimation techniques.

This repository is intended to document my practical learning and implementation of STM32-based embedded systems and will continue to be updated as new features are added.
