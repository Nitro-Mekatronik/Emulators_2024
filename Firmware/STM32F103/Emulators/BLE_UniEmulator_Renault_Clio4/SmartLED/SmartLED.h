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

// Constants defining the LED blink timings (in milliseconds)
#ifndef LED_SLOW_TIME
#define LED_SLOW_TIME 500
#endif

#ifndef LED_MID_TIME
#define LED_MID_TIME 250
#endif

#ifndef LED_FAST_TIME
#define LED_FAST_TIME 100
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
        LED_MODE_BLINK_FAST
    } ledMode_t;

    // Structure to hold LED information
    typedef struct
    {
        GPIO_TypeDef *GPIOx; // GPIO port for LED
        uint16_t GPIO_Pin;   // GPIO pin for LED
        uint8_t blinkCount;  // Counter for LED blinking
        uint32_t blinkTimer; // Timer for LED blinking
        uint32_t nextTime;   // Next time the LED was updated
        ledMode_t mode;      // Current mode of the LED
        bool status;         // Current status of the LED (ON/OFF)
        bool revers;         // Reverse LED ON/OFF logic
        void (*LED_Callback)(bool status);
    } SmartLED_t;

    // Function to initialize an LED
    bool SmartLED_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool revers, void (*LED_Callback)(bool));

    // Delete an LED from management
    bool SmartLED_Delete(unsigned char led);

    // Function to update all LEDs (should be called periodically)
    void SmartLED_Update(void);

    // Get the current mode of a specific LED
    ledMode_t SmartLED_GetMode(unsigned char led);

    // Get the current state (ON/OFF) of a specific LED
    bool SmartLED_GetState(unsigned char led);

    // Set the mode for a specific LED and adjust its parameters
    void SmartLED_SetMode(unsigned char led, ledMode_t mode);

    // Set the LED to blink for a given time and number of times
    void SmartLED_SetBlink(unsigned char led, unsigned int time, unsigned char times);

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
