/******************************************************
 **  Project   : Smart LED V1.0
 **  Created on: 2023-09
 **  Author    : Eng Abdullah Jalloul
 ******************************************************/

#include "SmartLED.h" // Include the header file

// Array to hold pointers to LED structures
SmartLED_t *Leds[SMART_LED_MAX];

// Count of initialized LEDs
uint16_t LedsCount = 0;

// Function to initialize an LED
bool SmartLED_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool revers, void (*LED_Callback)(bool))
{
    SmartLED_t *LedStruct;

    // Check if more LEDs can be added
    if (LedsCount >= SMART_LED_MAX)
    {
        return false;
    }

    // Allocate memory for LED structure
    LedStruct = (SmartLED_t *)malloc(sizeof(SmartLED_t));

    // Check for memory allocation failure
    if (LedStruct == NULL)
    {
        return false;
    }

    // Save LED settings
    LedStruct->GPIOx = GPIOx;
    LedStruct->GPIO_Pin = GPIO_Pin;
    LedStruct->revers = revers;
    LedStruct->status = false;      // Initialize status as OFF
    LedStruct->mode = LED_MODE_OFF; // Initialize mode
    LedStruct->LED_Callback = LED_Callback;

    // Save pointer to LED structure in array and update count
    Leds[LedsCount++] = LedStruct;

    // LED Turn off
    SmartLED_Off(LedStruct);

    return true;
}

// Delete an LED from management
bool SmartLED_Delete(unsigned char led)
{
    if (led >= LedsCount)
    {
        // The LED index is out of range
        return false;
    }

    // LED Turn off
    SmartLED_Off(Leds[led]);

    // Free the allocated memory for the LED
    free(Leds[led]);

    // Shift the remaining elements down
    for (unsigned char i = led; i < LedsCount - 1; i++)
    {
        Leds[i] = Leds[i + 1];
    }

    // Decrease the count of LEDs
    LedsCount--;

    return true;
}

// Function to update all LEDs (should be called periodically)
void SmartLED_Update(void)
{
    unsigned long now = HAL_GetTick();
	
    // Loop through all initialized LEDs
    for (uint8_t i = 0; i < LedsCount; i++)
    {
        // Update individual LED
        if (now >= Leds[i]->nextTime)
            SmartLED_Check(Leds[i]);
    }
}

// Function to get the mode of a specific LED
ledMode_t SmartLED_GetMode(unsigned char led)
{
    if (led < LedsCount)
    {
        return Leds[led]->mode;
    }
    return LED_MODE_NOTHINK;
}

// Function to get the state of a specific LED
bool SmartLED_GetState(unsigned char led)
{
    if (led < LedsCount)
    {
        return Leds[led]->status;
    }
    return false;
}

// Set the mode for a specific LED and adjust its parameters
void SmartLED_SetMode(unsigned char led, ledMode_t mode)
{
    if (led < LedsCount)
    {

        if (mode > LED_MODE_BLINK_FAST)
            mode = LED_MODE_BLINK_FAST;

        Leds[led]->mode = mode;

        switch ((unsigned char)mode)
        {
        case LED_MODE_BLINK_SLOW:
            Leds[led]->blinkTimer = LED_SLOW_TIME;
            break;

        case LED_MODE_BLINK_MID:
            Leds[led]->blinkTimer = LED_MID_TIME;
            break;

        case LED_MODE_BLINK_FAST:
            Leds[led]->blinkTimer = LED_FAST_TIME;
            break;

        case LED_MODE_ON:
            SmartLED_On(Leds[led]);
            break;

        case LED_MODE_OFF:
            SmartLED_Off(Leds[led]);
            break;
        }
    }
}

// Set the LED to blink for a given time and number of times
void SmartLED_SetBlink(unsigned char led, unsigned int time, unsigned char times)
{
    if (led < LedsCount)
    {
        Leds[led]->blinkTimer = time;
        Leds[led]->blinkCount = times;
        Leds[led]->mode = LED_MODE_BLINK_TIMES;
    }
}

// Check and update the state of an individual LED
void SmartLED_Check(SmartLED_t *led)
{
    unsigned long now = HAL_GetTick();

    led->nextTime = now + led->blinkTimer;

    switch ((unsigned char)led->mode)
    {
    case LED_MODE_BLINK_SLOW:
    case LED_MODE_BLINK_MID:
    case LED_MODE_BLINK_FAST:
        SmartLED_Toggle(led);
        if (led->LED_Callback)
            led->LED_Callback(led->status);
        break;

    case LED_MODE_BLINK_TIMES:
        if (led->blinkCount)
        {
            SmartLED_Toggle(led);
            if (!led->status && --led->blinkCount == 0)
            {
                SmartLED_Off(led);
            }
        }
        break;
    }
}

// Turn an individual LED on
void SmartLED_On(SmartLED_t *led)
{
    HAL_GPIO_WritePin(led->GPIOx, led->GPIO_Pin, led->revers ? GPIO_PIN_RESET : GPIO_PIN_SET);
    led->status = true;
}

// Turn an individual LED off
void SmartLED_Off(SmartLED_t *led)
{
    HAL_GPIO_WritePin(led->GPIOx, led->GPIO_Pin, led->revers ? GPIO_PIN_SET : GPIO_PIN_RESET);
    led->status = false;
}

// Toggle the state of an individual LED
void SmartLED_Toggle(SmartLED_t *led)
{
    led->status ? SmartLED_Off(led) : SmartLED_On(led);
}
