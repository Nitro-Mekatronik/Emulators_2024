#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define HARDWARE_VER "HW 1.0"
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Volvo"
#define FIRMWARE_MODEL "S40, C70, C30, V50"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW 1.0b"

#define INSERT_KEY_LEN 30 // 22
#define EXIT_KEY_LEN   60 // 59

enum {CMD_NULL, CMD_WRITE, CMD_READ };
enum {KEY_EXIT, KEY_INSERT};

void ESL_Init(void);
void ESL_Receive(void);
void ESL_Update(void);

#endif
