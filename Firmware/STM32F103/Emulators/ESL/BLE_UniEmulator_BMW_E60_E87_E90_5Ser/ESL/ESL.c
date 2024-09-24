#include "main.h"
#include "ESL.h"
#include "SerialFIFO.h"
#include "SmartLED.h"
#include <string.h>

//**************************************************
//* Global Variables
//**************************************************
int8_t BmwCurrentCmd;
uint8_t BmwExitKeyFlag, BmwExitKeyIndex;
BmwResponse_t BmwResponse;
static BmwSerial_RxMSG_t BmwSerialRxMSG;


const BmwResponseData_t BmwResponseData[] = { // BMW E90
	// wait, len, data..
	{ 30, 15, {0x80, 0x0D, 0x00, 0x45, 0x59, 0xB2, 0x11, 0x04, 0x00, 0x00, 0x6B, 0x98, 0x12, 0xFA, 0x2D } }, // TURN_ON
	{ 15, 15, {0x80, 0x0D, 0x00, 0x45, 0x59, 0xB2, 0x12, 0x84, 0x00, 0x00, 0x00, 0xDC, 0x04, 0x65, 0x08 } }, // EIS_CAR_START
	{  2, 15, {0x80, 0x0D, 0x00, 0x53, 0x44, 0x13, 0x4A, 0x61, 0x07, 0x10, 0x10, 0x22, 0x08, 0x56, 0xD9 } }, // EIS_INSERT_KEY
	
	{  0,  7, {0x80, 0x05, 0x00, 0x48, 0x30, 0x2C, 0xD1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // EIS_EXIT_KEY EIS_CAR_STOP or EIS_CLOSE_DOOR
	{ 40,  7, {0x80, 0x05, 0x00, 0x48, 0x50, 0x2A, 0xB7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // EIS_EXIT_KEY EIS_CAR_STOP or EIS_CLOSE_DOOR
	{ 40,  7, {0x80, 0x05, 0x00, 0x48, 0x60, 0x3A, 0x97, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // EIS_EXIT_KEY EIS_CAR_STOP or EIS_CLOSE_DOOR
	{ 40, 15, {0x80, 0x0D, 0x00, 0x45, 0xA6, 0x7E, 0x82, 0x0A, 0x00, 0x00, 0x52, 0xF5, 0x3C, 0x13, 0x10 } }, // EIS_EXIT_KEY EIS_CAR_STOP or EIS_CLOSE_DOOR
	{ 40, 15, {0x80, 0x0D, 0x00, 0x45, 0xA6, 0x7E, 0xA3, 0x8A, 0x00, 0x00, 0xA5, 0xDD, 0xB5, 0x26, 0xD2 } }, // EIS_EXIT_KEY EIS_CAR_STOP or EIS_CLOSE_DOOR
};
//**************************************************
void ELV_Init(void)
{
	UART3_Init();

	BmwResponse.sendFlag = 1;
	BmwResponse.waitting = HAL_GetTick() + (uint32_t)BmwResponseData[EIS_TURN_ON].waitting;
	BmwResponse.len = BmwResponseData[EIS_TURN_ON].len;
	memcpy(BmwResponse.data, &BmwResponseData[EIS_TURN_ON].data[0], BmwResponse.len);
}
//**************************************************
void ELV_Update(void)
{
	if(BmwResponse.sendFlag == 1)
	{
		if(HAL_GetTick() >= BmwResponse.waitting) 
		{
			BmwResponse.sendFlag = 0;
			ELV_USART_Send(BmwResponse.data, BmwResponse.len);
			
			if((BmwExitKeyFlag) && (BmwExitKeyIndex < 8))
			{
				if(++BmwExitKeyIndex >= 7)
				{
					BmwExitKeyFlag = 0;
				}
				
				BmwResponse.sendFlag = 1;
				BmwResponse.waitting = HAL_GetTick() + (uint32_t)BmwResponseData[BmwExitKeyIndex].waitting;
				BmwResponse.len = BmwResponseData[BmwExitKeyIndex].len;
				memcpy(BmwResponse.data, BmwResponseData[BmwExitKeyIndex].data, BMW_LEN_RESPONSE_BYTES);
				
			}
		}
	}

}
//**************************************************
void ELV_USART_Receive(uint8_t len)
{
	while (len--) 
	{
		unsigned char c = UART3_Read();
		
		switch((unsigned char)BmwSerialRxMSG.Status) 
		{
			case ESL_SERIAL_IDLE:
				BmwSerialRxMSG.Status = (c == 0x00) ? ESL_SERIAL_HEADER_START : ESL_SERIAL_IDLE;
			break;
			
			case ESL_SERIAL_HEADER_START:
				BmwSerialRxMSG.frame.DataSize = c - 1;
				BmwSerialRxMSG.frame.Checksum = c;
			  BmwSerialRxMSG.Offset = 0;
			  BmwSerialRxMSG.IndexBuf = 0;
				BmwSerialRxMSG.Status = ESL_SERIAL_HEADER_LEN;
			break;
			
			case ESL_SERIAL_HEADER_LEN:
				BmwSerialRxMSG.frame.Id = c;
				BmwSerialRxMSG.frame.Checksum ^= c;
			  BmwSerialRxMSG.frame.DataSize -= 1;
			  BmwSerialRxMSG.Status = ESL_SERIAL_HEADER_ID;
			break;
				
			case ESL_SERIAL_HEADER_ID:
				BmwSerialRxMSG.frame.Command = c;
			  BmwSerialRxMSG.frame.Checksum ^= c;
			  BmwSerialRxMSG.frame.DataSize -= 1;
			  BmwSerialRxMSG.Status = ESL_SERIAL_HEADER_CMD;
			break;
			
			case ESL_SERIAL_HEADER_CMD:
				if (BmwSerialRxMSG.Offset < BmwSerialRxMSG.frame.DataSize) {
					BmwSerialRxMSG.frame.Checksum ^= c;
					BmwSerialRxMSG.frame.InDataBuffer[BmwSerialRxMSG.Offset++] = c;
				}
				else {
					BmwSerialRxMSG.Status = ESL_SERIAL_IDLE;
					if (BmwSerialRxMSG.frame.Checksum == c) { // compare calculated and transferred checksum
						 ELV_EvaluateCommand();
					}
				}
			break;
				
			default:
				BmwSerialRxMSG.Status = ESL_SERIAL_IDLE;
			break;
		}
	}
}
//**************************************************
void ELV_USART_Send(uint8_t *data, uint8_t len)
{
	while (len--) UART3_Write(*data++);
}
//**************************************************
void ELV_EvaluateCommand(void)
{
	BmwCurrentCmd = EIS_NO_KEY;
	
	switch(BmwSerialRxMSG.frame.Command) {
		
		case CMD_INSERT_KEY:
			if(BmwSerialRxMSG.frame.InDataBuffer[0] == 0x53)
			{
				BmwCurrentCmd = EIS_INSERT_KEY;
        SmartLED_SetMode(LED_G, LED_MODE_ON);
			}
			else if(BmwSerialRxMSG.frame.InDataBuffer[0] == 0x45)
			{
				BmwCurrentCmd = EIS_CAR_START;
			}
			
		break;
		
		case CMD_EXIT_KEY:
			BmwCurrentCmd = EIS_EXIT_KEY;
		  SmartLED_SetMode(LED_G, LED_MODE_BLINK_SLOW);
			BmwExitKeyFlag = 1;
			BmwExitKeyIndex = 3;
		break;

	}
	
	if(BmwCurrentCmd != EIS_NO_KEY) {
		BmwResponse.sendFlag = 1;
		BmwResponse.waitting = HAL_GetTick() + (uint32_t)BmwResponseData[BmwCurrentCmd].waitting;
		BmwResponse.len = BmwResponseData[BmwCurrentCmd].len;
		memcpy(BmwResponse.data, BmwResponseData[BmwCurrentCmd].data, BMW_LEN_RESPONSE_BYTES);
	}
}
//**************************************************

