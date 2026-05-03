# DACN_HK251 - Target Localization System

## Overview

This project implements a comprehensive target localization system using embedded hardware and software components. The system integrates GPS, IMU (Inertial Measurement Unit), and LoRa communication modules to estimate target coordinates based on measured distances and device positions. It includes real-time sensor fusion using Kalman filters, data visualization through a Qt-based GUI, and data processing scripts in Scilab.

The project consists of multiple components:
- **STM32 Firmware**: Embedded code for the microcontroller handling sensor data acquisition and processing.
- **Qt Application**: Desktop GUI for real-time visualization of maps and localization data.
- **Linux LoRa Station**: Server-side application for LoRa communication and cloud integration.
- **Scilab Scripts**: Data analysis and filtering algorithms for post-processing.

## Key Features

- **Precision Navigation**: Implements Karney's algorithm for accurate geodesic calculations on an ellipsoidal Earth model.
- **Sensor Fusion**: Uses Extended Kalman Filter (UKF) and Kalman Filter (KF) to combine GPS and 10-axis IMU data.
- **Real-time Localization**: Determines observation posture (attitude) and target coordinates in real-time.
- **Wireless Communication**: LoRa-based communication for remote data transmission.
- **Data Visualization**: Interactive maps and charts using Qt framework.
- **Data Analysis**: Scilab scripts for UKF filtering and data logging.

## Hardware Requirements

- **MCU**: STM32F411CEU6
- **GPS Module**: LC76GAB
- **IMU Module**: 10-axis IMU
- **LoRa Module**: SX1278
- **Display**: ST7735 LCD (optional)
- **Debugger**: ST-Link

## Software Requirements

- **STM32CubeIDE**: For firmware development and flashing
- **Qt Creator**: For GUI application development (Qt 5.14.2 or later)
- **GCC/Make**: For building Linux LoRa station
- **Scilab**: For data analysis scripts
- **Altium Designer**: For PCB design (optional)

## Project Structure

```
DACN_HK251/
├── App/APPDACN/              # Qt desktop application
│   ├── APPDACN.pro          # Qt project file
│   ├── main.cpp             # Application entry point
│   ├── mainwindow.cpp       # Main window implementation
│   ├── inform.cpp           # Information dialog
│   └── qmlMAPS.qml          # QML map interface
├── Linux Lora station/       # Linux-based LoRa communication
│   ├── version_new/
│   │   ├── cloud/           # Cloud integration components
│   │   └── lora_station/    # LoRa station firmware
│   └── version_old/         # Legacy version
├── Scilab/                   # Data processing scripts
│   ├── draw.sci             # Plotting scripts
│   ├── ukf.sci              # Unscented Kalman Filter
│   └── data_log.txt         # Sample data logs
└── STM32/                    # STM32 microcontroller firmware
    ├── Core/                # Application code
    │   ├── Inc/             # Header files
    │   └── Src/             # Source files
    ├── Drivers/             # HAL drivers
    └── DACN-KTMT.ioc        # STM32CubeMX configuration
```

## Installation and Setup

### 1. STM32 Firmware

1. Clone the repository:
   ```bash
   git clone https://github.com/bachbeastCE/DACN_HK251.git
   cd DACN_HK251
   ```

2. Open STM32CubeIDE and import the project:
   - File → Import → Existing Projects into Workspace
   - Select the `STM32` folder

3. Build and flash the firmware to your STM32 board using ST-Link.

### 2. Qt Application

1. Install Qt 5.14.2 or later with MinGW 32-bit compiler.

2. Open `App/APPDACN/APPDACN.pro` in Qt Creator.

3. Build and run the application:
   ```bash
   qmake APPDACN.pro
   make
   ./APPDACN
   ```

### 3. Linux LoRa Station

1. Navigate to the Linux LoRa station directory:
   ```bash
   cd "Linux Lora station/version_new"
   ```

2. Build the components:
   ```bash
   # For cloud component
   cd cloud
   make

   # For LoRa station
   cd ../lora_station
   make
   ```

3. Run the applications as needed.

### 4. Scilab Scripts

1. Install Scilab on your system.

2. Open the scripts in `Scilab/` directory:
   - `draw.sci`: For plotting data
   - `ukf.sci`: For UKF filtering
   - `uart-rev.c`: UART data receiver (if needed)

## Usage

1. **Hardware Setup**: Connect GPS, IMU, and LoRa modules to the STM32 board according to the pin mapping in `STM32/README.md`.

2. **Firmware Upload**: Flash the compiled firmware to the STM32 board.

3. **Data Collection**: Power on the device to start collecting sensor data and transmitting via LoRa.

4. **Visualization**: Run the Qt application to view real-time maps and localization data.

5. **Data Analysis**: Use Scilab scripts to process logged data and apply filtering algorithms.

## Pin Mapping (STM32)

| Device Pin      | STM32 Pin |
|-----------------|-----------|
| LCD_SPI SDA     | PA7       |
| LCD_SPI SCL     | PA5       |
| LCD_SPI CS      | PA4       |
| LCD_RESET       | PB0       |
| LCD_DC          | PB1       |
| GPS_TX          | PA3       |
| GPS_RX          | PA2       |
| IMU_I2C SCL     | PB10      |
| IMU_I2C SDA     | PB9       |
| POWER ADC       | PA1       |

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Karney's geodesic algorithms for precise Earth modeling
- STM32 HAL libraries for hardware abstraction
- Qt framework for GUI development
- Scilab for numerical computing