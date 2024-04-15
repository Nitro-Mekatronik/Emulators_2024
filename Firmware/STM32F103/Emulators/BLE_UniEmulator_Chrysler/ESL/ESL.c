#include "main.h"
#include "ESL.h"
#include "SmartLED.h"
#include "LIN_Slave.h"
#include <string.h>


//**************************************************

LIN_FRAME_t linFrame;

uint8_t eslStatus;
uint8_t eslResIndex, eslResOffset;
// LIN ID range & Data Length (ID 0x00-0xF1: 2 bytes, 0x20-0x2F: 4 bytes, 0x30-0x3F: 8 bytes)

eslData_t eslData[] = {
	{ 0x31, 0xB1, { 0x40, 0x00, 0x6B, 0x6C, 0x62, 0x37, 0xC2, 0xC3 }}, // crc = 0xC7 (RES status)
	{ 0x34, 0xB4, { 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }}, // crc = 0xEF (RES open)
  { 0x34, 0xB4, { 0x42, 0x00, 0x5A, 0x85, 0x17, 0x84, 0x46, 0x40 }}, // crc = 0xBB (RES open)
	{ 0x31, 0xB1, { 0x4C, 0x00, 0x1B, 0x31, 0x36, 0xE1, 0xB5, 0x61 }}, // crc = 0x38 (RES status)
	{ 0x34, 0xB4, { 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }}, // crc = 0xEF (RES close)
  { 0x34, 0xB4, { 0x41, 0x00, 0x06, 0x07, 0x50, 0xC0, 0x5F, 0x81 }}  // crc = 0xBF (RES close)
};

void ESL_Init(void) {
	UB_LIN_Slave_Init();
}

void ESL_Receive(void) {
	switch(linFrame.frame_id)
			{
				case 0x73: // RX pid = 0x33 : status req
					linFrame.data_len = 8; // LIN ID range & Data Length (ID 0x00-0xF1: 2 bytes, 0x20-0x2F: 4 bytes, 0x30-0x3F: 8 bytes)
				  if(UB_LIN_ReceiveData(&linFrame) == LIN_OK)
				  {
				  	__ASM("nop");
				  }
				break;
				
				case 0xB1: // TX pid = 0x31 : status req
					linFrame.data_len = 8;
				
					if(eslStatus) 
						memcpy(linFrame.data, eslData[3].data, 8);
					else 
						memcpy(linFrame.data, eslData[0].data, 8);
					
					UB_LIN_SendData(&linFrame);
				break;
					
				case 0xF0: // pid = 0x30
					linFrame.data_len = 8; // LIN ID range & Data Length (ID 0x00-0xF1: 2 bytes, 0x20-0x2F: 4 bytes, 0x30-0x3F: 8 bytes)
				  if(UB_LIN_ReceiveData(&linFrame) == LIN_OK)
				  {
				  	eslResIndex = 0;
						if(linFrame.data[0] == 0x01) // inser key
						{
							eslResOffset = 1;
							eslStatus = 1;
							SmartLED_SetMode(LED_G, LED_MODE_ON);
						}
						else if(linFrame.data[0] == 0x02) // exit key
						{
							eslStatus = 0;
							eslResOffset = 4;
							SmartLED_SetMode(LED_G, LED_MODE_OFF);
						}
				  }
					break;
					
				case 0xB4: // pid = 0x34
					eslResIndex &= 0x01;
					memcpy(linFrame.data, eslData[eslResIndex + eslResOffset].data, 8);
					UB_LIN_SendData(&linFrame);
					eslResIndex++;
				break;
				
			}
}

void ESL_Update(void) {
	
}
