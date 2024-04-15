#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define HARDWARE_VER "HW V1.0r"
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Mercedes Benz"
#define FIRMWARE_MODEL "Sprinter, Vito, VW Crafter, W169, W245, W202, W203, W208, W209, W210, W211, W639, W906"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // 1.1rc -> 1.2r

#define MERCEDES_LEN_RESPONSE_BYTES 11

enum {
	EIS_NO_KEY,
	EIS_TURN_ON, 
	EIS_INSERT_KEY, 
	EIS_EXIT_KEY, 
	EIS_ACTIVATE_KEY,
	EIS_CRAFTER_TURN_ON,
	EIS_CRAFTER_INSERT_KEY_1,
	EIS_CRAFTER_INSERT_KEY_2,
	EIS_CRAFTER_EXIT_KEY, 
};

typedef struct {
	uint8_t waitting;
	uint8_t data[MERCEDES_LEN_RESPONSE_BYTES];
} MbResponseData_t;

typedef struct {
	uint32_t waitting;
	uint8_t  sendFlag;
	uint8_t  data[MERCEDES_LEN_RESPONSE_BYTES];
} MbResponse_t;

typedef enum  { 
	ESL_SERIAL_IDLE, 
	ESL_SERIAL_HEADER_START, 
	ESL_SERIAL_HEADER_CMD,
	ESL_SERIAL_HEADER_ID,	
	ESL_SERIAL_DATA,
	ESL_SERIAL_CRC
} ESL_SERIAL_Status_t;

typedef struct {
//	uint32_t Timestamp;
//	uint8_t  Header;
	uint8_t  Command;
	uint8_t  Id;
	uint8_t  DataSize;
	uint8_t  InDataBuffer[8];	
	uint8_t  Checksum;
} MbFrame_t;

typedef struct {
	ESL_SERIAL_Status_t Status;
	uint8_t  IndexBuf;
	uint8_t  Offset;
	MbFrame_t MbFrame;
} MbSerial_RxMSG_t;

//**************************************************
//* Mercedes ESL functions
//**************************************************
void MB_Init(void);
void MB_Update(void);
void MB_USART_Receive(uint8_t len);
void MB_USART_Send(uint8_t *data, uint8_t len);;
void MB_EvaluateCommand(void);

#endif
