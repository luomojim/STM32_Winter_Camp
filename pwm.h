#ifndef PWM_H
#define PWM_H

#include "stm32f10x.h"

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetCompare2(uint16_t Compare);
void PWM_Task(void);
uint16_t PWM_GetCompare1(void);
uint16_t PWM_GetCompare2(void);

#endif
