/******************************************************
 **  Project   : Serial Protocol
 **  Version   : 1.5.0
 **  Created on: 01-02-2017
 **  Author    : Eng Abdullah Jalloul
 **  Ver 1.4.0 : Add support for ProductID
 **  Ver 1.4.1 : Fixed bug in Serial_ReadStr
 **  Ver 1.5.0 : 06-05-2024 Supported multi UART, remove ProductID, Added command broadcasting, and enhanced serial send
 *functions
 **  Ver 1.5.1 : 07-05-2024 Added ErrorReceived
 ******************************************************/

#include "SerialProtocol.h"
#include "SerialFIFO.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//**************************************************************************
// Initializes the SerialProtocol structure
bool Serial_Protocol_Init(SerialProtocol_t *hSerialProtocol, uint16_t (*Available)(void), uint8_t (*Read)(void),
                          uint8_t (*Write)(uint8_t), void (*EvaluateCommandEvent)(uint8_t),
                          void (*ConnectionLostEvent)(void))
{
    // Check for NULL pointers
    if (!hSerialProtocol || !Available || !Read || !Write || !EvaluateCommandEvent || !ConnectionLostEvent)
        return false;

    // Initialize the entire structure to zero
    memset(hSerialProtocol, 0, sizeof(SerialProtocol_t));

    // Initialize the function pointers
    hSerialProtocol->Available = Available;
    hSerialProtocol->Read = Read;
    hSerialProtocol->Write = Write;
    hSerialProtocol->EvaluateCommandEvent = EvaluateCommandEvent;
    hSerialProtocol->ConnectionLostEvent = ConnectionLostEvent;

    // Set initial state for the receive message
    hSerialProtocol->SerialRxMsg.State = SERIAL_IDLE;

    // Set initial connection timeout timer
    hSerialProtocol->ConnectionTimer = HAL_GetTick() + CONNECTION_TIMEOUT;

    return true;
}

//**************************************************************************
// Processes incoming and outgoing serial data and handles command broadcasts and connection timeouts
bool Serial_Update(SerialProtocol_t *hSerialProtocol)
{
    uint32_t currentTime = HAL_GetTick();

    // Broadcast command handling
    if (hSerialProtocol->BroadcastCommandList.number && hSerialProtocol->isConnected)
    {
        if (currentTime >= hSerialProtocol->BroadcastCommandList.timer)
        {
            hSerialProtocol->BroadcastCommandList.timer = currentTime + hSerialProtocol->BroadcastCommandList.interval;
            hSerialProtocol->EvaluateCommandEvent(
                hSerialProtocol->BroadcastCommandList.commands[hSerialProtocol->BroadcastCommandList.index]);

            if (++hSerialProtocol->BroadcastCommandList.index >= hSerialProtocol->BroadcastCommandList.number)
                hSerialProtocol->BroadcastCommandList.index = 0;
        }
    }

    // Connection timeout handling
    if (currentTime > hSerialProtocol->ConnectionTimer)
    {
        hSerialProtocol->isConnected = false;
        hSerialProtocol->ConnectionTimer = currentTime + CONNECTION_TIMEOUT;

        hSerialProtocol->ConnectionLostEvent();
    }

    // Decode the incoming data
    return Serial_Decode(hSerialProtocol);
}

//**************************************************************************
bool SerialProtocol_IsConnected(SerialProtocol_t *hSerialProtocol)
{
    return hSerialProtocol->isConnected;
}
//**************************************************************************
// Sets a list of commands to be broadcast at regular intervals
void Serial_SetCommandList(SerialProtocol_t *hSerialProtocol, broadcastCommandList_t *commandList)
{
    if (commandList)
    {
        memcpy((void *)&hSerialProtocol->BroadcastCommandList, (void *)commandList, sizeof(broadcastCommandList_t));
        hSerialProtocol->BroadcastCommandList.timer = HAL_GetTick() + hSerialProtocol->BroadcastCommandList.interval;
    }
}

//**************************************************************************

bool Serial_Decode(SerialProtocol_t *hSerialProtocol)
{

    uint16_t bytesCount = hSerialProtocol->Available();

    while (bytesCount--)
    {
        uint8_t c = hSerialProtocol->Read();

        if (hSerialProtocol->SerialRxMsg.State == SERIAL_IDLE)
        {
            hSerialProtocol->SerialRxMsg.State = (c == '$') ? SERIAL_HEADER_START : SERIAL_IDLE;
        }
        else if (hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_START)
        {
            hSerialProtocol->SerialRxMsg.State = (c == 'M') ? SERIAL_HEADER_M : SERIAL_IDLE;
        }
        else if (hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_M)
        {
            hSerialProtocol->SerialRxMsg.State = (c == '>') ? SERIAL_HEADER_ARROW : (c == '!') ? SERIAL_HEADER_ERR : SERIAL_IDLE;
        }
        else if ((hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_ARROW) ||
                 (hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_ERR))
        {

            hSerialProtocol->SerialRxMsg.ErrorReceived = (hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_ERR);

            if (c > SERIAL_PROTOCOL_BUF_SIZE) // now we are expecting the payload size
            {
                hSerialProtocol->SerialRxMsg.State = SERIAL_IDLE;
                continue;
            }

            hSerialProtocol->SerialRxMsg.State = SERIAL_HEADER_SIZE;
            hSerialProtocol->SerialRxMsg.DataSize = c;
            hSerialProtocol->SerialRxMsg.Offset = 0;
            hSerialProtocol->SerialRxMsg.IndexBuf = 0;
            hSerialProtocol->SerialRxMsg.Checksum = c;
        }
        else if (hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_SIZE)
        {
            hSerialProtocol->SerialRxMsg.State = SERIAL_HEADER_CMD;
            hSerialProtocol->SerialRxMsg.Command = c;
            hSerialProtocol->SerialRxMsg.Checksum ^= c;
        }
        else if ((hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_CMD) &&
                 (hSerialProtocol->SerialRxMsg.Offset < hSerialProtocol->SerialRxMsg.DataSize))
        {
            hSerialProtocol->SerialRxMsg.Checksum ^= c;
            hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.Offset++] = c;
        }
        else if ((hSerialProtocol->SerialRxMsg.State == SERIAL_HEADER_CMD) &&
                 (hSerialProtocol->SerialRxMsg.Offset >= hSerialProtocol->SerialRxMsg.DataSize))
        {
            hSerialProtocol->SerialRxMsg.State = SERIAL_IDLE;

            if (hSerialProtocol->SerialRxMsg.Checksum == c) // compare calculated and transferred checksum
            {
                hSerialProtocol->isConnected = true;
                hSerialProtocol->ConnectionTimer = HAL_GetTick() + CONNECTION_TIMEOUT;
                hSerialProtocol->EvaluateCommandEvent(hSerialProtocol->SerialRxMsg.Command);
            }

            bytesCount = 0;
        }
    }

    return !hSerialProtocol->SerialRxMsg.ErrorReceived; // true:No Error , false:Error Received
}

//**************************************************************************
// Sends a request with no additional data
void Serial_SendRequest(SerialProtocol_t *hSerialProtocol, uint8_t cmd)
{
    Serial_HeadCommand(hSerialProtocol, cmd, 0);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a command and data size
void SerialProtocol_SendCmdSize(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t size)
{
    Serial_HeadCommand(hSerialProtocol, cmd, size);
}

//**************************************************************************
// Sends a single byte command
void Serial_SendUint8(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t Value)
{
    Serial_HeadCommand(hSerialProtocol, cmd, sizeof(uint8_t));
    Serial_WriteByte(hSerialProtocol, Value);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a 16-bit unsigned integer command
void Serial_SendUInt16(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint16_t Value)
{
    Serial_HeadCommand(hSerialProtocol, cmd, sizeof(uint16_t));
    Serial_Write16(hSerialProtocol, Value);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a 32-bit unsigned integer command
void Serial_SendUint32(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint32_t Value)
{
    Serial_HeadCommand(hSerialProtocol, cmd, sizeof(uint32_t));
    Serial_Write32(hSerialProtocol, Value);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a floating point value command
void Serial_SendFloat(SerialProtocol_t *hSerialProtocol, uint8_t cmd, float Value)
{
    Serial_HeadCommand(hSerialProtocol, cmd, sizeof(float));
    Serial_WriteArray(hSerialProtocol, (uint8_t *)&Value, sizeof(float));
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a double precision floating point value command
void Serial_SendDouble(SerialProtocol_t *hSerialProtocol, uint8_t cmd, double Value)
{
    Serial_HeadCommand(hSerialProtocol, cmd, sizeof(double));
    Serial_WriteArray(hSerialProtocol, (uint8_t *)&Value, sizeof(double));
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends a string command
void Serial_SendString(SerialProtocol_t *hSerialProtocol, uint8_t cmd, char *str)
{
    Serial_HeadCommand(hSerialProtocol, cmd, strlen(str));
    Serial_WriteString(hSerialProtocol, str);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Sends an array of bytes as a command
void Serial_SendArray(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t *Array, uint8_t size)
{
    Serial_HeadCommand(hSerialProtocol, cmd, size);
    Serial_WriteArray(hSerialProtocol, Array, size);
    Serial_WriteChecksum(hSerialProtocol);
}

//**************************************************************************
// Reads a string from the serial interface
void Serial_ReadString(SerialProtocol_t *hSerialProtocol, char *str, unsigned char size)
{
    // Early exit if the buffer size is zero
    if (size == 0)
        return;

    unsigned char count = 0; // Counter for the number of characters read
    char c;
    unsigned char remainingDataSize = hSerialProtocol->SerialRxMsg.DataSize - hSerialProtocol->SerialRxMsg.IndexBuf;

    while (count < size - 1 && remainingDataSize > 0)
    {
        c = (char)(hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++]);

        // Ensure index is within bounds
        if (hSerialProtocol->SerialRxMsg.IndexBuf > SERIAL_PROTOCOL_BUF_SIZE)
        {
#ifdef DEBUG
            fprintf(stderr, "Buffer overflow prevented in Serial_ReadString\n");
#endif
            break;
        }

        str[count++] = c;
        remainingDataSize--;

        // Stop reading if a null terminator is found
        if (c == '\0')
            break;
    }

    // Ensure the string is properly null-terminated
    str[count] = '\0';
}

//**************************************************************************
// Reads an array of bytes from the serial interface
void Serial_ReadArray(SerialProtocol_t *hSerialProtocol, unsigned char *data, unsigned char size)
{
    // Check if reading the specified size will cause a buffer overflow
    if (hSerialProtocol->SerialRxMsg.IndexBuf + size > hSerialProtocol->SerialRxMsg.DataSize)
    {
#ifdef DEBUG
        // Log the error or handle it according to your application's needs
        fprintf(stderr, "Buffer overflow prevented in Serial_ReadArray\n");
#endif
        return;
    }

    // Read data into the provided array
    while (size--)
    {
        *data++ = hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++];
    }
}

//**************************************************************************
// Reads a single byte from the serial interface
uint8_t Serial_ReadByte(SerialProtocol_t *hSerialProtocol)
{
    if (hSerialProtocol->SerialRxMsg.IndexBuf >= SERIAL_PROTOCOL_BUF_SIZE)
    {

#ifdef DEBUG
        // Log the error or handle it according to your application's needs
        fprintf(stderr, "Buffer overflow prevented in Serial_ReadByte\n");
#endif
        return 0; // Return zero
    }

    return hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++];
}

//**************************************************************************
// Reads a 16-bit unsigned integer from the serial interface
uint16_t Serial_Read16(SerialProtocol_t *hSerialProtocol)
{
    if (hSerialProtocol->SerialRxMsg.IndexBuf + 1 >= SERIAL_PROTOCOL_BUF_SIZE)
    {

#ifdef DEBUG
        // Log the error or handle it according to your application's needs
        fprintf(stderr, "Buffer overflow prevented in Serial_Read16\n");
#endif
        return 0; // Return zero
    }

    uint16_t u16 = hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++];
    u16 |= (uint16_t)(hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++]) << 8;
    return u16;
}

//**************************************************************************
// Reads a 32-bit unsigned integer from the serial interface
uint32_t Serial_Read32(SerialProtocol_t *hSerialProtocol)
{
    if (hSerialProtocol->SerialRxMsg.IndexBuf + 3 >= SERIAL_PROTOCOL_BUF_SIZE)
    {

#ifdef DEBUG
        // Log the error or handle it according to your application's needs
        fprintf(stderr, "Buffer overflow prevented in Serial_Read32\n");
#endif
        return 0; // Return zero
    }

    uint32_t u32 = hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++];
    u32 |= (uint32_t)(hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++]) << 8;
    u32 |= (uint32_t)(hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++]) << 16;
    u32 |= (uint32_t)(hSerialProtocol->SerialRxMsg.DataBuffer[hSerialProtocol->SerialRxMsg.IndexBuf++]) << 24;
    return u32;
}

//**************************************************************************
// Writes a string to the serial interface
void Serial_WriteString(SerialProtocol_t *hSerialProtocol, char *str)
{
    while (*str)
    {
        hSerialProtocol->Write((unsigned char)*str);
        hSerialProtocol->SerialTxMsg.Checksum ^= (unsigned char)*str++;
    };
}

//**************************************************************************
// Writes an array of bytes to the serial interface
void Serial_WriteArray(SerialProtocol_t *hSerialProtocol, unsigned char *data, unsigned char size)
{
    while (size--)
    {
        hSerialProtocol->Write(*data);
        hSerialProtocol->SerialTxMsg.Checksum ^= *data++;
    }
}

//**************************************************************************
// Writes a single byte to the serial interface
void Serial_WriteByte(SerialProtocol_t *hSerialProtocol, unsigned char a)
{
    hSerialProtocol->Write(a);
    hSerialProtocol->SerialTxMsg.Checksum ^= a;
}

//**************************************************************************
// Writes a 16-bit unsigned integer to the serial interface
void Serial_Write16(SerialProtocol_t *hSerialProtocol, uint16_t u16)
{
    hSerialProtocol->Write(u16);
    hSerialProtocol->Write((u16 >> 8) & 0xFF);
    hSerialProtocol->SerialTxMsg.Checksum ^= u16 & 0xFF;
    hSerialProtocol->SerialTxMsg.Checksum ^= ((u16 >> 8) & 0xFF);
}

//**************************************************************************
// Writes a 32-bit unsigned integer to the serial interface
void Serial_Write32(SerialProtocol_t *hSerialProtocol, uint32_t u32)
{
    hSerialProtocol->Write(u32);
    hSerialProtocol->Write((u32 >> 8) & 0xFF);
    hSerialProtocol->Write((u32 >> 16) & 0xFF);
    hSerialProtocol->Write((u32 >> 24) & 0xFF);

    hSerialProtocol->SerialTxMsg.Checksum ^= u32;
    hSerialProtocol->SerialTxMsg.Checksum ^= (u32 >> 8) & 0xFF;
    hSerialProtocol->SerialTxMsg.Checksum ^= (u32 >> 16) & 0xFF;
    hSerialProtocol->SerialTxMsg.Checksum ^= (u32 >> 24) & 0xFF;
}

//**************************************************************************
// Sends a command header
void Serial_HeadCommand(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t size)
{
    hSerialProtocol->SerialTxMsg.Checksum = 0;
    hSerialProtocol->SerialTxMsg.Command = cmd;

    hSerialProtocol->Write('$');
    hSerialProtocol->Write('M');
    hSerialProtocol->Write('>');
    Serial_WriteByte(hSerialProtocol, size); // start calculating a new checksum
    Serial_WriteByte(hSerialProtocol, cmd);
    //    Serial_WriteByte(PRODUCT_ID);
}

//**************************************************************************
// Sends a reply header
void Serial_HeadReply(SerialProtocol_t *hSerialProtocol, uint8_t size)
{
    Serial_HeadResponse(hSerialProtocol, 0, size);
}

//**************************************************************************
// Sends a response header
void Serial_HeadResponse(SerialProtocol_t *hSerialProtocol, uint8_t err, uint8_t size)
{
    hSerialProtocol->SerialTxMsg.Checksum = 0;
    hSerialProtocol->SerialTxMsg.DataSize = size;

    hSerialProtocol->Write('$');
    hSerialProtocol->Write('M');
    hSerialProtocol->Write(err ? '!' : '>');
    Serial_WriteByte(hSerialProtocol, size); // start calculating a new checksum
    Serial_WriteByte(hSerialProtocol, hSerialProtocol->SerialTxMsg.Command);
}

//**************************************************************************
// Writes a checksum to the serial interface
void Serial_WriteChecksum(SerialProtocol_t *hSerialProtocol)
{
    hSerialProtocol->Write(hSerialProtocol->SerialTxMsg.Checksum);
}

//**************************************************************************
