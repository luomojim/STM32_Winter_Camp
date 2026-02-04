#ifndef __IR_SENSOR_H
#define __IR_SENSOR_H

#include "stm32f10x.h"

// 红外传感器硬件连接定义
#define IR_PORT        GPIOA
#define RED1_PIN       GPIO_Pin_4  // RED1 -> PA4
#define RED2_PIN       GPIO_Pin_5  // RED2 -> PA5
#define RED3_PIN       GPIO_Pin_8  // RED3 -> PA8
#define RED4_PIN       GPIO_Pin_9  // RED4 -> PA9
#define RED5_PIN       GPIO_Pin_11 // RED5 -> PA11
#define RED6_PIN       GPIO_Pin_12 // RED6 -> PA12

// 传感器返回值定义
#define IR_HAVE_OBSTACLE  1   // Detected obstacle
#define IR_NO_OBSTACLE    0   // No obstacle detected

// 函数声明
void IRSensor_Init(void);
uint8_t IRSensor_Detect(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif
