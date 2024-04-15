#ifndef _License_H_
#define _License_H_

#include <stdbool.h>
#include <stdint.h>

#define MANUFACTURER_CODE "TR"
// #define MANUFACTURER_CODE  "CN"

#define MAX_PROTOCOLS 2
#define MAX_NUMBER_IDS 128
#define MAX_DTC_CODE 256

#define DIAGNOSTIC_WAITTING (10 * 60 * 1000) // wait 10 minutes

// Constants for EEPROM address
#define EEPROM_START_ADDR 1
#define ENCRYPTED_ID_LENGTH 16
#define ENCRYPTED_ALGORITHM_LENGTH 1

#define EEPROM_CONFIG_DATA_ADDR (EEPROM_START_ADDR + ENCRYPTED_ID_LENGTH + ENCRYPTED_ALGORITHM_LENGTH)
#define CONFIG_DATA_LENGTH (8 * MAX_PROTOCOLS)

#define EEPROM_NUMBER_PROTOCOL_ADDR (EEPROM_CONFIG_DATA_ADDR + CONFIG_DATA_LENGTH)
#define NUMBER_PROTOCOL_LENGTH 1

#define EEPROM_DATABASE_1_ADDR (EEPROM_NUMBER_PROTOCOL_ADDR + NUMBER_PROTOCOL_LENGTH)
#define DATABASE_LENGTH (8 * MAX_NUMBER_IDS) // 1024 bytes

#define EEPROM_DATABASE_2_ADDR (EEPROM_DATABASE_1_ADDR + DATABASE_LENGTH)

#define EEPROM_DTC_DATA_ADDR (EEPROM_DATABASE_2_ADDR + DATABASE_LENGTH)
#define DTC_DATA_LENGTH (8 * MAX_DTC_CODE) // 2048 bytes

#define EEPROM_TOTAL_DATA (EEPROM_DTC_DATA_ADDR + DTC_DATA_LENGTH) // total 4136 bytes

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
