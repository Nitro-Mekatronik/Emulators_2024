/******************************************************
 **  Project   : Serial Protocol
 **  Version   : 1.5.0
 **  Created on: 01-02-2017
 **  Author    : Eng Abdullah Jalloul
 **  Ver 1.4.0 : Add support for ProductID
 **  Ver 1.4.1 : Fixed bug in Serial_ReadStr
 **  Ver 1.5.0 : 06-05-2024 Supported multi UART, remove ProductID, Added command broadcasting, and enhanced serial send functions
 **  Ver 1.5.1 : 07-05-2024 Added ErrorReceived
 ******************************************************/

#ifndef _SERIAL_PROTOCOL_H_
#define _SERIAL_PROTOCOL_H_

#include <stdbool.h>
#include <stdint.h>

// Version of the serial protocol and buffer size definition
#define SERIAL_PROTOCOL_VER 151 // Version 1.5.1
#define SERIAL_PROTOCOL_BUF_SIZE 250

#define CONNECTION_TIMEOUT 2000 // ms

#define SERIAL_PROTOCOL_MAX 2
#define BROADCAST_MAX_COMMAND_LIST 10

// Enumeration of possible serial commands
typedef enum
{
    SERIAL_PING = 0,  // Ping command
    SERIAL_READ_INFO, // Command to read info
    SERIAL_READ_UNIQUE,
    SERIAL_RESPONSE_UNIQUE,

    SERIAL_ACTIVATE_DEVICE = 110,
    SERIAL_ACTIVATE_BOOTLOADER,
} SERIAL_COMMAND_t;

// Enumeration of possible states in the serial communication protocol
typedef enum
{
    SERIAL_IDLE,         // Idle state
    SERIAL_HEADER_START, // Start of header detected
    SERIAL_HEADER_M,     // 'M' character of header detected
    SERIAL_HEADER_ARROW, // '>' character of header detected
    SERIAL_HEADER_SIZE,  // Data size field of header
    SERIAL_HEADER_CMD,   // Command field of header
	SERIAL_HEADER_ERR
} SERIAL_STATE_t;

// Structure to hold details of received serial messages
typedef struct
{
    SERIAL_STATE_t State;                         // Current state of serial communication
    uint8_t Command;                              // Command received
    uint8_t DataSize;                             // Size of the data received
    uint8_t Checksum;                             // Checksum of the data received
    uint8_t IndexBuf;                             // Index buffer (used for parsing)
    uint8_t Offset;                               // Offset for data reading
	uint8_t ErrorReceived;
    uint8_t DataBuffer[SERIAL_PROTOCOL_BUF_SIZE]; // Buffer to hold incoming data
} SERIAL_RxMSG_t;

// Structure to hold details of transmitted serial messages
typedef struct
{
	//uint8_t messageID;
    uint8_t Command;  // Command to be sent
    uint8_t DataSize; // Size of the data to be sent
    uint8_t Checksum; // Checksum of the data to be sent
} SERIAL_TxMSG_t;

// Structure to manage a list of broadcast commands
typedef struct
{
    uint16_t interval;                            // Interval between broadcasts in milliseconds
    uint32_t timer;                               // Timer used to keep track of broadcasting intervals
    uint8_t commands[BROADCAST_MAX_COMMAND_LIST]; // Array holding the commands to broadcast
    uint8_t number;                               // Number of commands in the list
    uint8_t index;                                // Current index in the command list
} broadcastCommandList_t;

// Structure to hold the entire serial protocol state
typedef struct
{
    bool isConnected;                            // Connection status flag
    uint32_t ConnectionTimer;                    // Timer used for connection timeout management
    SERIAL_RxMSG_t SerialRxMsg;                  // Structure to hold received serial messages
    SERIAL_TxMSG_t SerialTxMsg;                  // Structure to hold transmitted serial messages
    broadcastCommandList_t BroadcastCommandList; // List of commands to be broadcast
    uint16_t (*Available)(void);                 // Function pointer to check the number of available bytes to read
    uint8_t (*Read)(void);                       // Function pointer to read a single byte
    uint8_t (*Write)(uint8_t);                   // Function pointer to write a single byte
    void (*EvaluateCommandEvent)(uint8_t);       // Function pointer to process received commands
    void (*ConnectionLostEvent)(void);           // Function pointer to handle lost connection events
} SerialProtocol_t;

// Initializes the SerialProtocol structure
bool Serial_Protocol_Init(SerialProtocol_t *hSerialProtocol, uint16_t (*Available)(void), uint8_t (*Read)(void),
                          uint8_t (*Write)(uint8_t), void (*EvaluateCommandEvent)(uint8_t), void (*ConnectionLostEvent)(void));

// Processes incoming and outgoing serial data and handles command broadcasts and connection timeouts
bool Serial_Update(SerialProtocol_t *hSerialProtocol);


bool SerialProtocol_IsConnected(SerialProtocol_t *hSerialProtocol);

// Sets a list of commands to be broadcast at regular intervals
void Serial_SetCommandList(SerialProtocol_t *hSerialProtocol, broadcastCommandList_t *commandList);

// Sends a request with no additional data
void Serial_SendRequest(SerialProtocol_t *hSerialProtocol, uint8_t cmd);

// Sends a command and data size
void SerialProtocol_SendCmdSize(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t size);

// Sends a single byte command
void Serial_SendUint8(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t Value);

// Sends a 16-bit unsigned integer command
void Serial_SendUInt16(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint16_t Value);

// Sends a 32-bit unsigned integer command
void Serial_SendUint32(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint32_t Value);

// Sends a floating point value command
void Serial_SendFloat(SerialProtocol_t *hSerialProtocol, uint8_t cmd, float Value);

// Sends a double precision floating point value command
void Serial_SendDouble(SerialProtocol_t *hSerialProtocol, uint8_t cmd, double Value);

// Sends a string command
void Serial_SendString(SerialProtocol_t *hSerialProtocol, uint8_t cmd, char *Value);

// Sends an array of bytes as a command
void Serial_SendArray(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t *Array, uint8_t size);

// Function to decode the received serial data
bool Serial_Decode(SerialProtocol_t *hSerialProtocol);

// Reads a string from the serial interface
void Serial_ReadString(SerialProtocol_t *hSerialProtocol, char *str, unsigned char size);

// Reads an array of bytes from the serial interface
void Serial_ReadArray(SerialProtocol_t *hSerialProtocol, unsigned char *data, unsigned char size);

// Reads a single byte from the serial interface
uint8_t Serial_ReadByte(SerialProtocol_t *hSerialProtocol);

// Reads a 16-bit unsigned integer from the serial interface
uint16_t Serial_Read16(SerialProtocol_t *hSerialProtocol);

// Reads a 32-bit unsigned integer from the serial interface
uint32_t Serial_Read32(SerialProtocol_t *hSerialProtocol);

// Writes a string to the serial interface
void Serial_WriteString(SerialProtocol_t *hSerialProtocol, char *str);

// Writes an array of bytes to the serial interface
void Serial_WriteArray(SerialProtocol_t *hSerialProtocol, unsigned char *data, unsigned char size);

// Writes a single byte to the serial interface
void Serial_WriteByte(SerialProtocol_t *hSerialProtocol, unsigned char a);

// Writes a 16-bit unsigned integer to the serial interface
void Serial_Write16(SerialProtocol_t *hSerialProtocol, uint16_t u16);

// Writes a 32-bit unsigned integer to the serial interface
void Serial_Write32(SerialProtocol_t *hSerialProtocol, uint32_t u32);

// Sends a command header
void Serial_HeadCommand(SerialProtocol_t *hSerialProtocol, uint8_t cmd, uint8_t size);

// Sends a reply header
void Serial_HeadReply(SerialProtocol_t *hSerialProtocol, uint8_t size);

// Sends a response header
void Serial_HeadResponse(SerialProtocol_t *hSerialProtocol, uint8_t err, uint8_t size);

// Writes a checksum to the serial interface
void Serial_WriteChecksum(SerialProtocol_t *hSerialProtocol);

#endif /* _SERIAL_PROTOCOL_H_ */
