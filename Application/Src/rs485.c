/*
 * rs485.c
 *
 *  Created on: Jul 28, 2023
 *      Author: ittuann
 */

#include "rs485.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define PHSENSORUSART huart1
#define DOCSENSORUSART huart2

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern osMessageQId PHQueueHandle;
extern osMessageQId DOCQueueHandle;
extern osMessageQId oContentQueueHandle;

static uint8_t PHSensorRxData[9];
static uint8_t O2SensorRxData[17];

static uint8_t O2SensorTxData[8];
static uint8_t PHSensorTxData[8];

static float PHVal;
static float DOC;
static float oContent;

static union FloatBytes_u {
    float floatValue;
    unsigned char bytes[4];	// 按照0为高字节,4为低字节的顺序存储数据
} fb;

/**
* @breif   	初始化RS485收发器为接收模式
* @retval  	HAL_StatusTypeDef
*/
HAL_StatusTypeDef RS485ReceiveIT_Init(void)
{
	// 使能串口接收中断
//	__HAL_UART_ENABLE_IT(&PHSENSORUSART, UART_IT_IDLE);
	if (HAL_UART_Receive_IT(&PHSENSORUSART, PHSensorRxData, sizeof(PHSensorRxData) / sizeof(uint8_t)) != HAL_OK) {
		Error_Handler();
	}
// HAL_UART_Receive_DMA(&PHSENSORUSART, PHSensorRxData, sizeof(PHSensorRxData) / sizeof(uint8_t));

//	__HAL_UART_ENABLE_IT(&DOCSENSORUSART, UART_IT_IDLE);
	if (HAL_UART_Receive_IT(&DOCSENSORUSART, O2SensorRxData, sizeof(O2SensorRxData) / sizeof(uint8_t)) != HAL_OK) {
		Error_Handler();
	}
//	status = HAL_UART_Receive_DMA(&DOCSENSORUSART, O2SensorRxData, sizeof(O2SensorRxData) / sizeof(uint8_t));

	return HAL_OK;
}

/**
* @breif   	计算CRC16
* @param   	bufData 数组指针
* @param   	buflen 要计算的长度
* @retval  	CRC码
* @note     CRCLow = (CRC16Res & 0x00FF);
* 	     	CRCHigh = (CRC16Res >> 8);
*/
uint16_t claCRC16(const uint8_t *bufData, uint8_t bufLen)
{
	uint16_t crc = 0xFFFF;
	const uint16_t A001 = 0xA001;

	// 校验计算的长度为0
	if (bufLen == 0) {
		return 0;
	}

	for (uint8_t i = 0; i < bufLen; i++) {
		crc ^= bufData[i];
		// 总共八次右移操作
		for (uint8_t j = 0; j < 8; j++) {
			if ((crc & 0x0001) != 0) {
				// 右移的移出位为1
				crc >>= 1;
				crc ^= A001;
			} else {
                // 右移的移出位为0
				crc >>= 1;
			}
		}
	}

    return crc;
}


/**
* @breif   RS485发送指令获取溶解氧浓度和饱和度数据
*/
void O2SensorSendCmd_GetDOC(void)
{
	O2SensorTxData[0] = 0x01;	// 地址码
	O2SensorTxData[1] = 0x03;	// 功能码
	O2SensorTxData[2] = 0x00;	// 寄存器起始地址低位
	O2SensorTxData[3] = 0x00;	// 寄存器起始地址高位
	O2SensorTxData[4] = 0x00;	// 寄存器长度低位
	O2SensorTxData[5] = 0x06;	// 寄存器长度高位
	O2SensorTxData[6] = 0xC5;	// CRC校验码低位
	O2SensorTxData[7] = 0xC8;	// CRC校验码高位

	HAL_UART_Transmit(&DOCSENSORUSART, O2SensorTxData, sizeof(O2SensorTxData) / sizeof(uint8_t), 20);
//	HAL_UART_Transmit_IT
}

/**
* @breif   RS485发送指令获取PH值
*/
void PHSensorSendCmd_GetPH(void)
{
	PHSensorTxData[0] = 0x01;	// 地址码
	PHSensorTxData[1] = 0x03;	// 功能码
	PHSensorTxData[2] = 0x00;	// 寄存器起始地址低位
	PHSensorTxData[3] = 0x00;	// 寄存器起始地址高位
	PHSensorTxData[4] = 0x00;	// 寄存器长度低位
	PHSensorTxData[5] = 0x02;	// 寄存器长度高位
	PHSensorTxData[6] = 0xC4;	// CRC16校验码低位
	PHSensorTxData[7] = 0x0B;	// CRC16校验码高位

	HAL_UART_Transmit(&PHSENSORUSART, PHSensorTxData, sizeof(PHSensorTxData) / sizeof(uint8_t), 20);
}

/**
* @breif   解析氧浓度传感器通讯协议
* @param   rxData 接收数据指针
* @param   oContent 溶解氧饱和度
* @param   DOC 溶解氧浓度
*/
static void O2SensorAnalyzingDOC(const uint8_t *rxData, float *oContent, float *DOC)
{
	if ((claCRC16(rxData, 15) == (((uint16_t)rxData[16] << 8) | rxData[15]))
		&& rxData[0] == O2SensorTxData[0]
	    && rxData[1] == O2SensorTxData[1]
	    && rxData[2] == 0x0C) {

		fb.bytes[0] = rxData[3], fb.bytes[1] = rxData[4], fb.bytes[2] = rxData[5], fb.bytes[3] = rxData[6];
		*oContent = fb.floatValue;
		fb.bytes[0] = rxData[7], fb.bytes[1] = rxData[8], fb.bytes[2] = rxData[9], fb.bytes[3] = rxData[10];
		*DOC = fb.floatValue;
	}
}

/**
* @breif   解析PH传感器通讯协议
* @param   rxData 接收数据指针
* @retval  PH值
*/
static float PHSensorAnalyzingPH(const uint8_t *rxData)
{
	if ((claCRC16(rxData, 7) == (((uint16_t)rxData[8] << 8) | rxData[7]))
		&& rxData[0] == PHSensorTxData[0]
	    && rxData[1] == PHSensorTxData[1]
	    && rxData[2] == 0x04) {

		return (float)(((uint16_t)rxData[4] << 8) | rxData[3]) / 100.0f;
	} else {
		return 0;
	}
}


/**
 * @brief   串口接收中断回调函数
 * @note    reDefine
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	if (huart->Instance == USART1) {
	if (huart == &PHSENSORUSART) {
		if (__HAL_UART_GET_FLAG(&PHSENSORUSART, UART_FLAG_IDLE) != RESET) {
			__HAL_UART_CLEAR_IDLEFLAG(&PHSENSORUSART);
			PHVal = PHSensorAnalyzingPH(PHSensorRxData);

			if (PHQueueHandle != NULL) {
				osMessagePut(PHQueueHandle, PHVal, 0);
			}

			HAL_UART_Receive_IT(&PHSENSORUSART, PHSensorRxData, sizeof(PHSensorRxData) / sizeof(uint8_t));
		}
    } else if (huart == &DOCSENSORUSART) {
    	if (__HAL_UART_GET_FLAG(&DOCSENSORUSART, UART_FLAG_IDLE) != RESET) {
			__HAL_UART_CLEAR_IDLEFLAG(&DOCSENSORUSART);
			O2SensorAnalyzingDOC(O2SensorRxData, &oContent, &DOC);

			if (oContentQueueHandle != NULL) {
				osMessagePut(oContentQueueHandle, oContent, 0);
			}
			if (DOCQueueHandle != NULL) {
				osMessagePut(DOCQueueHandle, DOC, 0);
			}

			HAL_UART_Receive_IT(&DOCSENSORUSART, O2SensorRxData, sizeof(O2SensorRxData) / sizeof(uint8_t));
    	}
    }
}
