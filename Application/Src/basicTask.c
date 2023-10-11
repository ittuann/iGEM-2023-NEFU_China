/*
 * basicTask.c
 *
 *  Created on: Jul 28, 2023
 *      Author: ittuann
 */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "rs485.h"
#include "l298n.h"
#include "adcsensor.h"

extern osMessageQId PHQueueHandle;
extern osMessageQId DOCQueueHandle;
extern osMessageQId oContentQueueHandle;

static float PHVal;
static float DOC;
static float oContent;

float PHADCOffset = 0.00f;
uint16_t PHADCRawVal;
float PHADCVoltageVal;
float PHADCVal;

uint8_t PPump1Speed = 20;
uint8_t PPump1Dir = 0;
uint8_t PPump2Speed = 1;
uint8_t PPump2Dir = 0;

/**
  * @brief  Function implementing the basicTask thread.
  * @param  argument: Not used
  * @retval None
  */
void BasicTask(void const * argument)
{
	static portTickType xLastWakeTime;
	const portTickType xTimeIncrement = pdMS_TO_TICKS(500UL);	// 绝对延时

	osEvent evt;

	ADCVrefintInit();

	// 用当前tick时间初始化 pxPreviousWakeTime
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		// 任务绝对延时
		vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);

		PeristalticPumpSet(1, PPump1Speed, PPump1Dir);
		PeristalticPumpSet(2, PPump2Speed, PPump2Dir);

		O2SensorSendCmd_GetDOC();
		PHSensorSendCmd_GetPH();

		PHADCRawVal = getADCPHSensor();
		PHADCVoltageVal = PHADCRawVal * 3.3f / 4096.0f;
		PHADCVal = PHADCVoltageVal * 3.5f + PHADCOffset;

		evt = osMessageGet(PHQueueHandle, 1);
		if (evt.status == osEventMessage) {
			PHVal = evt.value.v;
		}
		evt = osMessageGet(DOCQueueHandle, 1);
		if (evt.status == osEventMessage) {
			DOC = evt.value.v;
		}
		evt = osMessageGet(oContentQueueHandle, 1);
		if (evt.status == osEventMessage) {
			oContent = evt.value.v;
		}

	}
}
