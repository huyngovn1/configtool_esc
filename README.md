# ESC Config

> Windows configuration, EEPROM management, and firmware flashing tool for AM32-based ESCs.
> <img width="1920" height="1032" alt="image" src="https://github.com/user-attachments/assets/8de0549e-1a2b-4092-b133-f2752c901c8b" />


ESC Config is a Windows desktop application designed for configuring, managing, and updating AM32-based ESCs through serial communication.

The application provides a graphical interface that allows users to connect to an ESC, read device information, read and modify ESC parameters, manage EEPROM data, create configuration backups, and flash firmware directly through the ESC bootloader.

ESC Config supports two communication methods:

- Direct Bootloader connection through a USB-to-Serial adapter
- Arduino / ESP Bridge connection through an intermediate microcontroller device

The software is designed to simplify ESC configuration and firmware update procedures without requiring users to manually interact with bootloader commands or raw serial data.

---

## Features

ESC Config includes the following features:

- Automatic COM port detection
- Serial communication with ESC devices
- Direct connection through USB-to-Serial adapters
- Arduino / ESP bridge communication
- Support for CH340 USB-to-Serial adapters
- Support for CP2102 USB-to-Serial adapters
- Support for compatible USB-UART interfaces
- Support for Arduino-based communication bridges
- Support for ESP32-based communication bridges
- Support for ESP32-C3 bridge devices
- Direct ESC bootloader communication
- Bootloader communication at `19200 baud`
- Read ESC device information
- Read firmware information
- Read bootloader information
- Read EEPROM information
- Read current ESC configuration
- Modify ESC parameters
- Write ESC parameters back to the device
- Read EEPROM data
- Write EEPROM data
- Back up ESC configuration data
- Restore supported ESC configuration data
- Select firmware files
- Flash firmware through the ESC bootloader
- Flash firmware using Direct Bootloader mode
- Flash firmware using Arduino / ESP Bridge mode
- Display firmware flashing progress
- Display connection status
- Display bootloader communication status
- Display firmware flashing errors
- Built-in communication log
- Simple graphical user interface
- Portable Windows application
- No installation required

---

# Communication Modes

ESC Config supports two different communication modes.

The correct communication mode must be selected depending on how the ESC is connected to the computer.

---

## 1. Direct Bootloader Mode

Direct Bootloader mode is used when the ESC is connected directly to the computer through a USB-to-Serial adapter.

Typical connection:

```text
PC
│
USB
│
CH340 / CP2102 / USB-UART
│
ESC Bootloader Communication
│
ESC
```

Supported USB-to-Serial adapters may include:

- CH340
- CH341
- CP2102
- CP210x
- Other compatible USB-UART devices

The default bootloader communication speed is:

```text
19200 baud
```

This mode should be selected when the USB-to-Serial adapter communicates directly with the ESC bootloader.

Typical connection example:

```text
PC → CH340 / CP2102 → ESC
```

Direct Bootloader mode is recommended when:

- A USB-UART adapter is connected directly to the ESC
- No Arduino or ESP bridge is required
- The ESC bootloader communication pin is directly accessible
- A simple wired connection is preferred
- Maximum communication simplicity is required

---

## 2. Arduino / ESP Bridge Mode

Arduino / ESP Bridge mode is used when an Arduino, ESP32, ESP32-C3, or another compatible microcontroller acts as an intermediate communication bridge between the computer and the ESC.

Typical connection:

```text
PC
│
USB
│
Arduino / ESP
│
Bridge Communication
│
UART / One-Wire / Custom Interface
│
ESC
```

Typical connection example:

```text
PC → Arduino / ESP → ESC
```

Supported bridge devices may include:

- Arduino
- Arduino Nano
- Arduino Uno
- Arduino-compatible devices
- ESP32
- ESP32-C3
- Other compatible ESP devices
- Custom microcontroller bridge devices

In this mode, ESC Config communicates with the Arduino or ESP device through USB Serial.

The Arduino or ESP device then forwards the required commands, data, and bootloader packets between the computer and the ESC.

This mode can be useful when:

- A direct USB-UART connection is not available
- A custom ESC configuration cable is used
- An Arduino-based ESC interface is used
- An ESP32-based ESC interface is used
- An ESP32-C3 configuration cable is used
- One-wire communication is required
- Additional protocol handling is required
- A custom communication bridge is required between the PC and the ESC

The Arduino / ESP device must contain compatible bridge firmware.

---

# ESC Configuration

ESC Config allows users to read and modify ESC parameters directly from the graphical interface.

Depending on the connected ESC firmware version, available settings may include:

- PWM Frequency
- PWM Mode
- Motor Timing
- Motor Pole Count
- Motor KV
- Startup Power
- Startup Speed
- Startup Duty
- Minimum Startup Duty
- Sine Startup
- Sine Startup Power
- Sine Transition Level
- Low RPM Settings
- High RPM Settings
- Throttle Range
- Neutral Position
- Forward / Reverse Configuration
- Brake Mode
- Active Brake
- Running Brake
- Drag Brake
- Brake Strength
- Buzzer Settings
- Control Signal Type
- Servo Input Settings
- DShot Settings
- Protection Settings
- Motor Protection Settings
- Startup Protection
- Current Protection
- Stall Protection
- Firmware-dependent configuration parameters

Available parameters may vary depending on:

- ESC hardware
- MCU type
- AM32 firmware version
- Bootloader version
- EEPROM format
- ESC configuration format

It is recommended to read the current ESC configuration before modifying any parameter.

After making changes, the updated configuration can be written back to the ESC.

---

# EEPROM Management

ESC Config provides tools for reading and working with EEPROM configuration data stored inside the ESC.

The EEPROM section may allow the user to:

- Read EEPROM data
- Display EEPROM information
- Display configuration values
- Modify supported EEPROM parameters
- Write EEPROM data back to the ESC
- Back up configuration data
- Restore supported configuration data
- Compare configuration values
- Verify ESC settings before firmware updates

It is strongly recommended to create a backup before:

- Changing important ESC parameters
- Updating firmware
- Testing experimental firmware
- Changing EEPROM values
- Modifying motor control parameters
- Changing bootloader-related configuration
- Testing a new ESC configuration

A valid backup can help restore the previous configuration if a new configuration does not operate correctly.

---

# Firmware Flashing

ESC Config supports flashing firmware directly through the ESC bootloader.

Firmware flashing can be performed using either communication mode:

```text
Direct Bootloader
```

or:

```text
Arduino / ESP Bridge
```

The application sends firmware data to the ESC bootloader and monitors the flashing process.

Depending on the ESC bootloader and firmware format, the flashing process may include:

- Bootloader connection
- Device detection
- Bootloader initialization
- Flash memory erase
- Firmware data transfer
- Flash programming
- Address control
- Data verification
- Bootloader response checking
- Flash progress monitoring
- Completion status reporting

---

# Direct Bootloader Firmware Flashing

Typical connection:

```text
PC → CH340 / CP2102 → ESC
```

To flash firmware using Direct Bootloader mode:

1. Connect the ESC to a compatible USB-to-Serial adapter.

2. Connect the USB-to-Serial adapter to the computer.

3. Make sure the ESC and USB-UART device share the correct ground connection.

4. Power the ESC correctly.

5. Open:

```text
BasicEscConfig.exe
```

6. Select the correct COM port.

7. Select:

```text
Direct Bootloader - 19200 baud
```

8. Click the connection button.

9. Wait for the ESC bootloader to respond.

10. Verify that the ESC information is detected correctly.

11. Open the firmware flashing section.

12. Select the correct firmware file.

13. Confirm that the firmware is compatible with the connected ESC hardware.

14. Start the flashing process.

15. Wait while the firmware data is transferred.

16. Do not disconnect the ESC during the flashing process.

17. Wait until ESC Config reports that the firmware flashing process has completed successfully.

18. Restart the ESC if required.

---

# Arduino / ESP Bridge Firmware Flashing

Typical connection:

```text
PC → Arduino / ESP → ESC
```

To flash firmware using Arduino / ESP Bridge mode:

1. Connect the Arduino or ESP bridge device to the computer.

2. Connect the bridge device to the ESC.

3. Make sure the ESC, bridge device, and computer interface use the correct common ground.

4. Make sure the Arduino or ESP device is running compatible bridge firmware.

5. Power the ESC correctly.

6. Open:

```text
BasicEscConfig.exe
```

7. Select the COM port assigned to the Arduino or ESP device.

8. Select the Arduino / ESP communication mode.

9. Click the connection button.

10. Wait for the bridge device to establish communication with the ESC.

11. Verify that the ESC information is detected correctly.

12. Open the firmware flashing section.

13. Select the correct firmware file.

14. Start the firmware flashing process.

15. ESC Config will communicate with the bridge device.

16. The bridge device will forward bootloader commands and firmware data to the ESC.

17. Wait until the firmware update is completed.

18. Do not disconnect the USB cable, bridge device, or ESC during flashing.

19. Restart the ESC if required after flashing is complete.

---

# Important Firmware Flashing Notes

Firmware flashing is a critical operation.

During firmware flashing:

- Do not disconnect the USB cable
- Do not disconnect the USB-to-Serial adapter
- Do not disconnect the Arduino / ESP bridge
- Do not disconnect the ESC communication wire
- Do not remove power from the ESC
- Do not reset the ESC
- Do not reset the bridge device
- Do not close ESC Config
- Do not open another program that uses the same COM port
- Do not change the selected COM port during flashing
- Do not interrupt the firmware flashing process
- Make sure the ESC power supply is stable
- Make sure the USB connection is stable
- Make sure the serial communication line is stable
- Always use firmware compatible with the ESC hardware

Using incorrect or incompatible firmware may cause the ESC to stop operating correctly.

In some cases, incorrect firmware may require the ESC to be recovered using an external programmer.

---

# Hardware Requirements

One of the following connection methods is required.

---

## Direct Connection Hardware

For Direct Bootloader mode:

- CH340 USB-to-Serial adapter
- CP2102 USB-to-Serial adapter
- Compatible USB-UART interface
- Correct ESC bootloader communication wiring
- Stable ESC power supply

Typical connection:

```text
Computer → USB-UART → ESC
```

---

## Bridge Connection Hardware

For Arduino / ESP Bridge mode:

- Arduino
- ESP32
- ESP32-C3
- Compatible microcontroller bridge
- Compatible bridge firmware
- Correct ESC communication wiring
- Stable ESC power supply

Typical connection:

```text
Computer → Arduino / ESP → ESC
```

---

# Software Requirements

Supported operating systems:

- Windows 10
- Windows 11

Recommended:

```text
64-bit Windows
```

Depending on the USB interface used, the correct USB driver may also be required.

Examples:

```text
CH340 Driver
CP210x Driver
Arduino USB Driver
ESP USB Driver
```

---

# Installation

ESC Config is distributed as a portable Windows application.

No installer is required.

To install and run ESC Config:

1. Open the GitHub repository.

2. Go to:

```text
Releases
```

3. Download the latest release package.

Example:

```text
BasicEscConfig_v1.0.0.zip
```

4. Extract the entire archive.

5. Open the extracted folder.

6. Run:

```text
BasicEscConfig.exe
```

Do not move only the `.exe` file to another location.

The required DLL files and Qt runtime folders must remain with the application.

---

# Included Runtime Files

The release package may contain files similar to the following:

```text
BasicEscConfig.exe

Qt5Core.dll
Qt5Gui.dll
Qt5Widgets.dll
Qt5SerialPort.dll
Qt5Network.dll

libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll

platforms/
imageformats/
iconengines/
styles/
translations/
```

The exact list may vary depending on the application version.

These files are required by the application and should not be removed.

---

# Basic Usage

To use ESC Config:

1. Connect the ESC using one of the supported connection methods.

For Direct Mode:

```text
PC → CH340 / CP2102 → ESC
```

For Bridge Mode:

```text
PC → Arduino / ESP → ESC
```

2. Power the ESC.

3. Open:

```text
BasicEscConfig.exe
```

4. Select the correct COM port.

5. Select the correct communication mode.

Available modes may include:

```text
Direct Bootloader
Arduino / ESP Bridge
```

6. Click **Connect**.

7. Wait for the application to detect the ESC.

8. Read the ESC information.

9. Read the current ESC configuration.

10. Modify the required parameters.

11. Write the updated configuration back to the ESC.

12. Use the EEPROM section for EEPROM operations.

13. Use the firmware section for firmware updates.

14. Restart the ESC if required.

---

# Recommended Workflow

For safe operation, the following workflow is recommended:

1. Connect to the ESC.

2. Read ESC information.

3. Read the current configuration.

4. Save or back up the existing configuration.

5. Make the required configuration changes.

6. Write the new configuration.

7. Test the ESC carefully.

8. If a firmware update is required, verify the correct hardware and firmware target.

9. Create an EEPROM backup before flashing.

10. Flash the firmware.

11. Restart the ESC.

12. Read the ESC information again.

13. Verify firmware version.

14. Restore or modify configuration if required.

15. Perform a controlled motor test.

---

# Troubleshooting

## Application Does Not Start

If ESC Config does not start, make sure that:

- The entire release archive was extracted
- `BasicEscConfig.exe` is located with the required DLL files
- Qt runtime files are present
- MinGW runtime files are present
- The correct Windows architecture is used

Do not download random missing DLL files from third-party websites.

Always use the complete official release package.

---

## No COM Port Detected

If no COM port appears:

- Check the USB cable
- Check the USB port
- Check Windows Device Manager
- Install the correct USB driver
- Reconnect the USB device
- Restart ESC Config
- Make sure the USB cable supports data

For CH340 devices, install the CH340 driver if required.

For CP2102 devices, install the Silicon Labs CP210x driver if required.

For Arduino or ESP devices, make sure the correct USB driver is installed.

---

## Unable to Connect to ESC

Check the following:

- Correct COM port selected
- Correct communication mode selected
- ESC is powered
- USB device is connected
- TX/RX wiring is correct if required
- One-wire communication wiring is correct if used
- Ground is shared between devices
- Bootloader communication pin is connected correctly
- Correct baud rate is selected
- ESC bootloader is compatible
- No other application is using the COM port

---

## Direct Bootloader Connection Failed

If Direct Bootloader mode cannot connect:

- Verify the USB-UART adapter
- Verify the communication wiring
- Verify ESC power
- Verify common ground
- Verify the ESC bootloader communication pin
- Verify `19200 baud`
- Disconnect and reconnect the ESC
- Restart ESC Config
- Try another USB port
- Try another USB-UART adapter if available

---

## Arduino / ESP Bridge Connection Failed

If Arduino / ESP Bridge mode cannot connect:

- Verify the bridge device is detected by Windows
- Verify the correct COM port
- Verify compatible bridge firmware is installed
- Verify communication wiring between the bridge and ESC
- Verify common ground
- Verify ESC power
- Restart the bridge device
- Restart ESC Config
- Reconnect the USB cable

---

## Firmware Flash Failed

If firmware flashing fails:

- Verify the firmware file is correct
- Verify firmware compatibility with ESC hardware
- Verify ESC power is stable
- Verify USB connection
- Verify communication wiring
- Verify bootloader communication
- Verify correct communication mode
- Verify bridge firmware
- Close other serial applications
- Reconnect the ESC
- Restart the application
- Retry the flashing process

If flashing repeatedly fails at the same position, check:

- Serial communication quality
- USB cable quality
- Power supply stability
- Bootloader compatibility
- Firmware file integrity

---

## Firmware Flash Stops at the Beginning

If firmware flashing fails immediately:

- Make sure the ESC entered bootloader mode
- Make sure the application successfully detected the bootloader
- Verify communication mode
- Verify COM port
- Verify ESC communication wiring
- Verify bridge firmware if using Arduino / ESP mode
- Verify the firmware file format

---

## Firmware Flash Stops During Transfer

If flashing starts but stops during firmware transfer:

- Check USB cable stability
- Check ESC power supply
- Check serial wiring
- Check electrical noise
- Avoid long communication wires
- Avoid poor-quality USB cables
- Do not move connectors during flashing
- Make sure no other application accesses the same COM port

---

# Safety

Always test ESC configuration carefully.

When testing an ESC with a motor:

- Keep the motor away from people
- Secure the motor properly
- Remove propellers when testing drone or aircraft ESCs
- Keep wheels off the ground when testing RC car systems if necessary
- Use a current-limited power supply during development when possible
- Verify throttle input before enabling motor operation
- Make sure the control signal is correct
- Make sure the motor direction is correct

Incorrect ESC settings may cause unexpected motor operation.

---

# Download

The latest version can be downloaded from the GitHub Releases page.

Open:

```text
GitHub Repository
↓
Releases
↓
Latest Release
```

Download the latest package.

Example:

```text
BasicEscConfig_v1.0.0.zip
```

Extract the archive and run:

```text
BasicEscConfig.exe
```

---

# Release Package

A typical release package may look like:

```text
BasicEscConfig_v1.0.0/
│
├── BasicEscConfig.exe
├── Qt5Core.dll
├── Qt5Gui.dll
├── Qt5Widgets.dll
├── Qt5SerialPort.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
│
├── platforms/
├── imageformats/
├── iconengines/
├── styles/
└── translations/
```

Do not delete files from the release package unless you know they are not required.

---

# Bug Reports

If you find a bug, please open a GitHub Issue.

When reporting a problem, please provide as much information as possible.

Recommended information:

- ESC Config version
- Windows version
- ESC firmware version
- ESC bootloader version
- ESC hardware type
- MCU type
- Communication mode
- Direct Bootloader or Arduino / ESP Bridge
- USB adapter model
- Arduino / ESP device type
- Bridge firmware version
- Firmware file used
- Application log
- Screenshot of the error
- Description of the problem
- Steps required to reproduce the problem

Detailed reports make debugging much easier.

---

# Version

Current release:

```text
ESC Config v1.0.0
```

---

# Project Status

ESC Config is under active development.

New versions may include:

- Improved ESC detection
- Improved bootloader communication
- Improved firmware flashing
- Additional ESC parameters
- Improved EEPROM management
- Additional hardware support
- Improved Arduino / ESP bridge support
- Improved error handling
- Improved communication reliability
- User interface improvements

---

# Disclaimer

ESC configuration and firmware flashing can directly affect ESC operation.

Always make sure that:

- The firmware matches the ESC hardware
- The correct bootloader is used
- Important configuration data is backed up
- EEPROM data is backed up when necessary
- ESC power is stable
- USB communication is stable
- Communication wiring is correct
- ESC parameters are configured correctly

The developer is not responsible for damage caused by:

- Incorrect firmware
- Incompatible firmware
- Incorrect ESC configuration
- Incorrect EEPROM data
- Unsupported ESC hardware
- Power loss during firmware flashing
- USB communication failure
- Serial communication failure
- Incorrect wiring
- Bridge firmware errors
- Improper use of the application

Use this software at your own risk.

---

# License

Please refer to the repository license for information about:

- Usage
- Modification
- Redistribution
- Source code usage
- Commercial use
- Project contribution

If no license is provided, please contact the project owner before redistributing or modifying the software.

---

# Credits

ESC Config is developed as a configuration and firmware management tool for AM32-based ESC development.

The project may use or reference technologies and concepts from:

- AM32 Firmware
- AM32 Bootloader
- Qt Framework
- USB Serial communication
- Arduino
- ESP32
- ESP32-C3
- CH340
- CP2102

All trademarks and product names belong to their respective owners.
