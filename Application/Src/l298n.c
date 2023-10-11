/*
 * l298n.c
 *
 *  Created on: Jul 29, 2023
 *      Author: ittuann
 */

#include "l298n.h"
#include "main.h"
#include "time.h"

extern TIM_HandleTypeDef htim1;

#define PUMP_TIM			htim1
#define PUMP1_TIM_CH		TIM_CHANNEL_1
#define PUMP2_TIM_CH		TIM_CHANNEL_2

#define LIMIT_MIN_MAX(x, min, max) ((x) = (((x) <= (min)) ? (min) : (((x) >= (max)) ? (max) : (x))))

/**
  * @brief		电机PWM初始化
  */
void Peristaltic_Pump_Init(void)
{
	HAL_TIM_Base_Start(&PUMP_TIM);									// 开启定时器

	HAL_TIM_PWM_Start(&PUMP_TIM, PUMP1_TIM_CH);					// 使对应定时器的对应通道开始PWM输出
	HAL_TIM_PWM_Start(&PUMP_TIM, PUMP2_TIM_CH);

	__HAL_TIM_SetCompare(&PUMP_TIM, PUMP1_TIM_CH, __HAL_TIM_GetAutoreload(&PUMP_TIM));
	__HAL_TIM_SetCompare(&PUMP_TIM, PUMP2_TIM_CH, __HAL_TIM_GetAutoreload(&PUMP_TIM));

	HAL_GPIO_WritePin(M1IN1_GPIO_Port, M1IN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M1IN2_GPIO_Port, M1IN2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(M2IN1_GPIO_Port, M2IN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2IN2_GPIO_Port, M2IN2_Pin, GPIO_PIN_SET);
}

/**
  * @brief		设置电机转速百分比
  * @param   	num		电机编号 1左 2右
  * @param   	precent	电机转速百分比 (0-100)
  * @param   	dir		电机正反转 1正 0反
  */
void PeristalticPumpSet(uint8_t num, uint8_t precent, uint8_t dir)
{
	uint16_t arr = __HAL_TIM_GetAutoreload(&PUMP_TIM);

	LIMIT_MIN_MAX(precent, 0, 100);
	uint8_t deadZone = 50;

	if (num == 1) {
		if (dir) {
			HAL_GPIO_WritePin(M1IN1_GPIO_Port, M1IN1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(M1IN2_GPIO_Port, M1IN2_Pin, GPIO_PIN_RESET);
		} else {
			HAL_GPIO_WritePin(M1IN1_GPIO_Port, M1IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(M1IN2_GPIO_Port, M1IN2_Pin, GPIO_PIN_SET);
		}

		if (precent == 0) {
			HAL_GPIO_WritePin(M1IN1_GPIO_Port, M1IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(M1IN2_GPIO_Port, M1IN2_Pin, GPIO_PIN_RESET);
		} else {
			__HAL_TIM_SetCompare(&PUMP_TIM, PUMP1_TIM_CH, (arr * (100 - deadZone) * precent / 100 + arr * deadZone) / 100 + 1);
		}
	} else if (num == 2) {
		if (dir) {
			HAL_GPIO_WritePin(M2IN1_GPIO_Port, M2IN1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(M2IN2_GPIO_Port, M2IN2_Pin, GPIO_PIN_RESET);
		} else {
			HAL_GPIO_WritePin(M2IN1_GPIO_Port, M2IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(M2IN2_GPIO_Port, M2IN2_Pin, GPIO_PIN_SET);
		}

		if (precent == 0) {
			HAL_GPIO_WritePin(M2IN1_GPIO_Port, M2IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(M2IN2_GPIO_Port, M2IN2_Pin, GPIO_PIN_RESET);
		} else {
			__HAL_TIM_SetCompare(&PUMP_TIM, PUMP2_TIM_CH, (arr * (100 - deadZone) * precent / 100 + arr * deadZone) / 100 + 1);
		}
	}
}



