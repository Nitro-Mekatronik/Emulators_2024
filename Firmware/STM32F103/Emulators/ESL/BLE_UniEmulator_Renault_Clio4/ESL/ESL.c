#include "ESL.h"
#include "SmartLED.h"
#include "main.h"
#include <string.h>

extern CAN_HandleTypeDef hcan;

typedef struct
{
    uint32_t id;
    uint8_t data[8];
} CanTxMsg;

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint32_t TxMailbox;
uint8_t TxData[8];
uint8_t RxData[8];

bool insertkey = false;
uint32_t key1_timer;
int16_t addTimer;
int16_t counter = -1;

// Renault Megane 2, Megane 3, Laguna 2, Scenic 2, Scenic 3, Clio 3, Clio 4, Escape 4, Talisman, Captur, Fluence

// ************ RENAULT Fluence **********
const CanTxMsg key1a = {0x581, {0x02, 0xA0, 0x02, 0x03}}; // 0ms
const CanTxMsg key1b = {0x581, {0x82, 0x20, 0x00, 0x03}}; // 100ms
const CanTxMsg key1c = {0x581, {0x02, 0x20, 0x00, 0x03}}; // 100ms

// if rx id:505 4 2A B2 44 00
const CanTxMsg key2a = {0x581, {0x02, 0x20, 0x00, 0x83}}; // 20ms
const CanTxMsg key2b = {0x581, {0x80, 0x10, 0x00, 0x83}}; // 20ms
const CanTxMsg key2c = {0x581, {0x00, 0x08, 0x04, 0x83}};
const CanTxMsg key2d = {0x581, {0x80, 0x10, 0x00, 0x83}};
const CanTxMsg key2e = {0x581, {0x00, 0x10, 0x00, 0x83}}; //

const CanTxMsg key3a = {0x581, {0x88, 0x10, 0x00, 0x83}};
const CanTxMsg key3b = {0x581, {0x08, 0x10, 0x00, 0x83}};

//**************************************************

// LIN_FRAME_t linFrame;

void CanSenderMsg(const CanTxMsg *msg)
{
    TxHeader.DLC = 4;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.StdId = msg->id;

    HAL_CAN_AddTxMessage(&hcan, &TxHeader, msg->data, &TxMailbox);
    while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox))
    {
        ;
    }
}

//**************************************************

void ESL_Init(void)
{
    HAL_CAN_Start(&hcan);
}

//**************************************************

void ESL_Receive(CAN_HandleTypeDef *hcan)
{
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
    {

        if ((RxHeader.StdId == 0x505) && (RxHeader.DLC == 4))
        {
            counter = 0;
            key1_timer = 0;
            addTimer = 22;
            // Insert key
            // Megane 2: 505 Rx   d 4 73 F0 44 00
            // Clio IV : 505 Rx   d 4 B2 21 44 00
            // Fluence : 505 Rx   d 4 2A B2 44 00

            if (RxData[2] == 0x44)
            {
                insertkey = true;
            }
            else if (RxData[2] == 0x04)
            {
                insertkey = false;
            }
        }
    }
}

//**************************************************

void ESL_Update(void)
{
    uint32_t curTime = HAL_GetTick();

    if (curTime >= key1_timer)
    {

        if (insertkey == false)
        {
            switch (counter)
            {
            case -1:
                CanSenderMsg(&key1a);
                break;
            case 0:
                CanSenderMsg(&key1b);
                break;
            case 1:
                CanSenderMsg(&key1c);
                break;
            }

            if (++counter > 1)
                counter = 0;
        }
        else
        {
            switch (counter)
            {
            case 0:
                CanSenderMsg(&key2a);
                break;
            case 1:
                CanSenderMsg(&key2b);
                break;
            case 2:
                CanSenderMsg(&key2c);
                break;
            case 3:
                CanSenderMsg(&key2d);
                break;
            case 4:
                CanSenderMsg(&key2e);
                CanSenderMsg(&key3a);
                addTimer = 100;
                break;
            case 5:
                CanSenderMsg(&key3b);
                break;
            case 6:
                CanSenderMsg(&key3a);
                break;
            }

            if (++counter > 6)
                counter = 5;
        }

        key1_timer = curTime + addTimer;
    }
}

//**************************************************
