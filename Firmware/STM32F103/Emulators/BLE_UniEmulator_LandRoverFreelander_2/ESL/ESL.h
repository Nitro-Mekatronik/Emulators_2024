#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define HARDWARE_VER "HW V1.0r"
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Land Rover Freelander 2"
#define FIRMWARE_MODEL "2006-2014 LR023801, LR024607, LR020964, LR037025, P30665124, 6G9N-3F880-DC"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // 1.1rc -> 1.2r

enum {LIN_WRITE = 0, LIN_READ};

typedef struct {
	uint8_t Hi, Lo;
} lock_t;

typedef enum { 
	LIN_IDLE, 
	LIN_HEADER_START,
  LIN_HEADER_SYNC,	
	LIN_HEADER_ID,	
	LIN_RECEIVE_DATA,
	LIN_RECEIVE_CRC
} LIN_Status_t;

typedef struct {
	uint8_t  Id;
	uint8_t  Index;
	uint8_t  DataSize;
	uint8_t  DataBuffer[8];	
	uint8_t  Checksum;
} LinFrame_t;

typedef struct {
	LIN_Status_t Status;
	uint8_t  IndexBuf;
	uint8_t  Offset;
	LinFrame_t frame;
} Serial_RxMSG_t;

void ESL_Update(void);
void ESL_UART_Receive(uint8_t len);
void ESL_EvaluateCommand(void);

uint8_t LIN_checkID(uint8_t id);
void LIN_SendData(LinFrame_t *frame);
uint8_t LIN_CalcChecksum(LinFrame_t *frame);

#endif
