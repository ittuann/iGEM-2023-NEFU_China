/*
 * rs485.h
 *
 *  Created on: Jul 28, 2023
 *      Author: ittuann
 */

#pragma once

#include "main.h"

HAL_StatusTypeDef RS485ReceiveIT_Init(void);
void O2SensorSendCmd_GetDOC(void);
void PHSensorSendCmd_GetPH(void);
