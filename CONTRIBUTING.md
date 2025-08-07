# Contributing Guide

Thank you for your interest in contributing to Emulators 2024. This guide explains the basic rules and processes for contributors.

## How Can You Contribute?

1. Bug Reports
2. Feature Suggestions
3. Code Contributions
4. Hardware Design Contributions
5. Documentation Improvements

## Development Environment Setup

### Firmware Development
1. STM32CubeIDE 1.9.0 or higher
2. STM32CubeMX
3. ARM GCC Toolchain
4. ST-Link software

### Hardware Development
1. KiCad 6.0 or higher
2. Gerber viewer

## Code Standards

### Firmware
- Follow MISRA C guidelines
- Use meaningful names for functions and variables
- Document your code with comments
- Add unit tests
- Use HAL library

### Hardware
- Follow KiCad design rules
- Use standard footprints
- Use meaningful names for schematics and PCB
- Perform design checks
- Generate Gerber files

## Pull Request Process

1. Create a new branch
2. Make your changes
3. Run tests
4. Write clear commit messages
5. Open pull request

## Commit Messages

Your commit messages should follow this format:

```
[Type]: Short description

Detailed description (if needed)
```

Type can be one of:
- firmware: Firmware changes
- hardware: Hardware design changes
- docs: Documentation changes
- test: Adding or modifying tests
- fix: Bug fix
- feat: New feature

## Hardware Changes

1. Schematic updates
   - Clearly indicate changes
   - List component changes
   - Include test results

2. PCB updates
   - Clearly indicate changes
   - Perform DRC check
   - Update Gerber files

## Firmware Changes

1. Code changes
   - Follow MISRA guidelines
   - Optimize memory usage
   - Perform performance tests

2. Bootloader changes
   - Perform security checks
   - Test fallback mechanisms
   - Update memory map

## Communication

- Open an Issue for questions
- Discuss significant changes by opening an Issue first
- Contact project maintainers directly for security vulnerabilities

## Security

Please report security vulnerabilities privately to the project maintainers.

## License

Your contributions will be licensed under the MIT License.