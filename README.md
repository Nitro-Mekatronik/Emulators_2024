# Emulators 2024

An open-source automotive diagnostic and emulation system with hardware and software components.

## Project Structure

### Firmware

#### STM32F103
- **BLE_K_Line_Analyzer**: Bluetooth Low Energy and K-Line protocol analysis software
- **Bootloader**: Custom bootloader for STM32F103
- **Emulators**: Emulator software for various vehicle protocols
- **Template**: Template for new firmware projects

### Hardware

#### Arduino_Emulator New
PCB designs and schematics for Arduino-based emulator hardware

#### Libraries
Component libraries used in hardware design

#### NTR-MAIN-U2xx
Main emulator board design

## Features

- STM32F103 microcontroller support
- Bluetooth Low Energy communication
- K-Line protocol support
- Arduino compatible design
- Custom bootloader
- Multi-protocol emulation

## Hardware Requirements

### STM32F103 Based Board
- STM32F103C8T6 or STM32F103CBT6 microcontroller
- 8MHz external crystal
- USB support
- Bluetooth module connection
- K-Line interface

### Arduino Emulator
- Arduino Nano or compatible board
- K-Line level converter
- Bluetooth module

## Software Requirements

### Firmware Development
- STM32CubeIDE 1.9.0 or higher
- STM32CubeMX
- ARM GCC Toolchain
- ST-Link software

### PCB Design
- KiCad 6.0 or higher
- Gerber viewer

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Emulators_2024.git
```

2. For firmware development:
   - Install STM32CubeIDE
   - Open the project in IDE
   - Install required dependencies
   - Build the project

3. For hardware development:
   - Install KiCad
   - Open projects in Hardware folder
   - Generate Gerber files

## Usage

### Firmware Upload
1. Connect ST-Link programmer
2. Upload bootloader
3. Upload main firmware

### Emulator Usage
1. Connect hardware to vehicle OBD port
2. Connect via Bluetooth
3. Select appropriate protocol
4. Start emulation

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## Security

If you discover any security-related issues, please email us instead of using the issue tracker.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For support, please create an issue in the GitHub issue tracker.