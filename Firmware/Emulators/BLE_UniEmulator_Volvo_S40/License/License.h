#ifndef _License_H_
#define _License_H_

#include <stdbool.h>
#include <stdint.h>

#define MANUFACTURER_CODE "TR"
// #define MANUFACTURER_CODE  "CN"


// Constants for EEPROM address
#define EEPROM_START_ADDR 1
#define ENCRYPTED_ID_LENGTH 16   			// MANUFACTURER_CODE[2] + UNIQUE[12] + EMPTY[2]
#define ENCRYPTED_ALGORITHM_LENGTH 1 	// PASSWORD_SETTING[1]

#define EEPROM_CONFIG_DATA_ADDR (EEPROM_START_ADDR + ENCRYPTED_ID_LENGTH + ENCRYPTED_ALGORITHM_LENGTH)


enum
{
    FAKE = -1,
    VALID = 1
};

//void Enter_Check_License(void);
uint8_t Check_License(void);
//void Exit_Check_License(void);

void License_SetPrimary(uint8_t level, uint8_t shift);
void License_UniqueEncryption(uint8_t *data, uint8_t level, uint8_t shift, bool isPublic);
bool License_Check_Unique_HASH(uint8_t *data);

#endif
