# Target Localization Module using GPS, IMU and LoRa
An embedded system that estimates target coordinates based on measured distance and current device position. The system integrates GPS, IMU and LoRa communication for remote localization.

## Key Features

- **Precision Navigation:** Implementing Karney’s Algorithm to solve the inverse geodesic problem, providing highly accurate distance and azimuth calculations on an ellipsoidal Earth model.
- **Advanced Sensor Fusion:** Utilizing an Extended Kalman Filter (UKF) and a Kalman Filter (KF) to fuse GPS and 10-axis IMU data. This enables real-time determination of observation posture (attitude) and precise target localization.
- **High-Performance I/O:** Optimizing communication peripherals (UART/SPI) with DMA (Direct Memory Access) and interrupts to minimize CPU overhead and ensure stable, high-frequency sensor sampling.

## Hardware
- MCU: STM32F411CEU6
- GPS module LC76GAB
- IMU module 10 axis
- LoRa module SX1278
- ST-Link debugger

## Pin Mapping

### STM32 (Tracker Node)

#### LCD (SPI1)

| Device pin  | STM32 pin | Description        |
|-------------|-----------|--------------------|
| LCD_SPI SDA | PA7       | SPI MOSI           |
| LCD_SPI SCL | PA5       | SPI clock          |
| LCD_SPI CS  | PA4       | Chip select        |
| LCD_RESET   | PB0       | Reset              |
| LCD_DC      | PB1       | Data/Command       |

#### GPS (UART2)

| Device pin | STM32 pin |
|------------|-----------|
| GPS_TX     | PA3       |
| GPS_RX     | PA2       |

#### IMU (I2C)

| Device pin   | STM32 pin |
|--------------|-----------|
| IMU_I2C SCL  | PB10      |
| IMU_I2C SDA  | PB9       |

#### Power Monitor

| Device pin | STM32 pin |
|------------|-----------|
| POWER ADC  | PA1       |

#### LoRa (SPI)

| LoRa pin | STM32 pin | Description                 |
|----------|-----------|-----------------------------|
| MOSI     | PB5       | Data Master → Slave         |
| MISO     | PB4       | Data Slave → Master         |
| SCK      | PB3       | SPI clock                   |
| NSS      | PA15      | Chip select                 |
| RESET    | PA10      | Reset module                |

### Raspberry Pi 5 ↔ LoRa

| RPi 5 pin | RPi 5 GPIO | LoRa pin | Description                  |
|-----------|------------|----------|------------------------------|
| Pin 19    | MOSI       | MOSI     | Data Master → Slave          |
| Pin 21    | MISO       | MISO     | Data Slave → Master          |
| Pin 23    | SCLK       | SCK      | SPI clock                    |
| Pin 24    | CE0        | NSS      | Chip select                  |
| Pin 15    | GPIO22     | RESET    | Reset module                 |
| Pin 13    | GPIO27     | DIO0     | Interrupt on data received   |

## Installation

### Firmware

Clone the repository:

```bash
git clone https://github.com/bachbeastCE/DACN_HK251.git
```

Open the project in **STM32CubeIDE**:

1. Launch STM32CubeIDE
2. Select **File → Import → Existing Projects into Workspace**
3. Choose the cloned repository folder
4. Build the project

### Hardware Design (Altium)

PCB and schematic files designed in **Altium Designer** are located in:

https://github.com/bachbeastCE/DACN_HK251/tree/PCB

To open:

1. Launch Altium Designer
2. Open the `.PrjPcb` file in the `PCB` directory
3. View schematic and PCB layout

## Build
1. Open STM32CubeIDE
2. Import existing project
3. Build project

## Flash
Connect ST-Link and click Run → Debug or Run on STM32CubeProgrammer

## Test

Detailed testing procedures and evaluation results are described in
[**REPORT_GD1**](./REPORT_GD1.pdf), Chapter 5: *System Implementation and Evaluation*.

## Documentation

The complete project documentation is available at:
[**HK252_1.143_LVTN_DATN_Finish_2210022_2210185**](./HK252_1.143_LVTN_DATN_Finish_2210022_2210185)

