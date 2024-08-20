#include "Bootloader.h"
#include "EEPROM.h"
#include "SmartLED.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

extern CAN_HandleTypeDef hcan;

extern CRC_HandleTypeDef hcrc;

extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

uint32_t Flashed_offset;
Flash_Status flashStatus;
BootloaderMode bootloaderMode = JumpMode;

BootloaderMode bootloaderInit(void)
{

    Flashed_offset = 0;
    flashStatus = FLASH_UNERASED;

    EEPROM_Read(0x00, (uint8_t *)&bootloaderMode, 1);

    if (bootloaderMode != JumpMode)
    {
        bootloaderMode = FlashMode;
        //		SmartLED_SetMode(LED_B, LED_MODE_BLINK_SLOW);
    }
    else // Jump Mode
    {
        // Check if the application is there
        uint8_t emptyCellCount = 0;
        for (uint8_t i = 0; i < 10; i++)
        {
            if (readWord(APPLICATION_START_ADDRESS + (i * 4)) == 0xFFFFFFFF)
                emptyCellCount++;
        }

        if (emptyCellCount != 10)
        {
            jumpToApp();
        }
        else
        {
            SmartLED_SetMode(LED_R, LED_MODE_BLINK_MID); // Error
        }
    }
    return bootloaderMode;
}

Flashing_Status flashWord(uint32_t dataToFlash)
{

    Flashing_Status FlashingStatus = FLASHING_OK;

    if (flashStatus == FLASH_UNLOCKED)
    {
        volatile HAL_StatusTypeDef status;
        uint8_t flashAttemptCounter = 0;
        uint32_t address;

        do
        {
            address = APPLICATION_START_ADDRESS + Flashed_offset;
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, dataToFlash);
            flashAttemptCounter++;
        } while ((status != HAL_OK) && (flashAttemptCounter < MAX_FLASH_ATTEMPTS) &&
                 (dataToFlash != readWord(address)));

        if (status != HAL_OK)
        {
            FlashingStatus = FLASHING_ERROR;
            SmartLED_SetBlink(LED_R, 100, 1);
        }
        else
        { // Word Flash Successful
            Flashed_offset += sizeof(uint32_t);
        }
    }
    else
    { // Error
        FlashingStatus = FLASHING_ERROR_MEMORY_LOCKED;
        SmartLED_SetBlink(LED_R, 300, 1);
    }

    return FlashingStatus;
}

uint32_t readWord(uint32_t address)
{
    uint32_t read_data;
    read_data = *(uint32_t *)(address);
    return read_data;
}

void eraseMemory(void)
{
    /* Unock the Flash to enable the flash control register access *************/
    while (HAL_FLASH_Unlock() != HAL_OK)
        while (HAL_FLASH_Lock() != HAL_OK)
            ; // Weird fix attempt

    /* Allow Access to option bytes sector */
    while (HAL_FLASH_OB_Unlock() != HAL_OK)
        while (HAL_FLASH_OB_Lock() != HAL_OK)
            ; // Weird fix attempt

    /* Fill EraseInit structure*/
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = APPLICATION_START_ADDRESS;
    EraseInitStruct.NbPages = FLASH_BANK_SIZE / FLASH_PAGE_SIZE_USER;
    uint32_t PageError;

    volatile HAL_StatusTypeDef status_erase;
    status_erase = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    /* Lock the Flash to enable the flash control register access *************/
    while (HAL_FLASH_Lock() != HAL_OK)
        while (HAL_FLASH_Unlock() != HAL_OK)
            ; // Weird fix attempt

    /* Lock Access to option bytes sector */
    while (HAL_FLASH_OB_Lock() != HAL_OK)
        while (HAL_FLASH_OB_Unlock() != HAL_OK)
            ; // Weird fix attempt

    if (status_erase != HAL_OK)
        SmartLED_SetMode(LED_R, LED_MODE_BLINK_FAST);
    flashStatus = FLASH_ERASED;
    Flashed_offset = 0;
}

void unlockFlashAndEraseMemory(void)
{
    /* Unock the Flash to enable the flash control register access *************/
    while (HAL_FLASH_Unlock() != HAL_OK)
        while (HAL_FLASH_Lock() != HAL_OK)
            ; // Weird fix attempt

    /* Allow Access to option bytes sector */
    while (HAL_FLASH_OB_Unlock() != HAL_OK)
        while (HAL_FLASH_OB_Lock() != HAL_OK)
            ; // Weird fix attempt

    if (flashStatus != FLASH_ERASED)
    {
        /* Fill EraseInit structure*/
        FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.PageAddress = APPLICATION_START_ADDRESS;
        EraseInitStruct.NbPages = FLASH_BANK_SIZE / FLASH_PAGE_SIZE_USER;
        uint32_t PageError;

        volatile HAL_StatusTypeDef status_erase;
        status_erase = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

        if (status_erase != HAL_OK)
            SmartLED_SetMode(LED_R, LED_MODE_BLINK_FAST);
        Flashed_offset = 0;
    }

    flashStatus = FLASH_UNLOCKED;
}

void lockFlash(void)
{

    /* Lock the Flash to enable the flash control register access *************/
    while (HAL_FLASH_Lock() != HAL_OK)
        while (HAL_FLASH_Unlock() != HAL_OK)
            ; // Weird fix attempt

    /* Lock Access to option bytes sector */
    while (HAL_FLASH_OB_Lock() != HAL_OK)
        while (HAL_FLASH_OB_Unlock() != HAL_OK)
            ; // Weird fix attempt

    flashStatus = FLASH_LOCKED;
}

void jumpToApp(void)
{
    application_t app_entry;
    uint32_t app_stack;

    if (bootloaderMode == FlashMode)
    {
        bootloaderMode = JumpMode;
        EEPROM_Write(0x00, (uint8_t *)&bootloaderMode, 1);
    }

    deinitEverything();

    // Set HSION bit.
    RCC->CR |= 0x00000001U;

    // Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits.
    RCC->CFGR &= 0xF8FF0000U;

    // Reset HSEON, CSSON and PLLON bits.
    RCC->CR &= 0xFEF6FFFFU;

    // Reset HSEBYP bit.
    RCC->CR &= 0xFFFBFFFFU;

    // Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits.
    RCC->CFGR &= 0xFF80FFFFU;

    // Disable all interrupts and clear pending bits.
    RCC->CIR = 0x009F0000U;

    // Get the application stack pointer (first entry in the application vector table).
    app_stack = (uint32_t) * ((__IO uint32_t *)APPLICATION_START_ADDRESS);

    // Get the application entry point (second entry in the application vector table).
    app_entry = (application_t) * (__IO uint32_t *)(APPLICATION_START_ADDRESS + 4u);

    // Reconfigure vector table offset register to match the application location.
    SCB->VTOR = APPLICATION_START_ADDRESS;

    // Set the application stack pointer.
    __set_MSP(app_stack);

    // Start the application.
    app_entry();
}

void deinitEverything()
{
    //-- reset peripherals to guarantee flawless start of user application
    HAL_GPIO_DeInit(LED_R_GPIO_Port, LED_R_Pin);
    HAL_GPIO_DeInit(LED_G_GPIO_Port, LED_G_Pin);
    HAL_GPIO_DeInit(LED_B_GPIO_Port, LED_B_Pin);
    HAL_ADC_MspDeInit(&hadc1);
    HAL_CAN_MspDeInit(&hcan);
    HAL_CRC_MspDeInit(&hcrc);
    HAL_I2C_MspDeInit(&hi2c1);
    HAL_SPI_MspDeInit(&hspi1);
    HAL_TIM_MspPostInit(&htim3);
    HAL_TIM_MspPostInit(&htim4);
    HAL_UART_MspDeInit(&huart1);
    HAL_UART_MspDeInit(&huart3);

    HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);

    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOA_CLK_DISABLE();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}
