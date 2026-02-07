#ifndef __MOTOR_H
#define __MOTOR_H
#include "stm32f10x.h"

/************************ 电机驱动引脚（匹配硬件） ************************/
#define MOTOR_PORT GPIOA 
#define IN1_PIN GPIO_Pin_0  // DRV8833 IN1 → PA0
#define IN2_PIN GPIO_Pin_1  // DRV8833 IN2 → PA1
#define IN3_PIN GPIO_Pin_2  // DRV8833 IN3 → PA2
#define IN4_PIN GPIO_Pin_3  // DRV8833 IN4 → PA3

/************************ 编码器引脚（匹配硬件） ************************/
#define MOTOR_TT1_PORT GPIOA
#define MOTOR_TT1_A GPIO_Pin_6  // TT1 1A → PA6
#define MOTOR_TT1_B GPIO_Pin_7  // TT1 1B → PA7

#define MOTOR_TT2_PORT GPIOB
#define MOTOR_TT2_A GPIO_Pin_6  // TT2 2A → PB6
#define MOTOR_TT2_B GPIO_Pin_7  // TT2 2B → PB7

/************************ 定时器配置 ************************/
#define MOTOR_TIM TIM3
#define MOTOR_TIM_CLK RCC_APB1Periph_TIM3

/************************ 红外/超声引脚（匹配硬件） ************************/
// 红外引脚
#define IR_PORT GPIOA
#define RED1_PIN GPIO_Pin_4  // RED1 → PA4
#define RED2_PIN GPIO_Pin_5  // RED2 → PA5
#define RED3_PIN GPIO_Pin_8  // RED3 → PA8
#define RED4_PIN GPIO_Pin_9  // RED4 → PA9
#define RED5_PIN GPIO_Pin_11 // RED5 → PA11
#define RED6_PIN GPIO_Pin_12 // RED6 → PA12

// 超声引脚
#define ULTRASONIC_PORT GPIOB
#define ECHO_PIN GPIO_Pin_1  // ECHO → PB1
#define TRIG_PIN GPIO_Pin_0  // TRIG → PB0

// 函数声明
void Motor_Init(void);
void Motor_Forward(float left_pwm, float right_pwm);
void Motor_Left(float right_pwm);
void Motor_Right(float left_pwm);
void Motor_Stop(void);
void Motor_Left_Brake(void);
void Motor_Right_Brake(void);

// 红外/超声辅助函数声明
uint8_t IRSensor_Detect(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
float Test_Distance(void);

#endif
