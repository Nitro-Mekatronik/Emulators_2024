#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define HARDWARE_VER "HW 1.0"
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Chrysler"
#define FIRMWARE_MODEL "up to 2011"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW 1.0b"

typedef struct {
	uint8_t pid;
	uint8_t id;
	uint8_t data[8];
} eslData_t;

void ESL_Init(void);
void ESL_Receive(void);
void ESL_Update(void);

#endif
