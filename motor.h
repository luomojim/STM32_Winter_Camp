#ifndef __MOTOR_H
#define __MOTOR_H
#include "stm32f10x.h"

#define MOTOR_PORT GPIOA
#define IN1_PIN GPIO_Pin_0
#define IN2_PIN GPIO_Pin_1
#define IN3_PIN GPIO_Pin_2
#define IN4_PIN GPIO_Pin_3

#define MOTOR_TT1_PORT GPIOA
#define MOTOR_TT1_A GPIO_Pin_6
#define MOTOR_TT1_B GPIO_Pin_7

#define MOTOR_TT2_PORT GPIOB
#define MOTOR_TT2_A GPIO_Pin_6
#define MOTOR_TT2_B GPIO_Pin_7

#define MOTOR_TIM TIM3
#define MOTOR_TIM_CLK RCC_APB1Periph_TIM3

#define MOTOR_DIRECTION_FORWARD 0
#define MOTOR_DIRECTION_REVERSE 1

#define NORMAL_LEFT_SPEED 70.0f
#define NORMAL_RIGHT_SPEED 70.0f

#define TURN_SPEED 50.0f
#define BACK_SPEED 50.0f

#define IR_PORT GPIOA
#define RED1_PIN GPIO_Pin_4
#define RED2_PIN GPIO_Pin_5
// #define RED3_PIN GPIO_Pin_8
// #define RED4_PIN GPIO_Pin_9
#define RED5_PIN GPIO_Pin_11
#define RED6_PIN GPIO_Pin_12

#define ULTRASONIC_PORT GPIOB
#define ECHO_PIN GPIO_Pin_1
#define TRIG_PIN GPIO_Pin_0

void Motor_Init(void);
void Motor_SetDirection(uint8_t direction);
uint8_t Motor_GetDirection(void);
void Motor_Forward(float left_pwm, float right_pwm);
void Motor_Back(float left_pwm, float right_pwm);
void Motor_Left(float right_pwm);
void Motor_Right(float left_pwm);
void Motor_Stop(void);
void Motor_Left_Brake(void);
void Motor_Right_Brake(void);
void Motor_MoveBack(float cm);
void Motor_MoveForward(float cm, float left_speed, float right_speed);
void Motor_TurnRight90(void);
void Motor_TurnLeft90(void);
void Motor_ResumeNormal(void);

uint8_t IRSensor_Detect(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
float Test_Distance(void);

#endif
