#ifndef __LIN_SLAVE_H
#define __LIN_SLAVE_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f1xx_hal.h"
#include "main.h"

//--------------------------------------------------------------
// GPIO: UART-TX (PA2)
//--------------------------------------------------------------
#define  LIN_TX_CLOCK    RCC_AHB1Periph_GPIOA
#define  LIN_TX_PIN      GPIO_Pin_2
#define  LIN_TX_SOURCE   GPIO_PinSource2
#define  LIN_TX_PORT     GPIOA


//--------------------------------------------------------------
// GPIO: UART-RX (PA3)
//--------------------------------------------------------------
#define  LIN_RX_CLOCK    RCC_AHB1Periph_GPIOA
#define  LIN_RX_PIN      GPIO_Pin_3
#define  LIN_RX_SOURCE   GPIO_PinSource3
#define  LIN_RX_PORT     GPIOA


//--------------------------------------------------------------
// UART: (UART-1)
//--------------------------------------------------------------
#define  LIN_UART        USART1
#define  LIN_UART_CLOCK  RCC_APB1Periph_USART1
#define  LIN_UART_AF     GPIO_AF_USART1
//#define  LIN_UART_IRQ    USART1_IRQn
//#define  LIN_UART_ISR    USART1_IRQHandler

void LIN_UART_ISR(void);
//--------------------------------------------------------------
// GPIO: NSLP-Pin from the transceiver (PA1)
//--------------------------------------------------------------
#define  LIN_NSLP_CLOCK    RCC_AHB1Periph_GPIOA
#define  LIN_NSLP_PIN      GPIO_Pin_1
#define  LIN_NSLP_PORT     GPIOA



//--------------------------------------------------------------
// Baud rate (max 20 kBaud)
//
// @9600 Baud: Field time (10bit) = approx. 1ms
//--------------------------------------------------------------
#define  LIN_UART_BAUD   9600



//--------------------------------------------------------------
// Global Defines
//--------------------------------------------------------------
#define  LIN_SYNC_DATA               0x55  // SyncField (do not change)
#define  LIN_MAX_DATA                   8  // max 8 data bytes



//--------------------------------------------------------------
// Defines for the transceiver (do not change!!)
//--------------------------------------------------------------
#define  LIN_POWERON_DELAY          10  // Delay at PowerOn     (approx. 10ms)
#define  LIN_ACTIVATE_DELAY          5  // Delay for transceiver (approx. 50us)



//--------------------------------------------------------------
// Break times (be careful when changing!!)
//--------------------------------------------------------------
#define  LIN_INTER_FRAME_DELAY       50  // Delay (Frame->Frame)   (approx. 10ms)
#define  LIN_FRAME_RESPONSE_DELAY     5  // Delay (Header->Data)   (approx.  2ms)
#define  LIN_BREAKFIELD_DELAY        50  // Delay (Breakfield)     (approx.  4ms)
#define  LIN_DATA_BYTE_DELAY          2  // Delay (Data->Data)     (approx.  1ms)
#define  LIN_RX_TIMEOUT_CNT           5  // Timeout when receiving (approx.  5ms)



//--------------------------------------------------------------
// Structure of a LIN frame
//--------------------------------------------------------------
typedef struct {
  uint8_t frame_id;              // Unique ID number of the frame
  uint8_t data_len;              // Number of data
  uint8_t data[LIN_MAX_DATA];    // Data fields
} LIN_FRAME_t;



//--------------------------------------------------------------
// Mode of the slave
//--------------------------------------------------------------
typedef enum {
  WAIT_4_BREAK = 0, // Wait for BreakField
  RECEIVE_SYNC,     // SYNC receive
  RECEIVE_ID,       // ID receive
  ID_OK,            // ID ok
  WAIT_4_CMD,       // Wait for command
  RECEIVE_DATA,     // Receive data
  RECEIVE_CRC,      // CRC received
  SEND_DATA         // Send data
} LIN_MODE_t;



//--------------------------------------------------------------
// Structure of the slave
//--------------------------------------------------------------
typedef struct {
  LIN_MODE_t mode;   // Current mode
  uint8_t data_ptr;  // Data pointer
  uint8_t crc;       // Checksum
} LIN_SLAVE_t;



//--------------------------------------------------------------
// Error messages
//--------------------------------------------------------------
typedef enum {
  LIN_OK  = 0,   // No error
  LIN_WRONG_LEN, // Wrong number of data
  LIN_RX_EMPTY,  // No frame received
  LIN_WRONG_CRC, // Checksum wrong
  LIN_NO_ID      // No ID was received
} LIN_ERR_t;




//--------------------------------------------------------------
// Global Functions
//--------------------------------------------------------------
void UB_LIN_Slave_Init(void);
LIN_ERR_t UB_LIN_checkID(LIN_FRAME_t *frame);
LIN_ERR_t UB_LIN_SendData(LIN_FRAME_t *frame);
LIN_ERR_t UB_LIN_ReceiveData(LIN_FRAME_t *frame);




//--------------------------------------------------------------
#endif // __LIN_SLAVE_H
