/******************************************************
 **  Project   : Serial Protocol
 **  Version   : 1.4.1
 **  Created on: 01-02-2017
 **  Author    : Eng Abdullah Jalloul
 **  Ver 1.4.0 : add Support PRODUCT_ID
 **  Ver 1.4.1 : Fixed bug in Serial_ReadString
 ******************************************************/

#include "SerialProtocol.h"
#include "Bootloader.h"
#include "License.h"
#include "SerialFIFO.h"
#include "SmartLED.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1
#define PRODUCT_ID 0 // zero for all devices


//**************************************************************************

const uint8_t BOOTLOADER_Version[] = {BOOTLOADER_MAJOR, BOOTLOADER_MINOR};

// https://github.com/gbesnard/stm32f1-bootloader/tree/master

//**************************************************************************
// Structures to hold details of received and transmitted serial messages
SERIAL_RxMSG_t SerialRxMsg;
SERIAL_TxMSG_t SerialTxMsg;

//**************************************************************************

extern Flash_Status flashStatus;

bool isRealServer = false;
uint32_t challengeServerTimeout;
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

    case SERIAL_READ_INFO:
        Serial_HeadReply(sizeof(BOOTLOADER_Version) + 2);
        Serial_WriteByte(0x01); // from Bootloader
        Serial_WriteByte(SERIAL_PROTOCOL_VER);
        Serial_WriteArray((uint8_t *)BOOTLOADER_Version, sizeof(BOOTLOADER_Version));
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
            challengeServerTimeout = HAL_GetTick() + 300000; // Timeout = 1000 * 60 * 5 minutes
        }

        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)isRealServer);
        break;
    }

    case SERIAL_ERASE_FLASH_MEMORY:
        if (flashStatus != FLASH_UNLOCKED)
        {
            eraseMemory();
        }
        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)flashStatus);
        break;

    case SERIAL_UNLOCK_FLASH_AND_ERASE:
        unlockFlashAndEraseMemory();
        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)flashStatus);
        break;

    case SERIAL_FLASHING_START: {

        Flashing_Status status = FLASHING_ERROR_MEMORY_LOCKED;

        if (flashStatus == FLASH_UNLOCKED)
        {
            SmartLED_SetBlink(LED_B, 50, 1);
            // Calculate the number of remaining words to flash
            uint8_t wordsRemaining = (SerialRxMsg.DataSize - SerialRxMsg.IndexBuf) / sizeof(uint32_t);
            // Continue fetching and flashing words until all are processed
            while (wordsRemaining > 0)
            {
                // Assuming Serial_Read32 reads the next 32-bit word from the current IndexBuf position
                uint32_t dataToFlash = Serial_Read32(); // Fetch the data to be flashed
                status = flashWord(dataToFlash);        // Flash the data
                if (status != FLASHING_OK)
                {
                    // Handle flashing error
                    break;
                }
                // decrement the word count
                wordsRemaining--;
            }
        }

        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)status);
    }
    break;

    case SERIAL_FLASHING_FINISH:
        if (flashStatus == FLASH_UNLOCKED)
        {
            lockFlash();
            jumpToApp();
        }
        break;

    case SERIAL_FLASHING_ABORT:
        if (flashStatus == FLASH_UNLOCKED)
        {
            lockFlash();
            eraseMemory();
        }
        Serial_HeadReply(1);
        Serial_WriteByte((uint8_t)flashStatus);
        break;

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
        else if (SerialRxMsg.State == SERIAL_HEADER_CMD)
        {

            if (c == PRODUCT_ID)
            {
                SerialRxMsg.Checksum ^= c;
                SerialRxMsg.State = SERIAL_HEADER_ID;
            }
            else
            {
                SerialRxMsg.Command = 0;
                SerialRxMsg.State = SERIAL_IDLE;
            }
        }
        else if ((SerialRxMsg.State == SERIAL_HEADER_ID) && (SerialRxMsg.Offset < SerialRxMsg.DataSize))
        {
            SerialRxMsg.Checksum ^= c;
            SerialRxMsg.DataBuffer[SerialRxMsg.Offset++] = c;
        }
        else if ((SerialRxMsg.State == SERIAL_HEADER_ID) && (SerialRxMsg.Offset >= SerialRxMsg.DataSize))
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
    Serial_WriteByte(PRODUCT_ID);
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
    Serial_WriteByte(PRODUCT_ID);
}

//**************************************************************************
void Serial_WriteChecksum(void)
{
    UART_Write(SerialTxMsg.Checksum);
}

//**************************************************************************
