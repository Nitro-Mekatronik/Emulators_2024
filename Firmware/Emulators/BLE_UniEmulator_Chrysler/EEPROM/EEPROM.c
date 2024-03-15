/******************************************************
 ** Project    : I2C EEPROM V2.0b
 ** Created on : 13/07/2022
 ** Author     : Eng Abdullah Jalloul
 ******************************************************/

#include "EEPROM.h"
#include <string.h>

extern I2C_HandleTypeDef EEPROM_I2C_PORT;

// Timer to track time since last EEPROM operation
uint32_t eepromTimer = 0;

// Current EEPROM model in use
uint8_t deviceNumber = AT24C01;

// Array of structs providing metadata for various AT24C EEPROM models
const eepromInfo_t eepromInfo[] = {
    {128, 8},     // AT24C01,
    {256, 8},     // AT24C02,
    {512, 16},    // AT24C04,
    {1024, 16},   // AT24C08,
    {2048, 16},   // AT24C16,
    {4096, 32},   // AT24C32,
    {8192, 32},   // AT24C64, // <= this
    {16384, 64},  // AT24C128,
    {32768, 64},  // AT24C256
    {65536, 128}, // AT24C512,
};

//******************************************************************************************
HAL_StatusTypeDef EEPROM_Init(uint8_t eepromNumber)
{
    // Ensure valid EEPROM model number and check device readiness
    deviceNumber = (eepromNumber > AT24C512) ? AT24C512 : eepromNumber;
    return HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, EEPROM_DEVICE_ADDR, 10, 100);
}
//******************************************************************************************
HAL_StatusTypeDef EEPROM_Read(uint16_t MemAddress, uint8_t *data, uint16_t length)
{

    uint16_t DeviceAddress;
    HAL_StatusTypeDef EepromStatus = HAL_OK;

    if (((MemAddress + length) > eepromInfo[deviceNumber].memorySize) || (length == 0))
    {
        return HAL_ERROR;
    }

    uint32_t wait = HAL_GetTick() - eepromTimer;
    if (wait < SELF_TIMED_READ_CYCLE)
        HAL_Delay(SELF_TIMED_READ_CYCLE - wait);

    // Different EEPROM models may need adjusted device addresses for reading
    switch (deviceNumber)
    {
    case AT24C01:
    case AT24C02:
        DeviceAddress = EEPROM_DEVICE_ADDR;
        EepromStatus = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT, data,
                                        length, 100);
        break;
    case AT24C04:
        DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0100) >> 7);
        EepromStatus = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT, data,
                                        length, 100);
        break;
    case AT24C08:
        DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0300) >> 7);
        EepromStatus = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT, data,
                                        length, 100);
        break;
    case AT24C16:
        DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0700) >> 7);
        EepromStatus = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT, data,
                                        length, 100);
        break;
    default:
        EepromStatus = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, EEPROM_DEVICE_ADDR, MemAddress, I2C_MEMADD_SIZE_16BIT, data,
                                        length, 100);
        break;
    }

    eepromTimer = HAL_GetTick();
    return EepromStatus;
}
//******************************************************************************************
HAL_StatusTypeDef EEPROM_Write(uint16_t MemAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef EepromStatus = HAL_OK;

    if (((MemAddress + length) > eepromInfo[deviceNumber].memorySize) || (length == 0))
    {
        return HAL_ERROR;
    }

    while (length > 0)
    {
        uint16_t nWrite; // number of bytes to write
        uint16_t nPage;  // number of bytes remaining on current page, starting at addr
        uint16_t DeviceAddress;

        uint32_t wait = HAL_GetTick() - eepromTimer;
        if (wait < SELF_TIMED_WRITE_CYCLE)
            HAL_Delay(SELF_TIMED_WRITE_CYCLE - wait);

        nPage = eepromInfo[deviceNumber].pageSize - (MemAddress & (eepromInfo[deviceNumber].pageSize - 1));
        nWrite = length < nPage ? length : nPage;

        // Different EEPROM models may need adjusted device addresses for writing
        switch (deviceNumber)
        {
        case AT24C01:
        case AT24C02:
            DeviceAddress = EEPROM_DEVICE_ADDR;
            EepromStatus = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT,
                                             data, nWrite, 100);
            break;
        case AT24C04:
            DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0100) >> 7);
            EepromStatus = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT,
                                             data, nWrite, 100);
            break;
        case AT24C08:
            DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0300) >> 7);
            EepromStatus = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT,
                                             data, nWrite, 100);
            break;
        case AT24C16:
            DeviceAddress = EEPROM_DEVICE_ADDR | ((MemAddress & 0x0700) >> 7);
            EepromStatus = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, DeviceAddress, MemAddress & 0xFF, I2C_MEMADD_SIZE_8BIT,
                                             data, nWrite, 100);
            break;
        default:
            EepromStatus = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, EEPROM_DEVICE_ADDR, MemAddress, I2C_MEMADD_SIZE_16BIT,
                                             data, nWrite, 100);
            break;
        }

        MemAddress += nWrite; // increment the EEPROM address
        data += nWrite;       // increment the input data pointer
        length -= nWrite;     // decrement the number of bytes left to write
        eepromTimer = HAL_GetTick();
    }

    return EepromStatus;
}
//******************************************************************************************
