#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define CONCATENATE_WITH_SPACE(buffer, string) strcat(buffer, string); strcat(buffer, " ")

#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Volvo"
#define FIRMWARE_MODEL "V40, P31340956"
#define FIRMWARE_SHIELD "NO"
#define FIRMWARE_VER "FW V1.0b" // V1.0b -> 1.1rc -> 1.2r
#define HARDWARE_VER "HW V1.0r"

#define INSERT_KEY_LEN 17
#define EXIT_KEY_LEN 28

enum
{
    CMD_NULL,
    LIN_RECEIVE_CMD,
    LIN_REQ_DATA
};

enum
{
    KEY_EXIT,
    KEY_INSERT
};

void ESL_Init(void);
void ESL_Receive(void);
void ESL_Update(void);

#endif
