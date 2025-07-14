# SV2-V38 Original Firmware

This repository contains the original firmware for the SV2-V38 device, based on FreeRTOS and STM32H7 microcontroller. It includes source code for sound and vibration data acquisition, processing, and storage, as well as algorithm implementations for event detection and calibration.

## Project Structure

- `Core/` - Main application code, device initialization, and task management
- `Algo/` - Signal processing and event detection algorithms (EVS, BLS, SLM, VLM)
- `ads127l11/` - ADS127L11 ADC driver and configuration
- `FATFS/` - File system support for SD card storage
- `Drivers/` - STM32 HAL and CMSIS drivers
- `Middlewares/` - Third-party libraries (FreeRTOS, FatFs)

## Features
- Multi-channel sound and vibration data acquisition
- Real-time event detection algorithms (EVS, BLS)
- Calibration routines for sound and vibration channels
- Data logging to SD card in WAV and TXT formats
- FreeRTOS-based multitasking

## Build Instructions
1. Open the project in STM32CubeIDE or your preferred ARM toolchain.
2. Configure the target device (STM32H743VITx) and toolchain settings as needed.
3. Build the project using the provided makefile or IDE build system.
4. Flash the generated binary to the device using ST-Link or compatible programmer.

## License
This project is proprietary and intended for use with SV2-V38 hardware only. 