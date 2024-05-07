#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define CONCATENATE_WITH_SPACE(buffer, string) strcat(buffer, string); strcat(buffer, " ")

// Firmware details definitions
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND ""
#define FIRMWARE_MODEL ""
#define FIRMWARE_SHIELD "No"
#define HARDWARE_VER "HW:V1.0r"
#define FIRMWARE_VER "FW:V1.0b" // 1.1rc -> 1.2r

void ESL_Init(void);
void ESL_Receive(void);
void ESL_Update(void);

#endif
