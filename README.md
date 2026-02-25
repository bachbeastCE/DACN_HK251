# Target Localization Module using GPS, IMU and LoRa
An embedded system that estimates target coordinates based on measured distance and current device position. The system integrates GPS, IMU and LoRa communication for remote localization.

##Key Features
Precision Navigation: Implementing Karney’s Algorithm to solve the inverse geodesic problem, providing highly accurate distance and azimuth calculations on an ellipsoidal Earth model.
Advanced Sensor Fusion: * Utilizing an Extended Kalman Filter (UKF) and (KF) to fuse data from GPS and IMU (10-axis).
Real-time determination of Observation Posture (Attitude) and precise Target Localization.
High-Performance I/O: * Optimized communication peripherals (UART/SPI) using DMA (Direct Memory Access) and Interrupts. Minimizing CPU overhead to ensure stable high-frequency sensor sampling.

## Hardware
- MCU: STM32F411CEU6
- GPS module LC76GAB
- IMU module 10 axis
- LoRa module SX1278
- ST-Link debugger

## Pin Mapping
| Device pin     | STM32 pin |
|----------------|-----------|
| LCD_SPI SDA    | PA7       |
| LCD_SPI SCL    | PA5       |
| LCD_SPI CS     | PA4       |
| LCD_RESET      | PB0       |
| LCD_DC         | PB1       |
| GPS_TX         | PA3       |
| GPS_RX         | PA2       |
| IMU_I2C SCL    | PB10      |
| IMU_I2C SDA    | PB9       |
| POWER ADC      | PA1       |

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
Detailed testing procedures and evaluation results are described in **REPORT_GD1**, Chapter 5: *System Implementation and Evaluation*.
REPORT_GD1.pdf: https://github.com/bachbeastCE/DACN_HK251/blob/main/REPORT_GD1.pdf

##Future
RTOS Integration: Migrating the firmware to FreeRTOS for better task scheduling, priority management, and deterministic system behavior.
Long-Range Telemetry: Expanding the communication range by implementing a dedicated LoRa Gateway to transmit data to a remote management station.
Power Optimization: Implementing Low Power modes for the STM32 and peripheral sleep cycles to extend battery life for field operations.
Security: Integrated AES-128-GCM encryption using TinyAES to provide data confidentiality and integrity on resource-constrained MCUs. 

