#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define CONCATENATE_WITH_SPACE(buffer, string) strcat(buffer, string); strcat(buffer, " ")

#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "BMW"
#define FIRMWARE_MODEL "Mini Cooper, E60, E84, E87, E90, 3-5 Serie"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // 1.1rc -> 1.2r
#define HARDWARE_VER "HW V1.0r"

#define BMW_LEN_RESPONSE_BYTES 15

enum {
	EIS_NO_KEY = -1,
	EIS_TURN_ON,
	EIS_CAR_START,
	EIS_INSERT_KEY, 
	EIS_EXIT_KEY,
	
};

enum { 
	CMD_INSERT_KEY = 0x04, 
	CMD_TURNON_KEY = 0x45, 
	CMD_EXIT_KEY   = 0x47,
};

typedef struct {
	uint8_t waitting;
	uint8_t len;
	uint8_t data[BMW_LEN_RESPONSE_BYTES];
} BmwResponseData_t;

typedef struct {
	uint32_t waitting;
	uint8_t  len;
	uint8_t  sendFlag;
	uint8_t  data[BMW_LEN_RESPONSE_BYTES];
} BmwResponse_t;

typedef enum  { 
	ESL_SERIAL_IDLE, 
	ESL_SERIAL_HEADER_START, 
	ESL_SERIAL_HEADER_LEN,
	ESL_SERIAL_HEADER_ID,	
	ESL_SERIAL_HEADER_CMD,
	ESL_SERIAL_DATA,
	ESL_SERIAL_CRC
} ESL_SERIAL_Status_t;

typedef struct {
	uint8_t  Command;
	uint8_t  Id;
	uint8_t  DataSize;
	uint8_t  InDataBuffer[10];	
	uint8_t  Checksum;
} BmwFrame_t;

typedef struct {
	ESL_SERIAL_Status_t Status;
	uint8_t  IndexBuf;
	uint8_t  Offset;
	BmwFrame_t frame;
} BmwSerial_RxMSG_t;

//**************************************************
//* ELV functions
//**************************************************
void ELV_Init(void);
void ELV_Update(void);
void ELV_USART_Receive(uint8_t len);
void ELV_USART_Send(uint8_t *data, uint8_t len);;
void ELV_EvaluateCommand(void);

#endif
