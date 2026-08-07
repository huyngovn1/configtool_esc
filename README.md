# Config Tool AT32F421K8U7

An ESC configuration application for AT32F421K8U7 firmware. The tool connects through USB/Serial to read, modify, and save ESC settings.

## Features

- Connect to the ESC through USB / Arduino / Serial
- Read and save EEPROM configuration
- Configure motor, PWM, timing, brake, and protection settings
- Configure Input / Servo / Current Limit PID settings
- Flash firmware and load default settings
- Basic motor-control testing

## Interface

<img width="999" height="833" alt="Config Tool AT32F421K8U7 interface" src="https://github.com/user-attachments/assets/6013288e-c6e2-42b6-a0bc-df5ad1e68757" />

## Getting Started

1. Connect the ESC to your computer.
2. Select the correct COM port.
3. Click **Connect**.
4. Select the target motor and read its settings.
5. Adjust the required parameters, then click **Save Settings**.

> Note: Only write EEPROM data or flash firmware after confirming the correct ESC is selected and the power supply is stable.
