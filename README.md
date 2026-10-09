# ESC Config
**Original AM32 Firmware Credits**

This project is based on and developed for the original open-source AM32 firmware. All credits for the original firmware, motor control algorithms, and related resources belong to the AM32 developers and contributors.

**Original Source:** https://github.com/am32-firmware/AM32

**Windows Configuration & Firmware Flashing Tool for AM32 ESCs**

ESC Config is a Windows desktop application for configuring, managing, and updating AM32-based Electronic Speed Controllers (ESCs).

The software provides a simple graphical interface to read ESC information, modify motor parameters, manage EEPROM settings, and flash firmware through the ESC bootloader.

![ESC Config Interface](https://github.com/user-attachments/assets/8de0549e-1a2b-4092-b133-f2752c901c8b)

## Features

- Automatic COM port detection and serial communication
- Read ESC firmware, bootloader, and device information
- Read, modify, and save ESC configuration parameters
- EEPROM read/write and configuration backup
- Firmware flashing through the ESC bootloader
- Support for Direct USB and Arduino / ESP communication
- Firmware flashing progress and connection status
- Built-in communication logs and error reporting
- Portable Windows application with no installation required

## Communication Modes

ESC Config supports two connection methods:

### 1. Direct Bootloader Mode

Connect directly to the ESC using a compatible USB-to-Serial adapter, such as CH340 or CP2102.

**Connection:** PC → USB-UART → ESC

- Direct communication with the ESC bootloader
- Default bootloader baud rate: **19200**
- Supports firmware flashing and compatible ESC operations

### 2. Arduino / ESP Bridge Mode

Use a compatible Arduino, ESP32, or ESP32-C3 device as a communication bridge.

**Connection:** PC → Arduino / ESP → ESC

- Communication through an intermediate microcontroller
- Supports custom ESC configuration cables
- Requires compatible bridge firmware
- Supports configuration and firmware flashing through the bridge

## ESC Configuration & EEPROM

ESC Config allows users to read and modify supported parameters, including:

- PWM frequency and motor timing
- Startup power, speed, and duty settings
- Sine startup and RPM configuration
- Throttle range and neutral position
- Forward/reverse and braking settings
- Servo and DShot input settings
- Motor protection and other firmware-dependent parameters

Users can also read, write, back up, and restore supported EEPROM configuration data.

**Note:** Available parameters depend on the ESC hardware, firmware version, and EEPROM format.

## Firmware Flashing

Firmware updates are supported in both Direct Bootloader and Arduino / ESP Bridge modes.

To flash firmware:

1. Connect the ESC using a supported communication method.
2. Open ESC Config and select the correct COM port.
3. Choose the appropriate communication mode.
4. Connect to the ESC and verify device information.
5. Open the firmware section and select a compatible firmware file.
6. Start flashing and monitor the progress.
7. Restart the ESC after successful completion if required.

**Important:** Do not disconnect USB, interrupt power, or reset the ESC during firmware flashing. Always verify firmware compatibility before updating.

## Installation

**Supported OS:** Windows 10 / Windows 11 (64-bit recommended).

1. Open the GitHub repository's **Releases** section.
2. Download the latest application package.
3. Extract the complete archive.
4. Run `BasicEscConfig.exe`.

The application is portable and does not require installation.

Keep the executable, Qt libraries, and other included runtime files in the same extracted folder.

Depending on the connection hardware, CH340, CP210x, Arduino, or ESP USB drivers may be required.

## Basic Usage

1. Connect and power the ESC.
2. Select the correct COM port and communication mode.
3. Click **Connect**.
4. Read the ESC information and current configuration.
5. Modify parameters and write changes when necessary.
6. Use the EEPROM section to manage configuration data.
7. Use the firmware section to update ESC firmware.

It is recommended to back up the existing ESC configuration before making major changes or flashing new firmware.

## Troubleshooting

**Application does not start**
- Extract the complete release package.
- Ensure the required DLL and Qt runtime files are included.

**COM port not detected**
- Check the USB cable and Windows Device Manager.
- Install the correct USB driver.

**ESC connection failed**
- Verify the COM port, communication mode, wiring, and common ground.
- Check ESC power and bootloader compatibility.
- For Bridge Mode, verify that compatible bridge firmware is installed.

**Firmware flashing failed**
- Verify firmware compatibility and connection stability.
- Check ESC power, bootloader communication, and USB connection.
- Review the application log for error details.

## Safety & Compatibility

ESC configuration and firmware flashing can directly affect motor operation.

- Use firmware compatible with your ESC hardware.
- Back up important EEPROM settings before updating.
- Ensure stable power and communication during flashing.
- Test motor operation in a safe environment after configuration changes.
- Remove propellers or secure connected motors when testing.

Incorrect firmware or interrupted flashing may require recovery using an external programmer.

## Downloads & Bug Reports

Download the latest version from the repository's **Releases** page.

For bug reports or feature requests, open a **GitHub Issue** and include the application version, ESC hardware, communication mode, error description, and relevant logs.

## Project Status

ESC Config is under development, with ongoing improvements to firmware flashing, ESC compatibility, communication reliability, and user interface functionality.

## Disclaimer

This software is provided as-is. The developer is not responsible for damage caused by incompatible firmware, incorrect configuration, wiring errors, power interruptions, or improper use.

Use at your own risk.

## Credits

Developed for AM32-based ESC configuration and firmware management using the Qt Framework, USB-Serial communication, and Arduino / ESP bridge technologies.
