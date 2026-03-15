# Temperature Sensor System using AVR128DB48

## Overview
This project demonstrates a **multi-protocol temperature sensing system** implemented on the **AVR128DB48 microcontroller**. The system measures temperature using three different communication interfaces: **USART, SPI, and I²C**, each paired with a different temperature sensor and implementation approach.

The project highlights low-level embedded development techniques including **direct register configuration, peripheral control, and mixed-language programming using both AVR assembly and C**. Each communication protocol operates independently and demonstrates how the microcontroller interacts with different types of sensors (analog and digital).

The three implementations included in this project are:

- **USART + ADC (AVR Assembly)**
- **SPI (C Implementation)**
- **I²C (C Implementation)**

Together, these modules demonstrate how multiple communication peripherals on the AVR128DB48 can be used for **high-speed temperature acquisition and sensor interfacing**.

---

## USART Temperature Sensor (AVR Assembly)

This implementation reads temperature from an **MCP9700A analog temperature sensor** using the **on-board ADC** of the AVR128DB48. The firmware is written entirely in **AVR assembly** to demonstrate low-level hardware control and efficient peripheral configuration.

### Operation
1. The **MCP9700A** outputs an analog voltage proportional to temperature.
2. The **AVR ADC module** samples this voltage and converts it into a digital value.
3. The converted temperature data is transmitted using the **USART peripheral**.
4. Serial output allows high-speed monitoring or logging of temperature data.

### Key Features
- Pure **AVR assembly implementation**
- Direct **ADC register configuration**
- **USART serial transmission** of sensor data
- Demonstrates **analog sensor integration**

---

## SPI Temperature Sensor (C Implementation)

This module interfaces with an **LM74 digital temperature sensor** using the **SPI hardware module** of the AVR128DB48.

The firmware is written in **C** and uses the microcontroller as an **SPI master** to retrieve temperature data from the LM74 sensor.

### Operation
1. The AVR128DB48 initializes the **SPI peripheral in master mode**.
2. A read transaction is initiated with the **LM74 sensor**.
3. The sensor returns the current temperature as a digital value.
4. The microcontroller processes the received data.

### Key Features
- **Hardware SPI communication**
- **Master–slave sensor interface**
- Efficient **digital temperature acquisition**
- Implemented in **embedded C**

---

## I²C Temperature Sensor (C Implementation)

This implementation communicates with an **LM75 temperature sensor** using the **I²C (TWI) peripheral** of the AVR128DB48.

The structure is similar to the SPI module but uses the **I²C protocol** to read temperature registers from the sensor.

### Operation
1. The AVR initializes the **I²C (TWI) hardware module**.
2. The microcontroller sends a **read request to the LM75 sensor**.
3. The sensor returns temperature data stored in its internal registers.
4. The AVR reads and processes the data.

### Key Features
- **I²C/TWI hardware interface**
- Communication with **LM75 digital temperature sensor**
- **Register-based temperature readout**
- Implemented in **embedded C**
