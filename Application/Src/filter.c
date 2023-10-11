/*
 * filter.c
 *
 *  Created on: Oct 5, 2023
 *      Author: ittuann
 */
#include <math.h>
#include <string.h>
#include "main.h"
#include "filter.h"

typedef unsigned char bool_t;

/**
 * @brief			快速排序
 * @param[out]		q : 待排数组
 * @param[in]		l / r : 左右端点（闭区间
 * @param[in]		order 0为从小到大升序 1为从大到小降序
 */
void Quick_Sort(int16_t q[], int16_t l, int16_t r, bool_t order)
{
    if (l >= r) return;
    int16_t i = l - 1, j = r + 1, x = q[(l + r) >> 1];
    while (i < j) {
    	if (order) {
			do i ++ ; while (q[i] < x);
			do j -- ; while (q[j] > x);
    	} else {
            do i ++ ; while (q[i] > x);
            do j -- ; while (q[j] < x);
    	}
        if (i < j) {
        	int16_t k = q[i];
        	q[i] = q[j];
        	q[j] = k;
        }
    }
    Quick_Sort(q, l, j, order), Quick_Sort(q, j + 1, r, order);
}

/**
 * @brief			简单滤波
 * @param[out]		lpf : 滤波结构数据指针
 * @param[in]		rawData : 原始数据
 */
int16_t SimpleFilter(LpfSimple_t* lpf, int16_t rawData)
{
	int16_t FilterBuffSort[SimpleFilterDepth] = {0};	// 暂存排序值

    lpf->OriginData = rawData;
    // 滑动递推更新旧值
    for (uint16_t i = 1; i < SimpleFilterDepth; i ++ ) {
    	lpf->FilterBuff[i] = lpf->FilterBuff[i - 1];
    }
	// 简单强制消抖 加权存入新值
    if ((fabs(lpf->OriginData - lpf->FilterBuff[0])) < 2.0f) {
    	lpf->FilterBuff[0] = lpf->OriginData * 0.70f + lpf->FilterBuff[0] * 0.30f;
    } else {
    	lpf->FilterBuff[0] = lpf->OriginData;
    }
	// 存储待排序值
	memcpy(FilterBuffSort, lpf->FilterBuff, sizeof(lpf->FilterBuff));
	// 升序排序
	Quick_Sort(FilterBuffSort, 0, sizeof(lpf->FilterBuff) - 1, 0);
	// 中值滤波
	lpf->FilterBuff[0] = 0;    // 先清零
	for (uint16_t i = 2; i < (SimpleFilterDepth - 2); i ++ ) {
		lpf->FilterBuff[0] += FilterBuffSort[i];
	}
	lpf->FilterBuff[0] = lpf->FilterBuff[0] / (SimpleFilterDepth - 4);	// 去除四个极值再计算均值
	lpf->FilterData = lpf->FilterBuff[0];
	return lpf->FilterData;
}

