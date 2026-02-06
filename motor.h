#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

// 电机驱动模块（IN1-IN4）硬件连接定义
#define MOTOR_PORT GPIOA // IN1-IN4 -> PA0-PA3
#define IN1_PIN GPIO_Pin_0
#define IN2_PIN GPIO_Pin_1
#define IN3_PIN GPIO_Pin_2
#define IN4_PIN GPIO_Pin_3

// 电机TT1硬件连接（1A->PA6, 1B->PA7）
#define MOTOR_TT1_PORT GPIOA
#define MOTOR_TT1_A GPIO_Pin_6
#define MOTOR_TT1_B GPIO_Pin_7

// 电机TT2硬件连接（2A->PB6, 2B->PB7）
#define MOTOR_TT2_PORT GPIOB
#define MOTOR_TT2_A GPIO_Pin_6
#define MOTOR_TT2_B GPIO_Pin_7

// 电机PWM定时器配置（使用TIM3，避免与TIM2冲突）
#define MOTOR_TIM TIM3
#define MOTOR_TIM_CLK RCC_APB1Periph_TIM3

// 函数声明
void Motor_Init(void);
void Motor_Forward(float left_pwm, float right_pwm);
void Motor_Left(float right_pwm);
void Motor_Right(float left_pwm);
void Motor_Stop(void);

#endif
