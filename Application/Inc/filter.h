/*
 * filter.h
 *
 *  Created on: Oct 5, 2023
 *      Author: ittuann
 */

#pragma once

#define SimpleFilterDepth 10
typedef struct {
	int16_t OriginData;
	int16_t FilterData;
	int16_t FilterBuff[SimpleFilterDepth];
} LpfSimple_t;

extern int16_t SimpleFilter(LpfSimple_t* lpf, int16_t rawData);
