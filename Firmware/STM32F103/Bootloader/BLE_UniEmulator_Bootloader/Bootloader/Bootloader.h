#ifndef Bootloader_H
#define Bootloader_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define BOOTLOADER_MAJOR 1
#define BOOTLOADER_MINOR 0

#define BOOTLOADER_START_ADDRESS  0x08000000
#define APPLICATION_START_ADDRESS 0x08007000 // Origin + Bootloader size (28kB)
#define FLASH_PAGE_SIZE_USER           0x400 // 1kB
#define FLASH_BANK_SIZE              0x19000 // 100kB

#define MAX_FLASH_ATTEMPTS    10
#define WORD_OFFSET_INCREMENT 4


typedef enum {
	FLASHING_OK,
	FLASHING_ERROR,
	FLASHING_ERROR_MEMORY_LOCKED
} Flashing_Status;

typedef enum
{
  JumpMode,
	FlashMode
} BootloaderMode;

typedef enum
{
  FLASH_UNERASED,
	FLASH_ERASED,
	FLASH_UNLOCKED,
	FLASH_LOCKED
} Flash_Status;

typedef void (*application_t)(void);

BootloaderMode bootloaderInit(void);
Flashing_Status flashWord(uint32_t word);
uint32_t readWord(uint32_t address);
void eraseMemory(void);
void unlockFlashAndEraseMemory(void);
void lockFlash(void);
void jumpToApp(void);
void deinitEverything(void);

#endif // Bootloader_H
