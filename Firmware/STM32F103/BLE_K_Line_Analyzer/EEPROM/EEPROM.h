#ifndef _I2C_EEPROM_H_
#define _I2C_EEPROM_H_

/******************************************************
 ** Project    : I2C EEPROM V2.0b
 ** Created on : 13/07/2022
 ** Author     : Eng Abdullah Jalloul
 ******************************************************/

#include "stm32f1xx_hal.h"
#include <stdint.h>

// Define the I2C address for the EEPROM device
#define EEPROM_DEVICE_ADDR 0xA0

// Default I2C port configuration for EEPROM communication
#ifndef EEPROM_I2C_PORT
#define EEPROM_I2C_PORT hi2c1
#endif

// Timing constants for EEPROM operations
// Time (in ms) that must elapse between write operations to the EEPROM
#ifndef SELF_TIMED_WRITE_CYCLE
#define SELF_TIMED_WRITE_CYCLE 10
#endif

// Time (in ms) that must elapse between read operations from the EEPROM
#ifndef SELF_TIMED_READ_CYCLE
#define SELF_TIMED_READ_CYCLE 5
#endif

// Enumeration for supported EEPROM models
enum
{
    AT24C01,
    AT24C02,
    AT24C04,
    AT24C08,
    AT24C16,
    AT24C32,
    AT24C64,
    AT24C128,
    AT24C256,
    AT24C512,

    AT24CS01, // with Serial Number
    AT24CS02,
    AT24CS04,
    AT24CS08,
    AT24CS16,
    AT24CS32,
    AT24CS64,
};

// Structure defining the properties of a specific EEPROM model
typedef struct
{
    uint32_t memorySize; // Total memory size in bytes
    uint16_t pageSize;   // Page size in bytes (maximum 256 bytes)
} eepromInfo_t;

// Function to initialize the EEPROM
HAL_StatusTypeDef EEPROM_Init(uint8_t eepromNumber);

// Function to read data from the EEPROM
HAL_StatusTypeDef EEPROM_Read(uint16_t MemAddress, uint8_t *data, uint16_t length);

// Function to write data to the EEPROM
HAL_StatusTypeDef EEPROM_Write(uint16_t MemAddress, uint8_t *data, uint16_t length);

#endif // _I2C_EEPROM_H_
