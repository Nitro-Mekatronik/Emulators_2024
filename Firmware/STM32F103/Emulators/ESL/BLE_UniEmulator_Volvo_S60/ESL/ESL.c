#include "main.h"
#include "ESL.h"
#include "SmartLED.h"
#include "LIN_Slave.h"
#include <string.h>


//**************************************************
uint8_t rxBuffer[8];
uint8_t request;
uint8_t eslStatus = DOOR_LOCK_OPEN;
uint8_t eslFlage = 0;

LIN_FRAME_t linFrame;

eslData_t eslData = { .msb = 0x08, .lsb = 0x01, .add = 0 };

const eslData_t eslDoorConditions[] = { // if(lsb == x) msb = y
	{ .msb = 0x08, .lsb = 0x01, .add = 0 },
	{ .msb = 0x07, .lsb = 0x04, .add = 0 },
	{ .msb = 0x87, .lsb = 0x0A, .add = 0 },
	{ .msb = 0x88, .lsb = 0x0B, .add = 0 },
	{ .msb = 0x84, .lsb = 0x18, .add = 0 }
};

const eslData_t eslStartConditions[] = { // if(lsb == x) msb = y
	{ .msb = 0x04, .lsb = 0x01, .add = 0 },
	{ .msb = 0x07, .lsb = 0x15, .add = 0 }
};

uint8_t res37Cmd[2][6] = {
	{ 0xE2, 0x78, 0xF0, 0xE1, 0xD1, 0x00 }, // DOOR_LOCK_OPEN
	{ 0xA4, 0x46, 0xE1, 0xD6, 0x62, 0x00 }  // START_BTN_KEY
};


void getCondition(const eslData_t *pConditions, uint8_t size)
{
	
	for(uint8_t n = 0; n < size; n++)
	{
		if(pConditions[n].lsb == eslData.lsb) {
			eslData.msb = pConditions[n].msb;
			eslData.add = pConditions[n].add;
			break;
		}
	}
}

void ESL_Init(void) {
	
}

void ESL_Receive(void) {
	request = CMD_NULL;

      switch (linFrame.frame_id)
      {
        case 0x11:
          linFrame.data_len = 2;
          request = LIN_RECEIVE_CMD;
          break;

        case 0xC1:
          linFrame.data_len = 2;
          request = LIN_SEND_CMD;
          break;

        case 0x37:
          linFrame.data_len = 8;
          request = LIN_SEND_CMD;
          break;

        case 0xB1:
        case 0xF5:
          linFrame.data_len = 8;
          request = LIN_RECEIVE_CMD;
          break;
      }

      if (request == LIN_RECEIVE_CMD)
      {
        if (UB_LIN_ReceiveData(&linFrame) == LIN_OK)
        {
          switch (linFrame.frame_id)
          {
            case 0x11:
              if ((eslStatus == DOOR_LOCK_OPEN) && (eslFlage == 0x00))
              {
                eslData.msb = 0x08;
                eslData.lsb = 0x01;
                eslData.add = 0x00;
                eslFlage = 0x01;
								SmartLED_SetBlink(LED_G, 1000, 1);
              }
              break;

            case 0xB1: break;
            case 0xF5:

              if (linFrame.data[0] == 0x05)
              {
                eslData.msb = 0x04;
                eslData.lsb = 0x01;
                eslData.add = 0x00;
								eslFlage  = 0x00;
                eslStatus = BRAKE_PEDAL;
              }
              else if (linFrame.data[0] == 0x41)
              {
                eslData.msb = 0x04;
                eslData.lsb = 0x01;
                eslData.add = 0x00;
								eslFlage  = 0x00;
                eslStatus = START_BTN_KEY;
								SmartLED_SetBlink(LED_G, 300, 3);
              }
              break;
							
							default: break;
          }
        }
      }
      else if (request == LIN_SEND_CMD)
      {

        if (linFrame.frame_id == 0xC1)
        {
          switch (eslStatus)
          {
            case DOOR_LOCK_OPEN:
              getCondition(eslDoorConditions, sizeof(eslDoorConditions) / sizeof(eslData_t));
            break;

            case BRAKE_PEDAL:    
						break;
						
            case START_BTN_KEY:
              getCondition(eslStartConditions, sizeof(eslStartConditions) / sizeof(eslData_t));
             break;
          }

          linFrame.data[0] = eslData.msb + eslData.add;
          linFrame.data[1] = eslData.lsb;

          eslData.lsb = eslData.lsb + 1;

        }
        else if (linFrame.frame_id == 0x37)
        {
          switch (eslStatus)
          {
            case DOOR_LOCK_OPEN:
              getCondition(eslDoorConditions, sizeof(eslDoorConditions) / sizeof(eslData_t));
              memcpy(&linFrame.data[2], &res37Cmd[0][0], 6);
              break;

            case START_BTN_KEY:
              eslData.msb = 0x04;
              eslData.add = 0x04;
              memcpy(&linFrame.data[2], &res37Cmd[1][0], 6);
              break;
          }

          linFrame.data[0] = eslData.msb;
          linFrame.data[1] = eslData.lsb;

          eslData.lsb = eslData.lsb + 1;
        }
				
        UB_LIN_SendData(&linFrame);
      }
}

void ESL_Update(void) {
	
}
