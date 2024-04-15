#include "ESL.h"
#include "LIN_Slave.h"
#include "SmartLED.h"
#include "main.h"
#include <string.h>

extern CAN_HandleTypeDef hcan;

typedef struct
{
    uint16_t id;
    uint8_t data[4];
} CanTxMsg_t;

typedef struct
{
    uint8_t len;
    uint8_t time;
    CanTxMsg_t req;
    CanTxMsg_t res[2];
} EslData_t;

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint32_t TxMailbox;
uint8_t TxData[8];
uint8_t RxData[8];

int8_t reqIndex = -1;
uint8_t dataIndex;
uint32_t sendTime;

const EslData_t RenultEslData[] = {
    {1, 0, {0x505, {0x00, 0x00, 0x40, 0x00}}, {{0x581, {0x80, 0x20, 0x00, 0x00}}}},
    {1, 0, {0x505, {0x00, 0x00, 0x00, 0x00}}, {{0x581, {0x00, 0x20, 0x00, 0x00}}}},
    {2, 0, {0x505, {0x56, 0x8F, 0x02, 0x00}}, {{0x581, {0x00, 0x20, 0x00, 0x00}}, {0x581, {0x80, 0x20, 0x00, 0x00}}}},
    {2, 0, {0x505, {0x56, 0x8F, 0x42, 0x00}}, {{0x581, {0x80, 0x20, 0x00, 0x00}}, {0x581, {0x00, 0x20, 0x00, 0x00}}}},
    {2, 2, {0x505, {0x56, 0x8F, 0x04, 0x00}}, {{0x581, {0x00, 0x08, 0x04, 0x80}}, {0x581, {0x88, 0x10, 0x00, 0x00}}}},
    {2, 2, {0x505, {0x56, 0x8F, 0x44, 0x00}}, {{0x581, {0x88, 0x10, 0x00, 0x00}}, {0x581, {0x00, 0x20, 0x00, 0x00}}}},
};

//**************************************************

// LIN_FRAME_t linFrame;

void CanSenderMsg(const CanTxMsg_t *msg)
{
    //    TxHeader.DLC = 4;
    //    TxHeader.RTR = CAN_RTR_DATA;
    //    TxHeader.IDE = CAN_ID_STD;
    //    TxHeader.StdId = msg->id;

    //    HAL_CAN_AddTxMessage(&hcan, &TxHeader, msg->data, &TxMailbox);
    //    while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox))
    //    {
    //        ;
    //    }
}

//**************************************************

int8_t checkRequest(void)
{
    int8_t ret = -1;

    for (uint8_t n = 0; n < 6; n++)
    {
        if (RxHeader.StdId == RenultEslData[n].req.id)
        {
            if (RxData[2] == RenultEslData[n].req.data[2])
            {
                sendTime = HAL_GetTick() + RenultEslData[n].time;
                dataIndex = 0;
                SmartLED_SetBlink(LED_G, 200, 1);
                return n;
            }
        }
    }
    return ret;
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
            reqIndex = checkRequest();
        }
    }
}

//**************************************************

void ESL_Update(void)
{
    uint32_t curTime = HAL_GetTick();

    if ((reqIndex >= 0) && (HAL_GetTick() >= sendTime))
    {
        memcpy(TxData, &RenultEslData[reqIndex].res[dataIndex].data, 4);

        TxHeader.RTR = CAN_RTR_DATA;
        TxHeader.IDE = CAN_ID_STD;
        TxHeader.StdId = RenultEslData[reqIndex].res[dataIndex].id;
        TxHeader.DLC = 4;

        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
        while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox))
        {
            ;
        }

        if (++dataIndex >= RenultEslData[reqIndex].len)
        {
            dataIndex = 0;
            reqIndex = -1;
        }

        sendTime = HAL_GetTick() + RenultEslData[reqIndex].time;
    }
}

//**************************************************
