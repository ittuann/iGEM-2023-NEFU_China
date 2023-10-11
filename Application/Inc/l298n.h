/*
 * l298n.h
 *
 *  Created on: Jul 29, 2023
 *      Author: ittuann
 */

#pragma once

#include "main.h"

void Peristaltic_Pump_Init(void);
void PeristalticPumpSet(uint8_t num, uint8_t precent, uint8_t dir);

