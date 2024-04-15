#include "main.h"
#include "ESL.h"
#include "SmartLED.h"
#include "LIN_Slave.h"
#include <string.h>


//**************************************************

LIN_FRAME_t linFrame;

uint8_t rxBuffer[8];
uint8_t request = CMD_NULL, keyStat = KEY_EXIT;
uint8_t resC1_Index = 0, resC1_Len = EXIT_KEY_LEN;

// Insert Key
// 3C  2D  C3  56  40  B9  4C  D6  6F      2C     classic
// 3C  2D  C3  56  40  B9  4C  D6  6F      2C     classic
// 3C  2D  C3  56  40  B9  4C  D6  6F      2C     classic
// 37  04  01  C1  3E  4C  95  0D  63      A8     classic
// 37  04  02  C1  3E  4C  95  0D  63      A7     classic
// C1  08  03                              F4     classic
// C1  08  04                              F3     classic
// C1  08  05                              F2     classic
// C1  08  06                              F1     classic
// C1  08  07                              F0     classic
// C1  08  08                              EF     classic
// C1  08  09                              EE     classic
// C1  08  0A                              ED     classic
// C1  08  0B                              EC     classic
// C1  08  0C                              EB     classic
// C1  08  0D                              EA     classic
// C1  08  0E                              E9     classic
// C1  08  0F                              E8     classic
// C1  08  10                              E7     classic
// C1  08  11                              E6     classic
// C1  08  12                              E5     classic
// C1  08  13                              E4     classic
// C1  07  14                              E4     classic
// C1  07  15                              E3     classic
// C1  07  16                              E2     classic

// Exit Key                                                      
// C1  07  01                              F7     classic
// C1  07  02                              F6     classic
// C1  07  03                              F5     classic
// C1  08  04                              F3     classic
// C1  08  05                              F2     classic
// C1  08  06                              F1     classic
// C1  08  07                              F0     classic
// C1  08  08                              EF     classic
// C1  08  09                              EE     classic
// C1  08  0A                              ED     classic
// C1  08  0B                              EC     classic
// C1  08  0C                              EB     classic
// C1  08  0D                              EA     classic
// C1  08  0E                              E9     classic
// C1  08  0F                              E8     classic
// C1  08  10                              E7     classic
// C1  08  11                              E6     classic
// C1  08  12                              E5     classic
// C1  08  13                              E4     classic
// C1  08  14                              E3     classic
// C1  08  15                              E2     classic
// C1  08  16                              E1     classic
// C1  08  17                              E0     classic
// C1  08  18                              DF     classic
// C1  08  19                              DE     classic
// C1  08  1A                              DD     classic
// C1  08  1B                              DC     classic
// C1  08  1C                              DB     classic
// C1  08  1D                              DA     classic
// C1  08  1E                              D9     classic
// C1  08  1F                              D8     classic
// C1  08  20                              D7     classic
// C1  08  21                              D6     classic
// C1  08  22                              D5     classic
// C1  08  23                              D4     classic
// C1  08  24                              D3     classic
// C1  08  25                              D2     classic
// C1  08  26                              D1     classic
// C1  08  27                              D0     classic
// C1  08  28                              CF     classic
// C1  08  29                              CE     classic
// C1  08  2A                              CD     classic
// C1  08  2B                              CC     classic
// C1  08  2C                              CB     classic
// C1  08  2D                              CA     classic
// C1  08  2E                              C9     classic
// C1  08  2F                              C8     classic
// C1  08  30                              C7     classic
// C1  08  31                              C6     classic
// C1  08  32                              C5     classic
// C1  08  33                              C4     classic
// C1  08  34                              C3     classic
// C1  08  35                              C2     classic
// C1  08  36                              C1     classic
// C1  08  37                              C0     classic
// C1  08  38                              BF     classic
// C1  04  39                              C2     classic
// C1  04  3A                              C1     classic
// C1  04  3B                              C0     classic

uint8_t resInsertKey_C1[INSERT_KEY_LEN][8] = {
	{0x04, 0x01, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x04, 0x02, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x03, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x04, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x05, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x06, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x07, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x08, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x09, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0A, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0B, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0C, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0D, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0E, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x0F, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x10, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x11, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x12, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x08, 0x13, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x14, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x15, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x16, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x17, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x18, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x19, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x1A, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x1B, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x1C, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x1D, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
	{0x07, 0x1E, 0xC1, 0x3E, 0x4C, 0x95, 0x0D, 0x63},
};

uint8_t resExitKey_C1[EXIT_KEY_LEN][2] = { // Initially, the request must reach 0x11 . 
	{0x07, 0x01},
	{0x07, 0x02},
	{0x07, 0x03},
	{0x08, 0x04},
	{0x08, 0x05},
	{0x08, 0x06},
	{0x08, 0x07},
	{0x08, 0x08},
	{0x08, 0x09},
	{0x08, 0x0A},
	{0x08, 0x0B},
	{0x08, 0x0C},
	{0x08, 0x0D},
	{0x08, 0x0E},
	{0x08, 0x0F},
	{0x08, 0x10},
	{0x08, 0x11},
	{0x08, 0x12},
	{0x08, 0x13},
	{0x08, 0x14},
	{0x08, 0x15},
	{0x08, 0x16},
	{0x08, 0x17},
	{0x08, 0x18},
	{0x08, 0x19},
	{0x08, 0x1A},
	{0x08, 0x1B},
	{0x08, 0x1C},
	{0x08, 0x1D},
	{0x08, 0x1E},
	{0x08, 0x1F},
	{0x08, 0x20},
	{0x08, 0x21},
	{0x08, 0x22},
	{0x08, 0x23},
	{0x08, 0x24},
	{0x08, 0x25},
	{0x08, 0x26},
	{0x08, 0x27},
	{0x08, 0x28},
	{0x08, 0x29},
	{0x08, 0x2A},
	{0x08, 0x2B},
	{0x08, 0x2C},
	{0x08, 0x2D},
	{0x08, 0x2E},
	{0x08, 0x2F},
	{0x08, 0x30},
	{0x08, 0x31},
	{0x08, 0x32},
	{0x08, 0x33},
	{0x08, 0x34},
	{0x08, 0x35},
	{0x08, 0x36},
	{0x08, 0x37},
	{0x08, 0x38},
	{0x04, 0x39},
	{0x04, 0x3A},
	{0x04, 0x3B},
	{0x04, 0x3C},
};

void ESL_Init(void) {
	UB_LIN_Slave_Init();
}

void ESL_Receive(void) {
	      request = CMD_NULL;

      switch (linFrame.frame_id) {
        case 0x3C:
          keyStat = KEY_INSERT;
          resC1_Index = 0;
          resC1_Len = INSERT_KEY_LEN;
          linFrame.data_len = 8;
          request = CMD_WRITE;
          
          break;

        case 0x37:
          linFrame.data_len = 8;
          request = CMD_READ;

          if (keyStat == KEY_INSERT) {
            memcpy(linFrame.data, &resInsertKey_C1[resC1_Index][0], 8);
						SmartLED_SetMode(LED_G, LED_MODE_ON);
          } else {
            memcpy(linFrame.data, &resExitKey_C1[resC1_Index][0], 2);
						SmartLED_SetMode(LED_G, LED_MODE_BLINK_SLOW);
          }

          if (++resC1_Index >= resC1_Len) {
            resC1_Index = 0;
					}
          break;

        case 0xC1:
          linFrame.data_len = 2;
          request = CMD_READ;
          if (keyStat == KEY_INSERT) {
            memcpy(linFrame.data, &resInsertKey_C1[resC1_Index][0], 2);
          } else {
            memcpy(linFrame.data, &resExitKey_C1[resC1_Index][0], 2);
          }

          if (++resC1_Index >= resC1_Len) {
            resC1_Index = 0;
          }
          break;

      }

      if (request == CMD_WRITE) {
        UB_LIN_ReceiveData(&linFrame);
      }
      else if (request == CMD_READ) {
        UB_LIN_SendData(&linFrame);
      }
}

void ESL_Update(void) {
	
}
