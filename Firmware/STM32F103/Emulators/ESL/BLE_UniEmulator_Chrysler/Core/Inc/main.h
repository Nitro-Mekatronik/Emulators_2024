/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
enum { LED_R, LED_G, LED_B };
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void deinitEverything(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BLE_STATUS_Pin GPIO_PIN_13
#define BLE_STATUS_GPIO_Port GPIOC
#define BLE_nRST_Pin GPIO_PIN_14
#define BLE_nRST_GPIO_Port GPIOC
#define BLE_AT_MODE_Pin GPIO_PIN_15
#define BLE_AT_MODE_GPIO_Port GPIOC
#define KLINE_MODE_Pin GPIO_PIN_1
#define KLINE_MODE_GPIO_Port GPIOB
#define DI1_Pin GPIO_PIN_12
#define DI1_GPIO_Port GPIOB
#define DI2_Pin GPIO_PIN_13
#define DI2_GPIO_Port GPIOB
#define DO1_Pin GPIO_PIN_14
#define DO1_GPIO_Port GPIOB
#define DO2_Pin GPIO_PIN_15
#define DO2_GPIO_Port GPIOB
#define LED_B_Pin GPIO_PIN_15
#define LED_B_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_3
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_5
#define LED_R_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
