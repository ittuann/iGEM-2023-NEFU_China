/*
 * adcsensor.c
 *
 *  Created on: Oct 5, 2023
 *      Author: ittuann
 */
#include "main.h"
#include "adc.h"
#include "adcsensor.h"
#include "filter.h"

volatile float ADCVrefintProportion = 8.0586080586080586080586080586081e-4f;

/**
  * @brief			获取ADC采样值
  * @retval			none
  * @example		getADCxChxValue(&hadc1, ADC_CHANNEL_1);
  */
uint16_t getADCxChxValue(ADC_HandleTypeDef *ADCx, uint32_t ch)
{
	ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ch;							// ADC转换通道
    sConfig.Rank = 1;								// ADC序列排序 即转换顺序
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;	// ADC采样时间

	HAL_ADC_ConfigChannel(ADCx, &sConfig);			// 设置ADC通道的各个属性值

    HAL_ADC_Start(ADCx);							// 开启ADC采样

    HAL_ADC_PollForConversion(ADCx, 1);				// 等待ADC转换结束

	if (HAL_IS_BIT_SET(HAL_ADC_GetState(ADCx), HAL_ADC_STATE_REG_EOC)) {	// 判断转换完成标志位是否设置
		return (uint16_t)(HAL_ADC_GetValue(ADCx));	// 获取ADC值
	} else {
		return 0;
	}
}

/**
  * @brief          ADC内部参考校准电压Vrefint初始化
  * @retval         none
  */
void ADCVrefintInit(void)
{
    uint32_t total_vrefint = 0;

    for (uint8_t i = 0; i < 200; i ++ ) {
        total_vrefint += getADCxChxValue(&hadc1, ADC_CHANNEL_VREFINT);
    }

    ADCVrefintProportion = 200 * 1.200f / total_vrefint;
}

/**
  * @brief          ADC采集STM32内部温度
  * @retval         none
  */
float ADCGetSTM32Temprate(void)
{
	float temperate = 0.000f;
    uint16_t adcx = 0;

    adcx = getADCxChxValue(&hadc1, ADC_CHANNEL_TEMPSENSOR);
    temperate = (float)adcx * ADCVrefintProportion;
    temperate = (temperate - 0.76f) * 400.0f + 25.0f;

    return temperate;
}

LpfSimple_t ADCPHSensorRAWVal;

uint16_t getADCPHSensor(void)
{
	return getADCxChxValue(&hadc1, ADC_CHANNEL_1);
//	SimpleFilter(&ADCPHSensorRAWVal, (int16_t)getADCxChxValue(&hadc1, ADC_CHANNEL_1));
//	return ADCPHSensorRAWVal.FilterData;
}

