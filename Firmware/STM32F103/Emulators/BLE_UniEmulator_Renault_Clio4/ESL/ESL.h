#ifndef _ESL_H_
#define _ESL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/***************************************************************
 * 500Kb CAN systems :
 * 
 * LAGUNA 3
 * MAGANE 2  , MEGANE 3
 * CLIO 3    , CLIO 4
 * FLUENCE 2 , FLUENCE 3
 * SCENIC 2  , SCENIC 3
 * CAPTUR
 * LATITUDE
 ***************************************************************/
/*
 * 250Kb CAN systems : 
 * 
 * LAGUNA 2  2001 MODEL / 2008 MODEL
 ***************************************************************/
 
#define HARDWARE_VER "HW V1.0r"
#define FIRMWARE_TYPE "ESL" // 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "LAND ROVER / RANGE ROVER"
#define FIRMWARE_MODEL "LAGUNA 2-3, MAGANE 2-3, CLIO 3-4, FLUENCE 2-3, SCENIC 2-3, CAPTUR, LATITUDE" // 500Kb - all / 250Kb - LAGUNA 2  2001-2008 MODEL
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // 1.1rc -> 1.2r

void ESL_Init(void);
void ESL_Receive(CAN_HandleTypeDef *hcan);
void ESL_Update(void);

#endif
