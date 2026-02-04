#ifndef __ULTRASOUND_H
#define __ULTRASOUND_H

#include "stm32f10x.h"

// 超声传感器硬件连接（TRIG->PB0, ECHO->PB1）
#define ULTRASONIC_TRIG_PIN GPIO_Pin_0
#define ULTRASONIC_ECHO_PIN GPIO_Pin_1
#define ULTRASONIC_PORT GPIOB

// 超声传感器参数
#define ULTRASONIC_DELAY_TRIGGER 5.0f // 减速触发距离5cm
#define ULTRASONIC_STOP_TRIGGER 1.0f  // 停车触发距离1cm
#define ULTRASONIC_INVALID_LIMIT 2    // 无效值计数阈值

// 函数声明
void Ultrasonic_Init(void);
float Test_Distance(void);

#endif
