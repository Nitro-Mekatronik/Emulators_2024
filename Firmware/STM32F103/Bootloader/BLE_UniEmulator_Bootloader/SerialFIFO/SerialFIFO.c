/******************************************************
 **  Project   : Serial FIFO UART1
 **  Created on: 24/09/2019
 **  Author    : Eng Abdullah Jalloul
 ******************************************************/

#include "SerialFIFO.h"
#include "main.h"

extern UART_HandleTypeDef huart1;

uint8_t DMA_RX1_Buffer[DMA_RX1_BUFFER_SIZE];
SerialFifo_t UART1_RxFIFO = {{0}, 0, 0, 0}; // receive buffer
SerialFifo_t UART1_TxFIFO = {{0}, 0, 0, 0}; // transmit buffer

//************************************************************************************************
void UART1_Init(void)
{
    HAL_UART_Receive_DMA(&huart1, DMA_RX1_Buffer, DMA_RX1_BUFFER_SIZE);
}

uint16_t UART1_Available(void)
{
    return UART1_RxFIFO.numBytes;
}

//************************************************************************************************

uint8_t UART1_Read(void)
{
    uint8_t byte = 0;

    // disable interrupts
    HAL_NVIC_DisableIRQ(USART1_IRQn);

    if (UART1_RxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                     // if the sw buffer is full
        UART1_RxFIFO.flags &= ~FIFO_FULL; // clear the buffer full flag because we are about to make room
    }

    if (UART1_RxFIFO.numBytes > 0)
    {                                                        // if data exists in the sw buffer
        byte = UART1_RxFIFO.data_buf[UART1_RxFIFO.idxFirst]; // grab the oldest element in the buffer
        UART1_RxFIFO.idxFirst++;                             // increment the index of the oldest element
        UART1_RxFIFO.numBytes--;                             // decrement the bytes counter
    }
    else
    {                                          // RX sw buffer is empty
        UART1_RxFIFO.flags &= ~FIFO_NOT_EMPTY; // clear the rx flag
    }

    if (UART1_RxFIFO.idxFirst == FIFO_BUFFER_SIZE)
    {                              // if the index has reached the end of the buffer,
        UART1_RxFIFO.idxFirst = 0; // roll over the index counter
    }

    // enable interrupts
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    return byte; // return the data byte
}

//************************************************************************************************

void UART1_Flush(void)
{
    UART1_RxFIFO.idxFirst = 0;
    UART1_RxFIFO.idxLast = 0;
    UART1_RxFIFO.numBytes = 0;
}

void UART1_WriteStr(char *str)
{
    do
    {
        UART1_Write((unsigned char)*str++);
    } while (*str);
}

bool UART1_ReadStr(char *str, int maxLength, uint32_t timeout)
{
    int idx = 0;
    char ch;
    bool stringComplete = false;
    uint32_t startTime = HAL_GetTick(); // Get the start time

    while (!stringComplete && idx < maxLength - 1)
    {
        if (UART1_Available())
        {                      // Check if data is available
            ch = UART1_Read(); // Read a single character
            if (ch == '\r' || ch == '\n')
            { // Check for newline or carriage return
                stringComplete = true;
            }
            else
            {
                str[idx++] = ch; // Add character to string
            }
        }
        else if ((HAL_GetTick() - startTime) > timeout)
        {          // Check if timeout is exceeded
            break; // Exit the loop if timeout
        }
    }

    str[idx] = '\0'; // Null-terminate the string

    return stringComplete;
}

uint8_t UART1_Write(uint8_t byte)
{
    // disable interrupts
    HAL_NVIC_DisableIRQ(USART1_IRQn);

    if (UART1_TxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                    // no room in the sw buffer
        UART1_TxFIFO.flags |= FIFO_OVER; // set the overflow flag
    }
    else if (UART1_TxFIFO.numBytes < FIFO_BUFFER_SIZE)
    {                                                       // if there's room in the sw buffer
        UART1_TxFIFO.data_buf[UART1_TxFIFO.idxLast] = byte; // transfer data byte to sw buffer
        UART1_TxFIFO.idxLast++;                             // increment the index of the most recently added element
        UART1_TxFIFO.numBytes++;                            // increment the bytes counter
    }

    if (UART1_TxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                    // if sw buffer is full
        UART1_TxFIFO.flags |= FIFO_FULL; // set the TX FIFO full flag
    }

    if (UART1_TxFIFO.idxLast == FIFO_BUFFER_SIZE)
    {                             // if the "new data" index has reached the end of the buffer,
        UART1_TxFIFO.idxLast = 0; // roll over the index counter
    }

    // enable interrupts
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    if (UART1_TxFIFO.numBytes > 0)
    { // if there is data in the buffer

        UART1_TxFIFO.flags |= FIFO_NOT_EMPTY; // set flag

        if (!READ_BIT(USART1->CR1, USART_CR1_TXEIE))
        {
            SET_BIT(USART1->CR1, USART_CR1_TXEIE); // Enable the UART Transmit data register empty Interrupt
            CLEAR_BIT(USART1->CR1, USART_CR1_RE);
        }
    }

    return 0;
}

//************************************************************************************************
void UART1_Receive_IRQ(void)
{

    if (UART1_RxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                    // if the sw buffer is full
        UART1_RxFIFO.flags |= FIFO_OVER; // set the overflow flag
    }
    else if (UART1_RxFIFO.numBytes < FIFO_BUFFER_SIZE)
    { // if there's room in the sw buffer
        UART1_RxFIFO.data_buf[UART1_RxFIFO.idxLast] = USART1->DR;
        UART1_RxFIFO.idxLast++;  // increment the index of the most recently added element
        UART1_RxFIFO.numBytes++; // increment the bytes counter
    }

    if (UART1_RxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                    // if sw buffer just filled up
        UART1_RxFIFO.flags |= FIFO_FULL; // set the RX FIFO full flag
    }

    if (UART1_RxFIFO.idxLast == FIFO_BUFFER_SIZE)
    {                             // if the index has reached the end of the buffer,
        UART1_RxFIFO.idxLast = 0; // roll over the index counter
    }

    UART1_RxFIFO.flags |= FIFO_NOT_EMPTY; // set received-data flag
}

//************************************************************************************************

void UART1_Transmit_IRQ(void)
{
    if (UART1_TxFIFO.numBytes == FIFO_BUFFER_SIZE)
    {                                     // if the sw buffer is full
        UART1_TxFIFO.flags &= ~FIFO_FULL; // clear the buffer full flag because we are about to make room
    }

    if (UART1_TxFIFO.numBytes > 0)
    { // if data exists in the sw buffer

        USART1->DR =
            UART1_TxFIFO.data_buf[UART1_TxFIFO.idxFirst]; // place oldest data element in the TX hardware buffer

        UART1_TxFIFO.idxFirst++; // increment the index of the oldest element
        UART1_TxFIFO.numBytes--; // decrement the bytes counter
    }

    if (UART1_TxFIFO.idxFirst == FIFO_BUFFER_SIZE)
    {                              // if the index has reached the end of the buffer,
        UART1_TxFIFO.idxFirst = 0; // roll over the index counter
    }

    if (UART1_TxFIFO.numBytes == 0)
    { // if no more data exists

        UART1_TxFIFO.flags &= ~(FIFO_NOT_EMPTY | FIFO_FULL | FIFO_OVER); // clear flag

        // Disable the UART Transmit Complete Interrupt
        CLEAR_BIT(USART1->CR1, USART_CR1_TXEIE);

        // Enable the UART Transmit Complete Interrupt
        SET_BIT(USART1->CR1, USART_CR1_TCIE);
    }
}

//************************************************************************************************
