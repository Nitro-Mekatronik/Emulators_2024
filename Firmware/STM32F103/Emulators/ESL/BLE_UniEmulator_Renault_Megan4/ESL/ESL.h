#ifndef _ESL_H_
#define _ESL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

 //***************************************************************
#define CONCATENATE_WITH_SPACE(buffer, string) strcat(buffer, string); strcat(buffer, " ")

#define FIRMWARE_TYPE "ESL" // 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "RENAULT/NISSAN"
#define FIRMWARE_MODEL "MEGANE 4, KADJAR, X-TRAIL, QASHQAI, JUKE, MICRA" 
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // 1.1rc -> 1.2r
#define HARDWARE_VER "HW V1.0r"

void ESL_Init(void);
void ESL_Receive(CAN_HandleTypeDef *hcan);
void ESL_Update(void);

#endif
