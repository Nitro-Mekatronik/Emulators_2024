/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SerialFIFO.h"
#include "string.h"
#include "SmartLED.h"

#ifndef ESL_UART3
#include "LIN_Slave.h"
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
extern SerialFifo_t UART1_RxFIFO;
extern uint8_t DMA_RX1_Buffer[DMA_RX1_BUFFER_SIZE];
#ifdef ESL_UART3
extern SerialFifo_t UART3_RxFIFO;
extern uint8_t DMA_RX3_Buffer[DMA_RX3_BUFFER_SIZE];
#endif
/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
	SmartLED_Update();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */
	if((DMA1->ISR & DMA_ISR_GIF5) == DMA_ISR_GIF5) {
       //DMA1->IFCR |= DMA_IFCR_CTCIF5;           // Clear transfer complete flag 
        /* Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register */
			DMA1->IFCR |= DMA_IFCR_CGIF5|DMA_IFCR_CHTIF5|DMA_IFCR_CTCIF5;

	}
  /* USER CODE END DMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles USB low priority or CAN RX0 interrupts.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan);
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	if ((USART1->SR & USART_SR_IDLE) != RESET) {         /* We want IDLE flag only */
    size_t len, tocopy;
    uint8_t* ptr;
    volatile uint32_t tmp;                  /* Must be volatile to prevent optimizations */
    tmp = USART1->SR;                       /* Read status register */
    tmp = USART1->DR;                       /* Read data register */
    (void)tmp;                              /* Prevent compiler warnings */

    DMA1_Channel5->CCR &= ~DMA_CCR_EN;      /* Disabling DMA will force transfer complete interrupt if enabled */
    //NVIC_SetPendingIRQ(DMA1_Channel5_IRQn);

    len = DMA_RX1_BUFFER_SIZE - DMA1_Channel5->CNDTR;
    tocopy = FIFO_BUFFER_SIZE - UART1_RxFIFO.idxLast;      // Get number of bytes we can copy to the end of buffer

    // Check how many bytes to copy/
    if (tocopy > len) {
      tocopy = len;
    }

    // Write received data for UART main buffer for manipulation later
    ptr = DMA_RX1_Buffer;
    memcpy(&UART1_RxFIFO.data_buf[UART1_RxFIFO.idxLast], ptr, tocopy);   // Copy first part

    // Correct values for remaining data
    UART1_RxFIFO.idxLast += tocopy;
    UART1_RxFIFO.numBytes += tocopy;
    len -= tocopy;
    ptr += tocopy;

    // If still data to write for beginning of buffer
    if (len) {
      memcpy(&UART1_RxFIFO.data_buf[0], ptr, len);      // Don't care if we override Read pointer now
      UART1_RxFIFO.idxLast = len;
      UART1_RxFIFO.numBytes += len;
    }

    DMA1_Channel5->CMAR = (uint32_t)DMA_RX1_Buffer;   // Set memory address for DMA again
    DMA1_Channel5->CNDTR = DMA_RX1_BUFFER_SIZE;    // Set number of bytes to receive
    DMA1_Channel5->CCR |= DMA_CCR_EN;            // Start DMA transfer
  }

  // UART in mode Transmitter
  if (((USART1->SR & USART_SR_TXE) != RESET) && ((USART1->CR1 & USART_CR1_TXEIE) != RESET)) {
    UART1_Transmit_IRQ();
  }

  // UART in mode Transmitter end
  if (((USART1->SR & USART_SR_TC) != RESET) && ((USART1->CR1 & USART_CR1_TCIE) != RESET)) {
    CLEAR_BIT(USART1->CR1, USART_CR1_TCIE);
		SET_BIT(USART1->CR1, USART_CR1_RE);
  }
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
	#ifdef ESL_UART3
	if ((USART3->SR & USART_SR_IDLE) != RESET) {         /* We want IDLE flag only */
    size_t len, tocopy;
    uint8_t* ptr;
    volatile uint32_t tmp;                  /* Must be volatile to prevent optimizations */
    tmp = USART3->SR;                       /* Read status register */
    tmp = USART3->DR;                       /* Read data register */
    (void)tmp;                              /* Prevent compiler warnings */

    DMA1_Channel3->CCR &= ~DMA_CCR_EN;      /* Disabling DMA will force transfer complete interrupt if enabled */
    //NVIC_SetPendingIRQ(DMA1_Channel5_IRQn);

    len = DMA_RX3_BUFFER_SIZE - DMA1_Channel3->CNDTR;
    tocopy = FIFO_BUFFER_SIZE - UART3_RxFIFO.idxLast;      // Get number of bytes we can copy to the end of buffer

    // Check how many bytes to copy/
    if (tocopy > len) {
      tocopy = len;
    }

    // Write received data for UART main buffer for manipulation later
    ptr = DMA_RX3_Buffer;
    memcpy(&UART3_RxFIFO.data_buf[UART3_RxFIFO.idxLast], ptr, tocopy);   // Copy first part

    // Correct values for remaining data
    UART3_RxFIFO.idxLast += tocopy;
    UART3_RxFIFO.numBytes += tocopy;
    len -= tocopy;
    ptr += tocopy;

    // If still data to write for beginning of buffer
    if (len) {
      memcpy(&UART3_RxFIFO.data_buf[0], ptr, len);      // Don't care if we override Read pointer now
      UART3_RxFIFO.idxLast = len;
      UART3_RxFIFO.numBytes += len;
    }

    DMA1_Channel3->CMAR = (uint32_t)DMA_RX3_Buffer;   // Set memory address for DMA again
    DMA1_Channel3->CNDTR = DMA_RX3_BUFFER_SIZE;    // Set number of bytes to receive
    DMA1_Channel3->CCR |= DMA_CCR_EN;            // Start DMA transfer
  }

  // UART in mode Transmitter
  if (((USART3->SR & USART_SR_TXE) != RESET) && ((USART3->CR1 & USART_CR1_TXEIE) != RESET)) {
    UART3_Transmit_IRQ();
  }

  // UART in mode Transmitter end
  if (((USART3->SR & USART_SR_TC) != RESET) && ((USART3->CR1 & USART_CR1_TCIE) != RESET)) {
    CLEAR_BIT(USART3->CR1, USART_CR1_TCIE);
		SET_BIT(USART3->CR1, USART_CR1_RE);
  }
	#else
	LIN_UART_ISR();
	#endif
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
