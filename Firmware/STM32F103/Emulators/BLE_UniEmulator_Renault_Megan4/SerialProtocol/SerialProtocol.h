/****************************************************** 
 **  Project   : Serial Protocol
 **  Version   : 1.4.1
 **  Created on: 01-02-2017
 **  Author    : Eng Abdullah Jalloul
 **  Ver 1.4.0 : Add support for ProductID
 **  Ver 1.4.1 : Fixed bug in Serial_ReadStr
 ******************************************************/

#ifndef _SERIAL_PROTOCOL_H_
#define _SERIAL_PROTOCOL_H_

#include <stdint.h>

// Version of the serial protocol and buffer size definition
#define SERIAL_PROTOCOL_VER         141 // Version 1.4.1
#define SERIAL_PROTOCOL_BUF_SIZE    254

#ifndef UART_Available
	#define UART_Available    UART1_Available
#endif

#ifndef UART_Read
	#define UART_Read    UART1_Read
#endif

#ifndef UART_Write
	#define UART_Write(c)    if((UART1_Write(c) & FIFO_FULL) == FIFO_FULL) { HAL_Delay(200); }
#endif


// Enumeration of possible serial commands
typedef enum {
    SERIAL_PING = 0,           // Ping command
    SERIAL_READ_INFO,          // Command to read info
		SERIAL_READ_UNIQUE,
		SERIAL_RESPONSE_UNIQUE,
	
		SERIAL_ACTIVATE_DEVICE = 110,
		SERIAL_ACTIVATE_BOOTLOADER,
} SERIAL_COMMAND_t;

// Enumeration of possible states in the serial communication protocol
typedef enum {
    SERIAL_IDLE,               // Idle state
    SERIAL_HEADER_START,       // Start of header detected
    SERIAL_HEADER_M,           // 'M' character of header detected
    SERIAL_HEADER_ARROW,       // '>' character of header detected
    SERIAL_HEADER_SIZE,        // Data size field of header
    SERIAL_HEADER_CMD,         // Command field of header
    SERIAL_HEADER_ID           // Product ID field of header
} SERIAL_STATE_t;

// Structure to hold details of received serial messages
typedef struct {
    SERIAL_STATE_t State;             // Current state of serial communication
    uint8_t  Command;                       // Command received
    uint8_t  DataSize;                      // Size of the data received
    uint8_t  Checksum;                      // Checksum of the data received
    uint8_t  IndexBuf;                      // Index buffer (used for parsing)
    uint8_t  Offset;                        // Offset for data reading
    uint8_t  DataBuffer[SERIAL_PROTOCOL_BUF_SIZE]; // Buffer to hold incoming data
} SERIAL_RxMSG_t;

// Structure to hold details of transmitted serial messages
typedef struct {
    uint8_t  Command;                       // Command to be sent
    uint8_t  DataSize;                      // Size of the data to be sent
    uint8_t  Checksum;                      // Checksum of the data to be sent
} SERIAL_TxMSG_t;

// Function to evaluate and respond to received commands
void Serial_EvaluateCommand(void);

// Function to decode the received serial data
void Serial_Decode(void);

// Reading functions
void Serial_ReadStr(char *str, unsigned char size);   // Read a string
void Serial_ReadArray(unsigned char *data, unsigned char size); // Read an array
uint8_t  Serial_ReadByte(void);                       // Read a single byte
uint16_t Serial_Read16(void);                         // Read 16 bits
uint32_t Serial_Read32(void);                         // Read 32 bits

// Writing functions
void Serial_WriteStr(char *str);                      // Write a string
void Serial_WriteArray(unsigned char *data, unsigned char size); // Write an array
void Serial_WriteByte(unsigned char a);               // Write a single byte
void Serial_Write16(uint16_t u16);                    // Write 16 bits
void Serial_Write32(uint32_t u32);                    // Write 32 bits

// Functions to handle headers for commands, replies, and responses
void Serial_HeadCommand(uint8_t cmd, uint8_t size);   // Send a header for a command
void Serial_HeadReply(uint8_t size);                  // Send a header for a reply
void Serial_HeadResponse(uint8_t err, uint8_t size);  // Send a header for a response

// Function to write checksum to the serial interface
void Serial_WriteChecksum(void);

#endif
