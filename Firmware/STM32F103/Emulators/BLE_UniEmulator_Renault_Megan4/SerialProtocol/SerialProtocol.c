/******************************************************
 **  Project   : Serial Protocol Ver 1.4.1
 **  Version   : 1.4.1
 **  Created on: 01-02-2017
 **  Author    : Eng Abdullah Jalloul
 ******************************************************/

#include "SerialProtocol.h"
#include "License.h"
#include "SerialFIFO.h"
#include "SmartLED.h"
#include "main.h"
#include "ESL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define SUCCESS 0
#define FAILURE 1
// #define PRODUCT_ID 0 // zero for all devices

// https://github.com/gbesnard/stm32f1-bootloader/tree/master
#define BOOTLOADER_START_ADDRESS 0x08000000

//**************************************************************************

//**************************************************************************
// Structures to hold details of received and transmitted serial messages
SERIAL_RxMSG_t SerialRxMsg;
SERIAL_TxMSG_t SerialTxMsg;

//**************************************************************************

bool isRealServer = false;
uint32_t challengeServerTimeout;

typedef void (*p_function)(void);

p_function boot_entry;

//**************************************************************************
/**
 * Evaluates the received serial command and sends an appropriate response.
 */
void Serial_EvaluateCommand(void)
{
    SerialTxMsg.Command = SerialRxMsg.Command;

    if (isRealServer)
    {
        if (HAL_GetTick() > challengeServerTimeout)
        {
            isRealServer = false;
        }
    }

    switch (SerialRxMsg.Command)
    {
    case SERIAL_PING:
        Serial_HeadReply(0); // Pong response
        break;

    case SERIAL_READ_INFO: {
        char Device_Info[256];

        // Initialize the buffer with the first part
        strcpy(Device_Info, HARDWARE_VER);
        strcat(Device_Info, "_");
        strcat(Device_Info, FIRMWARE_TYPE);
        strcat(Device_Info, "_");
        strcat(Device_Info, FIRMWARE_BRAND);
        strcat(Device_Info, "_");
        strcat(Device_Info, FIRMWARE_MODEL);
				strcat(Device_Info, "_");
        strcat(Device_Info, FIRMWARE_SHIELD);
        strcat(Device_Info, "_");
        strcat(Device_Info, FIRMWARE_VER);
				strcat(Device_Info, "\0");
			
        Serial_HeadReply(strlen(Device_Info) + 1);
        Serial_WriteStr(Device_Info);
    }
    break;

    case SERIAL_READ_UNIQUE: {
        uint8_t uniqueData[17];
        uint8_t random = rand() % 256;
        uint8_t level = random >> 4;
        uint8_t shift = random & 0x0F;
        License_UniqueEncryption(uniqueData, level, shift, true);

        Serial_HeadReply(sizeof(uniqueData));
        Serial_WriteArray(uniqueData, sizeof(uniqueData));
        isRealServer = false;
        break;
    }

    case SERIAL_RESPONSE_UNIQUE: {
        uint8_t uniqueData[32];
        Serial_ReadArray(uniqueData, sizeof(uniqueData));

        isRealServer = License_Check_Unique_HASH(uniqueData);

        if (isRealServer)
        {
            challengeServerTimeout = HAL_GetTick() + 180000; // Timeout = 1000 * 60 * 3 minutes
        }

        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)isRealServer);
        break;
    }

    case SERIAL_ACTIVATE_DEVICE: {
        if (isRealServer)
        {
            uint8_t random = rand() % 256;
            uint8_t level = random >> 4;
            uint8_t shift = random & 0x0F;
            License_SetPrimary(level, shift);
        }

        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)isRealServer);
        break;
    }

    case SERIAL_ACTIVATE_BOOTLOADER: {
        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)isRealServer);
        if (isRealServer)
        {
            deinitEverything();

            uint32_t boot_stack;
            // Set HSION bit.
            RCC->CR |= 0x00000001U;

            // Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits.
            RCC->CFGR &= 0xF8FF0000U;

            // Reset HSEON, CSSON and PLLON bits.
            RCC->CR &= 0xFEF6FFFFU;

            // Reset HSEBYP bit.
            RCC->CR &= 0xFFFBFFFFU;

            // Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits.
            RCC->CFGR &= 0xFF80FFFFU;

            // Disable all interrupts and clear pending bits.
            RCC->CIR = 0x009F0000U;

            // Get the bootloader stack pointer (first entry in the bootloader vector table).
            boot_stack = (uint32_t) * ((__IO uint32_t *)BOOTLOADER_START_ADDRESS);

            // Get the bootloader entry point (second entry in the bootloader vector table).
            boot_entry = (p_function) * (__IO uint32_t *)(BOOTLOADER_START_ADDRESS + 4U);
            // Reconfigure vector table offset register to match the application location.
            SCB->VTOR = BOOTLOADER_START_ADDRESS;

            // Set the application stack pointer.
            __set_MSP(boot_stack);

            // Start the bootloader.
            boot_entry();
        }
        break;
    }

    default:
        // Handle unknown or unsupported commands
        Serial_HeadResponse(FAILURE, 0);
        break;
    }

    Serial_WriteChecksum();
}

//**************************************************************************
void Serial_Decode(void)
{
    uint16_t bytesCount = UART_Available();

    while (bytesCount--)
    {
        uint8_t c = UART_Read();

        if (SerialRxMsg.State == SERIAL_IDLE)
        {
            SerialRxMsg.State = (c == '$') ? SERIAL_HEADER_START : SERIAL_IDLE;
        }
        else if (SerialRxMsg.State == SERIAL_HEADER_START)
        {
            SerialRxMsg.State = (c == 'M') ? SERIAL_HEADER_M : SERIAL_IDLE;
        }
        else if (SerialRxMsg.State == SERIAL_HEADER_M)
        {
            SerialRxMsg.State = (c == '>') ? SERIAL_HEADER_ARROW : SERIAL_IDLE;
        }
        else if (SerialRxMsg.State == SERIAL_HEADER_ARROW)
        {

            if (c > SERIAL_PROTOCOL_BUF_SIZE)
            {
                SerialRxMsg.State = SERIAL_IDLE;
                continue;
            }

            SerialRxMsg.State = SERIAL_HEADER_SIZE;
            SerialRxMsg.DataSize = c;
            SerialRxMsg.Offset = 0;
            SerialRxMsg.IndexBuf = 0;
            SerialRxMsg.Checksum = c;
        }
        else if (SerialRxMsg.State == SERIAL_HEADER_SIZE)
        {
            SerialRxMsg.State = SERIAL_HEADER_CMD;
            SerialRxMsg.Command = c;
            SerialRxMsg.Checksum ^= c;
        }
        else if ((SerialRxMsg.State == SERIAL_HEADER_CMD) && (SerialRxMsg.Offset < SerialRxMsg.DataSize))
        {
            SerialRxMsg.Checksum ^= c;
            SerialRxMsg.DataBuffer[SerialRxMsg.Offset++] = c;
        }
        else if ((SerialRxMsg.State == SERIAL_HEADER_CMD) && (SerialRxMsg.Offset >= SerialRxMsg.DataSize))
        {
            SerialRxMsg.State = SERIAL_IDLE;

            if (SerialRxMsg.Checksum == c) // compare calculated and transferred checksum
            {
                Serial_EvaluateCommand();
            }

            bytesCount = 0;
        }
    }
}

//**************************************************************************
// Reads a string from the serial interface
void Serial_ReadStr(char *str, unsigned char size)
{
    do
    {
        *str = (char)Serial_ReadByte();
    } while (*str++);
}

//**************************************************************************
void Serial_ReadArray(unsigned char *data, unsigned char size)
{
    while (size--)
        *data++ = Serial_ReadByte();
}

//**************************************************************************
uint8_t Serial_ReadByte(void)
{
    return SerialRxMsg.DataBuffer[SerialRxMsg.IndexBuf++];
}

//**************************************************************************
uint16_t Serial_Read16(void)
{
    uint16_t u16 = Serial_ReadByte();
    u16 |= (uint16_t)Serial_ReadByte() << 8;
    return u16;
}

//**************************************************************************
uint32_t Serial_Read32(void)
{
    uint32_t u32 = Serial_Read16();
    u32 |= (uint32_t)Serial_Read16() << 16;
    return u32;
}

//**************************************************************************
void Serial_WriteStr(char *str)
{
    do
    {
        Serial_WriteByte((unsigned char)*str++);
    } while (*str);
		
		Serial_WriteByte(0);
}

//**************************************************************************
void Serial_WriteArray(unsigned char *data, unsigned char size)
{
    while (size--)
        Serial_WriteByte(*data++);
}

//**************************************************************************
void Serial_WriteByte(unsigned char a)
{
    UART_Write(a);
    SerialTxMsg.Checksum ^= a;
}

//**************************************************************************
void Serial_Write16(uint16_t u16)
{
    Serial_WriteByte(u16);
    Serial_WriteByte(u16 >> 8);
}

//**************************************************************************
void Serial_Write32(uint32_t u32)
{
    Serial_Write16(u32);
    Serial_Write16(u32 >> 16);
}

//**************************************************************************
void Serial_HeadCommand(uint8_t cmd, uint8_t size)
{
    SerialTxMsg.Checksum = 0;
    SerialTxMsg.Command = cmd;

    UART_Write('$');
    UART_Write('M');
    UART_Write('>');
    Serial_WriteByte(size); // start calculating a new checksum
    Serial_WriteByte(cmd);
    //    Serial_WriteByte(PRODUCT_ID);
}

//**************************************************************************
void Serial_HeadReply(uint8_t size)
{
    Serial_HeadResponse(0, size);
}

//**************************************************************************
void Serial_HeadResponse(uint8_t err, uint8_t size)
{
    SerialTxMsg.Checksum = 0;
    SerialTxMsg.DataSize = size;

    UART_Write('$');
    UART_Write('M');
    UART_Write(err ? '!' : '>');
    Serial_WriteByte(size); // start calculating a new checksum
    Serial_WriteByte(SerialTxMsg.Command);
    //    Serial_WriteByte(PRODUCT_ID);
}

//**************************************************************************
void Serial_WriteChecksum(void)
{
    UART_Write(SerialTxMsg.Checksum);
}

//**************************************************************************
