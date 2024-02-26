#include "main.h"
#include "ESL.h"
#include "SerialFIFO.h"
#include "SmartLED.h"
#include <string.h>

//**************************************************
//* Global Variables
//**************************************************
uint8_t MB_CurrentCmd;
MbResponse_t MbResponse;
static MbSerial_RxMSG_t MbSerialRxMSG;
extern void startPlayback(void);
const MbResponseData_t MbResponseData[] = {
	{ 10, {0xA3, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D} }, // MB_W906 EIS_TURN_ON
	{ 10, {0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34} }, // EIS_INSERT_KEY
	{ 10, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80} }, // EIS_EXIT_KEY
	{ 15, {0xA1, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B} }, // EIS_ACTIVATE_KEY
	{ 10, {0x5E, 0x00, 0x5F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D} }, // EIS_CRAFTER_TURN_ON
	{ 89, {0xCC, 0x01, 0xCF, 0x53, 0x36, 0x2A, 0x7A, 0x05, 0xF0, 0xFE, 0xBC} }, // EIS_CRAFTER_INSERT_KEY_1
	{ 89, {0xCB, 0x02, 0xCF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C} }, // EIS_CRAFTER_INSERT_KEY_2
	{ 89, {0x7F, 0x00, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C} }  // EIS_CRAFTER_EXIT_KEY
};
//**************************************************
void MB_Init(void)
{
	UART3_Init();
	
}
//**************************************************
void MB_Update(void)
{
	if(MbResponse.sendFlag == 1)
	{
		if(HAL_GetTick() >= MbResponse.waitting) 
		{
			MbResponse.sendFlag = 0;
			MB_USART_Send(MbResponse.data, MERCEDES_LEN_RESPONSE_BYTES);
		}
	}
}
//**************************************************
void MB_USART_Receive(uint8_t len)
{
	while (len--) 
	{
		unsigned char c = UART3_Read();
		
		switch((unsigned char)MbSerialRxMSG.Status) 
		{
			case ESL_SERIAL_IDLE:
				MbSerialRxMSG.Status = (c == 0xFF || c == 0xF0) ? ESL_SERIAL_HEADER_START : ESL_SERIAL_IDLE;
			break;
			
			case ESL_SERIAL_HEADER_START:
				MbSerialRxMSG.MbFrame.Command = c;
				MbSerialRxMSG.MbFrame.Checksum = c;
				MbSerialRxMSG.Status = ESL_SERIAL_HEADER_CMD;
			break;
			
			case ESL_SERIAL_HEADER_CMD:
					MbSerialRxMSG.MbFrame.Id = c;
					MbSerialRxMSG.MbFrame.Checksum += c;
					MbSerialRxMSG.MbFrame.DataSize = 8;
					MbSerialRxMSG.Offset = 0;
					MbSerialRxMSG.IndexBuf = 0;
			    MbSerialRxMSG.Status = ESL_SERIAL_DATA;
			break;
				
			case ESL_SERIAL_DATA:
				if (MbSerialRxMSG.Offset <  MbSerialRxMSG.MbFrame.DataSize) {
					MbSerialRxMSG.MbFrame.Checksum += c;
					MbSerialRxMSG.MbFrame.InDataBuffer[MbSerialRxMSG.Offset++] = c;
				}
				else {
					MbSerialRxMSG.Status = ESL_SERIAL_IDLE;
					if (MbSerialRxMSG.MbFrame.Checksum == c) { // compare calculated and transferred checksum
						 MB_EvaluateCommand();
					}
				}
			break;
				
			default:
				MbSerialRxMSG.Status = ESL_SERIAL_IDLE;
			break;
		}
	}
}
//**************************************************
void MB_USART_Send(uint8_t *data, uint8_t len)
{
	while (len--) UART3_Write(*data++);
}
//**************************************************
void MB_EvaluateCommand(void)
{
	MB_CurrentCmd = EIS_NO_KEY;
	
	switch(MbSerialRxMSG.MbFrame.Command) {
		
		case 0xA2:
			MB_CurrentCmd = EIS_TURN_ON;
		break;
		
		case 0xA0:
			MB_CurrentCmd = EIS_ACTIVATE_KEY;
		break;
		
		case 0x30:
		case 0x31:
			MB_CurrentCmd = EIS_INSERT_KEY;
		  SmartLED_SetMode(LED_G, LED_MODE_ON);
//		  startPlayback();

		break;

		case 0x42:
			MB_CurrentCmd = EIS_EXIT_KEY;
		  SmartLED_SetMode(LED_G, LED_MODE_BLINK_SLOW);
//		  startPlayback();

		break;

		case 0x5F:
			MB_CurrentCmd = EIS_CRAFTER_TURN_ON;
		break;
		
		case 0xCF:
			if(MbSerialRxMSG.MbFrame.Id == 1) {
				MB_CurrentCmd = EIS_CRAFTER_INSERT_KEY_1;
			} else {
				MB_CurrentCmd = EIS_CRAFTER_INSERT_KEY_2;
			}
//			 startPlayback();
			 SmartLED_SetMode(LED_G, LED_MODE_ON);
		break;
		
		case 0xBD:
			MB_CurrentCmd = EIS_CRAFTER_EXIT_KEY;
		  SmartLED_SetMode(LED_G, LED_MODE_BLINK_SLOW);
		break;
	}
	
	if(MB_CurrentCmd != EIS_NO_KEY) {
		MbResponse.sendFlag = 1;
		MbResponse.waitting = HAL_GetTick() + (uint32_t)MbResponseData[MB_CurrentCmd-1].waitting;
		memcpy(MbResponse.data, MbResponseData[MB_CurrentCmd-1].data, MERCEDES_LEN_RESPONSE_BYTES);
	}
}
//**************************************************

