#include "ESL.h"
#include "SerialFIFO.h"
#include "SmartLED.h"
#include "main.h"
#include <string.h>

Serial_RxMSG_t SerialRxMSG;
LinFrame_t TxFrame;

lock_t LockPos;
uint8_t IndexCmd32, IndexCmd37;

enum
{
    INSERT_KEY1 = 1,
    INSERT_KEY2,
    EXIT_KEY1,
    EXIT_KEY2
};
uint8_t KeyStatus;

const uint8_t resCmd37[5][8] = {
    {0x04, 0x03, 0xA4, 0x46, 0xE1, 0xD6, 0x62, 0x00}, // Insert Key
    {0x04, 0x02, 0xA4, 0x46, 0xE1, 0xD6, 0x62, 0x00}, // Insert Key
    {0x08, 0x03, 0x58, 0xAD, 0x54, 0xF6, 0x7C, 0x00}, // Exit Key
    {0x07, 0x04, 0x58, 0xAD, 0x54, 0xF6, 0x7C, 0x00}, // Exit Key
    {0x07, 0x05, 0x58, 0xAD, 0x54, 0xF6, 0x7C, 0x00}  // Exit Key
};

const uint8_t resCmd7D[2][8] = {
    {0x30, 0x80, 0x76, 0x9A, 0x9C, 0xA6, 0x00, 0x00}, // Insert Key
    {0xCF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // Exit Key
};

const uint8_t resCmd61[] = {0x87, 0x0B, 0x29, 0x0C};

const uint8_t resCmd32[4][8] = {{0x01, 0x00, 0x30, 0x79, 0x39, 0x71, 0x44, 0x00},
                                {0x02, 0x36, 0x47, 0x39, 0x4E, 0x00, 0x00, 0x00},
                                {0x03, 0x33, 0x46, 0x38, 0x38, 0x30, 0x00, 0x00},
                                {0x04, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00}};

void ESL_Update(void)
{

    if (SerialRxMSG.Status == LIN_HEADER_ID)
    {
        if (LIN_checkID(SerialRxMSG.frame.Id) == LIN_READ)
        {
            SerialRxMSG.frame.DataSize = 0;
            SerialRxMSG.Status = LIN_IDLE;

            if (TxFrame.DataSize)
            {
                LIN_SendData(&TxFrame);
                TxFrame.DataSize = 0;
            }
        }
    }
}

void ESL_EvaluateCommand(void)
{

    switch (SerialRxMSG.frame.Id)
    {

    case 0xF5:
        //			if ((SerialRxMSG.frame.DataBuffer[0] == 0x02) || (SerialRxMSG.frame.DataBuffer[0] == 0x22) ||
        //(SerialRxMSG.frame.DataBuffer[0] == 0x41)) { 				IndexCmd32 = 0; 				IndexCmd37 = 0; 				KeyStatus = INSERT_KEY1;
        //        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
        //			}
        break;

    case 0x11:
        break;

    case 0x92:
        IndexCmd32 = 0;
        IndexCmd37 = 0;
        // 0x55:insert key, 0x06:exit key
        if (SerialRxMSG.frame.DataBuffer[0] == 0x55)
        {
            KeyStatus = INSERT_KEY1;
            SmartLED_SetMode(LED_G, LED_MODE_ON);
        }
        else
        {
            KeyStatus = EXIT_KEY1;
            SmartLED_SetMode(LED_G, LED_MODE_BLINK_SLOW);
        }
        break;
    }
}

uint8_t LIN_checkID(uint8_t id)
{
    uint8_t ret = LIN_WRITE;
    TxFrame.DataSize = 0;

    switch (id)
    {

    case 0xF5: // LIN Write Command
        SerialRxMSG.Status = LIN_RECEIVE_DATA;
        SerialRxMSG.frame.DataSize = 8;
        break;

    case 0x11: // LIN Write Command
        SerialRxMSG.Status = LIN_RECEIVE_DATA;
        SerialRxMSG.frame.DataSize = 2;
        break;

    case 0x92: // LIN Write Command
        SerialRxMSG.Status = LIN_RECEIVE_DATA;
        SerialRxMSG.frame.DataSize = 2;
        break;

    case 0xC1: // LIN Read Command

        if (KeyStatus == INSERT_KEY1)
        {
            LockPos.Hi = 0x08;
            LockPos.Lo = 0x01;
            KeyStatus = INSERT_KEY2;
        }
        else if (KeyStatus == INSERT_KEY2)
        {
            if (LockPos.Lo == 0x01)
                LockPos.Lo = 0x04;
            else if (++LockPos.Lo == 0x016)
                LockPos.Hi = 0x07;
        }
        else if (KeyStatus == EXIT_KEY1)
        {
            LockPos.Hi = 0x08;
            LockPos.Lo = 0x01;
            KeyStatus = EXIT_KEY2;
        }
        else if (KeyStatus == EXIT_KEY2)
        {
            if (LockPos.Lo == 0x01)
                LockPos.Lo = 0x06;
            else if (++LockPos.Lo == 0x012)
                LockPos.Hi = 0x04;
        }

        TxFrame.DataSize = 2;
        TxFrame.DataBuffer[0] = LockPos.Hi;
        TxFrame.DataBuffer[1] = LockPos.Lo;
        ret = LIN_READ;
        break;

    case 0x32: // LIN Read Command
        memcpy(TxFrame.DataBuffer, &resCmd32[IndexCmd32][0], 8);
        if (++IndexCmd32 >= 4)
            IndexCmd32 = 0;
        TxFrame.DataSize = 8;
        ret = LIN_READ;
        break;

    case 0x37: // LIN Read Command
        if ((KeyStatus == INSERT_KEY1) || (KeyStatus == INSERT_KEY2))
        {
            memcpy(TxFrame.DataBuffer, &resCmd37[IndexCmd37][0], 8);
            if (++IndexCmd37 >= 2)
                IndexCmd37 = 0;
        }
        else if ((KeyStatus == EXIT_KEY1) || (KeyStatus == EXIT_KEY2))
        {
            memcpy(TxFrame.DataBuffer, &resCmd37[2 + IndexCmd37][0], 8);
            if (++IndexCmd37 >= 3)
                IndexCmd37 = 0;
        }

        TxFrame.DataSize = 8;
        ret = LIN_READ;
        break;

    case 0x61: // LIN Read Command
        memcpy(TxFrame.DataBuffer, &resCmd61[0], 4);
        TxFrame.DataSize = 4;
        ret = LIN_READ;
        break;

    case 0x7D: // LIN Read Command
        if ((KeyStatus == INSERT_KEY1) || (KeyStatus == INSERT_KEY2))
        {
            memcpy(TxFrame.DataBuffer, &resCmd7D[0][0], 8);
        }
        else if ((KeyStatus == EXIT_KEY1) || (KeyStatus == EXIT_KEY2))
        {
            memcpy(TxFrame.DataBuffer, &resCmd7D[1][0], 8);
        }
        TxFrame.DataSize = 8;
        ret = LIN_READ;
        break;
    }
    return ret;
}

void ESL_UART_Receive(uint8_t len)
{

    while (len--)
    {
        unsigned char c = UART3_Read();

        switch ((unsigned char)SerialRxMSG.Status)
        {
        case LIN_IDLE:
            SerialRxMSG.Status = (c == 0x00) ? LIN_HEADER_START : LIN_IDLE;
            break;

        case LIN_HEADER_START:
            SerialRxMSG.Status = (c == 0x55) ? LIN_HEADER_SYNC : LIN_IDLE;
            break;

        case LIN_HEADER_SYNC:
            SerialRxMSG.frame.Id = c;
            SerialRxMSG.Status = LIN_HEADER_ID;
            SerialRxMSG.frame.Index = 0;
            SerialRxMSG.frame.Checksum = 0;
            // LIN_checkID(c); // check data length from id
            return;

        case LIN_RECEIVE_DATA:
            SerialRxMSG.frame.DataBuffer[SerialRxMSG.frame.Index] = c;
            if (++SerialRxMSG.frame.Index >= SerialRxMSG.frame.DataSize)
                SerialRxMSG.Status = LIN_RECEIVE_CRC;
            break;

        case LIN_RECEIVE_CRC:
            if (c == LIN_CalcChecksum(&SerialRxMSG.frame))
            {
                UART3_Flush();
                len = 0;
                ESL_EvaluateCommand();
            }

            SerialRxMSG.Status = LIN_IDLE;
            break;
        }
    }
}

void LIN_SendData(LinFrame_t *frame)
{
    frame->Checksum = LIN_CalcChecksum(frame);
    for (uint8_t n = 0; n < frame->DataSize; n++)
        UART3_Write(frame->DataBuffer[n]);
    UART3_Write(frame->Checksum);
}

uint8_t LIN_CalcChecksum(LinFrame_t *frame)
{
    uint8_t crc = 0;
    uint16_t dummy = 0;

    // calculate checksum
    for (uint8_t n = 0; n < frame->DataSize; n++)
    {

        dummy += frame->DataBuffer[n];

        if (dummy > 0xFF)
        {
            dummy -= 0xFF;
        }
    }

    crc = (uint8_t)(dummy);
    crc ^= 0xFF;

    return (crc);
}
