#ifndef _SERIAL_FIFO_H_
#define _SERIAL_FIFO_H_

#include <stdbool.h>
#include <stdint.h>

#define DMA_RX1_BUFFER_SIZE 128
#define DMA_RX3_BUFFER_SIZE  32
#define FIFO_BUFFER_SIZE    256

typedef enum
{
    FIFO_NOT_EMPTY = 0x01,
    FIFO_FULL = 0x02,
    FIFO_OVER = 0x04
} FIFO_Flags_t;

typedef struct
{
    uint8_t data_buf[FIFO_BUFFER_SIZE]; // FIFO buffer
    uint16_t idxFirst;                  // index of oldest data byte in buffer
    uint16_t idxLast;                   // index of newest data byte in buffer
    uint16_t numBytes;                  // number of bytes currently in buffer
    uint8_t flags;
} SerialFifo_t;

uint16_t UART1_Available(void);
uint8_t UART1_Read(void);

bool UART1_ReadStr(char *str, int maxLength, uint32_t timeout);
void UART1_WriteStr(char *str);

uint8_t UART1_Write(uint8_t byte);
void UART1_Flush(void);
void UART1_Init(void);
void UART1_Receive_IRQ(void);
void UART1_Transmit_IRQ(void);

#ifdef ESL_UART3
uint16_t UART3_Available(void);
uint8_t UART3_Read(void);

bool UART3_ReadStr(char *str, int maxLength, uint32_t timeout);
void UART3_WriteStr(char *str);

uint8_t UART3_Write(uint8_t byte);
void UART3_Flush(void);
void UART3_Init(void);
void UART3_Receive_IRQ(void);
void UART3_Transmit_IRQ(void);
#endif
#endif
