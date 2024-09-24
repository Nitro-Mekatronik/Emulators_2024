/******************************************************
 **  Project   : Smart LED V1.0
 **  Created on: 2023-09
 **  Author    : Eng Abdullah Jalloul
 ******************************************************/
#ifndef _SmartLED_H
#define _SmartLED_H

/* C++ detection */
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef LED_CYCLE_TIME
#define LED_CYCLE_TIME 1000 // Constant cycle time in milliseconds
#endif

#ifndef LED_LONG_TIME
#define LED_LONG_TIME 800 // Long time in milliseconds
#endif

#ifndef LED_SHORT_TIME
#define LED_SHORT_TIME 200 // Short time in milliseconds
#endif

// Constants defining the LED blink timings (in milliseconds)
#ifndef LED_SLOW_TIME
#define LED_SLOW_TIME 800
#endif

#ifndef LED_MID_TIME
#define LED_MID_TIME 400
#endif

#ifndef LED_FAST_TIME
#define LED_FAST_TIME 200
#endif

// Maximal number of supported LEDs
#ifndef SMART_LED_MAX
#define SMART_LED_MAX 6
#endif

    // Enumeration for different LED modes
    typedef enum
    {
        LED_MODE_NOTHINK,
        LED_MODE_ON,
        LED_MODE_OFF,
        LED_MODE_BLINK_TIMES,
        LED_MODE_BLINK_SLOW,
        LED_MODE_BLINK_MID,
        LED_MODE_BLINK_FAST,
        LED_MODE_CUSTOM
    } ledMode_t;

    // Structure to hold LED information
    typedef struct
    {
        GPIO_TypeDef *GPIOx;                        // GPIO port for LED
        uint16_t GPIO_Pin;                          // GPIO pin for LED
        uint8_t blinkCount;                         // Counter for LED blinking
        uint32_t blinkTimer;                        // Timer for LED blinking
        uint32_t nextTime;                          // Next time the LED should be updated
        ledMode_t mode;                             // Current mode of the LED
        bool status;                                // Current status of the LED (ON/OFF)
        bool revers;                                // Reverse LED ON/OFF logic
        const char *pattern;                        // Pointer to the pattern string
        uint8_t patternIndex;                       // Index of the current character in the pattern
        uint8_t patternLength;                      // Length of the pattern to avoid repeated strlen calls
        uint8_t patternRepeat;                      // Number of times the pattern should repeat
        uint8_t currentRepeat;                      // Current repetition count
        ledMode_t delayedMode;                      // Mode to switch to after delay
        uint32_t delayTime;                         // Time after which to switch mode
        void (*StatusChange_Callback)(bool status); // Callback when the LED status changes
        void (*Complete_Callback)(void);            // Callback when the pattern or blink sequence completes
    } SmartLED_t;

    // Function to initialize an LED
    bool SmartLED_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool revers, void (*StatusChange_Callback)(bool),
                       void (*Complete_Callback)(void));

    // Delete an LED from management
    bool SmartLED_Delete(unsigned char led);

    // Function to update all LEDs (should be called periodically)
    void SmartLED_Update(void);

    // Get the current mode of a specific LED
    ledMode_t SmartLED_GetMode(unsigned char led);

    // Get the current state (ON/OFF) of a specific LED
    bool SmartLED_GetState(unsigned char led);

    void SmartLED_SetPattern(unsigned char led, const char *pattern, unsigned char repeat);

    // Set the mode for a specific LED and adjust its parameters
    void SmartLED_SetMode(unsigned char led, ledMode_t mode);

    void SmartLED_SetModeDelayed(unsigned char led, ledMode_t mode, uint32_t delay);

    // Set the LED to blink for a given time and number of times
    void SmartLED_SetBlink(unsigned char led, unsigned int time, unsigned char times);
		
		void SmartLED_SetBlinkDelayed(unsigned char led, unsigned int time, unsigned char times, uint32_t delay);
		
    // Private Functions (Not intended for external use)

    // Check and update the state of an individual LED
    void SmartLED_Check(SmartLED_t *led);

    // Turn an individual LED on
    void SmartLED_On(SmartLED_t *led);

    // Turn an individual LED off
    void SmartLED_Off(SmartLED_t *led);

    // Toggle the state of an individual LED
    void SmartLED_Toggle(SmartLED_t *led);

#ifdef __cplusplus
}
#endif

#endif // _SMARTLED_H_
