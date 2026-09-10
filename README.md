# Config Tool 

An ESC configuration application for AT or STM firmware. The tool connects through USB/Serial to read, modify, and save ESC settings.

## Features

- Connect to the ESC through USB / Arduino / Serial
- Read and save EEPROM configuration
- Configure motor, PWM, timing, brake, and protection settings
- Configure Input / Servo / Current Limit PID settings
- Flash firmware and load default settings
- Basic motor-control testing

## Interface

<img width="998" height="828" alt="image" src="https://github.com/user-attachments/assets/1948ef6c-9a0c-4cc5-b52b-9be7b6c8e92e" />




## Getting Started

1. Connect the ESC to your computer.
2. Select the correct COM port.
3. Click **Connect**.
4. Select the target motor and read its settings.
5. Adjust the required parameters, then click **Save Settings**.

> Note: Only write EEPROM data or flash firmware after confirming the correct ESC is selected and the power supply is stable.
