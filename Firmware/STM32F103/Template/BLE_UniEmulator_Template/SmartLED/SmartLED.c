/******************************************************
 **  Project   : Smart LED V1.0
 **  Created on: 2023-09
 **  Author    : Eng Abdullah Jalloul
 ******************************************************/

#include "SmartLED.h" // Include the header file
#include <stdio.h>
#include <string.h>

// Array to hold pointers to LED structures
SmartLED_t *Leds[SMART_LED_MAX];

// Count of initialized LEDs
uint16_t LedsCount = 0;

// Function to initialize an LED
bool SmartLED_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool revers, void (*StatusChange_Callback)(bool),
                   void (*Complete_Callback)(void))
{

    // Check if more LEDs can be added
    if (LedsCount >= SMART_LED_MAX)
    {
			#ifdef DEBUG
        fprintf(stderr, "Error: Maximum number of LEDs reached.\n");
			#endif
        return false;
    }

    // Allocate memory for LED structure
    SmartLED_t *LedStruct = (SmartLED_t *)malloc(sizeof(SmartLED_t));

    // Check for memory allocation failure
    if (!LedStruct)
    {
			#ifdef DEBUG
        fprintf(stderr, "Error: Memory allocation failed for LED structure.\n");
			#endif
        return false;
    }

    // Initialize all fields in the structure
    *LedStruct = (SmartLED_t){.GPIOx = GPIOx,
                              .GPIO_Pin = GPIO_Pin,
                              .revers = revers,
                              .status = false,      // Initialize status as OFF
                              .mode = LED_MODE_OFF, // Initialize mode
                              .pattern = NULL,
                              .patternIndex = 0,
                              .patternLength = 0,
                              .patternRepeat = 0,
                              .currentRepeat = 0,
                              .delayedMode = LED_MODE_OFF,
                              .delayTime = 0,
                              .StatusChange_Callback = StatusChange_Callback,
                              .Complete_Callback = Complete_Callback};

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
    Leds[led] = NULL; // Nullify the pointer to prevent dangling pointer

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

        // Check if it's time to change the mode after a delay
        if (Leds[i]->delayTime && now >= Leds[i]->delayTime)
        {
            Leds[i]->delayTime = 0; // Reset the delay time to prevent repeating the change
            SmartLED_SetMode(i, Leds[i]->delayedMode);
        }
    }
}

void SmartLED_SetPattern(unsigned char led, const char *pattern, unsigned char repeat)
{
    if (led >= LedsCount || !pattern)
        return; // Check for valid index and pattern

    SmartLED_t *ledStruct = Leds[led];
    ledStruct->pattern = pattern; // Set the pattern
    ledStruct->patternIndex = 0;  // Start from the first character
    ledStruct->patternLength = strlen(pattern);
    ledStruct->patternRepeat = repeat;
    ledStruct->mode = LED_MODE_CUSTOM;   // Custom mode to handle pattern
    ledStruct->nextTime = HAL_GetTick(); // Start immediately
    SmartLED_Check(ledStruct);           // Initial check to set the first state
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

void SmartLED_SetModeDelayed(unsigned char led, ledMode_t mode, uint32_t delay)
{
    if (led >= LedsCount)
        return; // Ensure the LED index is within the valid range

    Leds[led]->delayedMode = mode;                // Store the mode to be set after the delay
    Leds[led]->delayTime = HAL_GetTick() + delay; // Calculate the time when the mode should change
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

    if (now >= led->nextTime)
    {
        switch (led->mode)
        {
        case LED_MODE_CUSTOM:
            if (led->pattern && led->pattern[led->patternIndex] != '\0')
            {

                // Determine the duration and state from the pattern
                switch (led->pattern[led->patternIndex])
                {
                case '-':
                    SmartLED_Toggle(led);
                    // Long duration
                    led->nextTime = now + LED_LONG_TIME;
                    break;

                case '.':
                    SmartLED_Toggle(led);
                    // Short duration
                    led->nextTime = now + (led->status ? LED_SHORT_TIME : LED_SHORT_TIME * 2);
                    break;

                default:
                    led->nextTime = now + LED_CYCLE_TIME * 2; // Full cycle off

                    if (led->status)
                    {
                        SmartLED_Toggle(led);
                    }
                    break;
                }

                if (!led->status)
                {
                    // Increment pattern index or reset if at the end
                    if (++led->patternIndex >= led->patternLength)
                    {
                        led->patternIndex = 0; // Reset pattern index

                        // Check if the current repeat is the last one
                        if (++led->currentRepeat >= led->patternRepeat)
                        {
                            led->currentRepeat = 0;   // Reset repeat count
                            led->mode = LED_MODE_OFF; // Change mode to LED_MODE_OFF
                            if (led->Complete_Callback)
                            {
                                led->Complete_Callback(); // Invoke completion callback
                            }
                        }
                    }
                }
            }
            break;

        case LED_MODE_BLINK_TIMES:
            if (led->blinkCount > 0)
            {
                SmartLED_Toggle(led);
                led->nextTime += led->blinkTimer;

                if (!led->status && --led->blinkCount == 0)
                {
                    SmartLED_Off(led);
                    if (led->Complete_Callback)
                    {
                        led->Complete_Callback();
                    }
                }
            }
            break;

        default:
            if (led->mode >= LED_MODE_BLINK_SLOW && led->mode <= LED_MODE_BLINK_FAST)
            {
                SmartLED_Toggle(led);
                led->nextTime += led->blinkTimer;
            }
            break;
        }
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

    if (led->StatusChange_Callback)
    {
        led->StatusChange_Callback(led->status);
    }
}
