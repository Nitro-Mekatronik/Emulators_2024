#ifndef _ESL_H_
#define _ESL_H_

#include <stdint.h>

#define HARDWARE_VER "HW 1.0"
#define FIRMWARE_TYPE "ESL" // 0-Empty, 1-ESL, 2-AdBlue, 3-Chip Tuning Box, 4-IMMO, 5-Gateway
#define FIRMWARE_BRAND "Volvo"
#define FIRMWARE_MODEL "S60, S80, XC60, XC70"
#define FIRMWARE_VER "FW 1.0b"

#define INSERT_KEY_LEN 29
#define EXIT_KEY_LEN   26

enum { CMD_NULL, LIN_RECEIVE_CMD, LIN_SEND_CMD };

typedef struct {
	uint8_t msb;
	uint8_t lsb;
	uint8_t add;
} eslData_t;

enum {
	DOOR_LOCK_OPEN,      // Req ID: 0x11, 0xC1 ** The returned data is the same DOOR_OPEN and DOOR_LOCK **
	BRAKE_PEDAL,    // Req ID: 0xB1(0x01), 0xF5(0x02), 0xF5(0x05)
	START_BTN_KEY,  // Req ID: 0xB1(0x05), 0xF5(0x22), 0xF5(0x41)
//	START_BTN_NOKEY // Req ID: 0xB1(0x01), 0xF5(0x02), 0xF5(0x05)  ** The returned data is the same BRAKE_PEDAL **
};

void ESL_Init(void);
void ESL_Receive(void);
void ESL_Update(void);

#endif
