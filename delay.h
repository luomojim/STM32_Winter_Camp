#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

// 静态变量，用于存储每微秒和每毫秒的滴答数
static uint8_t fac_us = 0;
static uint16_t fac_ms = 0;

void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);
void Delay_s(uint32_t xs);
void delay_init(void);

#endif
