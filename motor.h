#ifndef __MOTOR_H
#define __MOTOR_H
#include "stm32f10x.h"

// ??????:DRV8833
// ?PWM??:IN1/IN2??????,IN3/IN4???????
// ??DRV8833?PWM??:
// - IN1 > IN2: ??
// - IN1 < IN2: ??
// - IN1 = IN2: ??

#define MOTOR_PORT GPIOA
#define IN1_PIN GPIO_Pin_0 // ???IN1 (PA0, TIM2_CH1)
#define IN2_PIN GPIO_Pin_1 // ???IN2 (PA1, TIM2_CH2)
#define IN3_PIN GPIO_Pin_2 // ???IN3 (PA2, TIM2_CH3)
#define IN4_PIN GPIO_Pin_3 // ???IN4 (PA3, TIM2_CH4)

// ?????(???)
#define MOTOR_TT1_PORT GPIOA
#define MOTOR_TT1_A GPIO_Pin_6
#define MOTOR_TT1_B GPIO_Pin_7

#define MOTOR_TT2_PORT GPIOB
#define MOTOR_TT2_A GPIO_Pin_6
#define MOTOR_TT2_B GPIO_Pin_7

// ????
#define NORMAL_LEFT_SPEED 84.0f  // ???????
#define NORMAL_RIGHT_SPEED 79.5f // ???????(??????)

#define TURN_SPEED 90.0f // ????
#define BACK_SPEED 90.0f // ????

// ?????????
#define IR_PORT GPIOA
#define RED1_PIN GPIO_Pin_4  // PA4
#define RED2_PIN GPIO_Pin_5  // PA5
#define RED3_PIN GPIO_Pin_8  // PA8
#define RED4_PIN GPIO_Pin_9  // PA9
#define RED5_PIN GPIO_Pin_11 // PA11
#define RED6_PIN GPIO_Pin_12 // PA12

// ???????
#define ULTRASONIC_PORT GPIOB
#define ECHO_PIN GPIO_Pin_1 // PB1
#define TRIG_PIN GPIO_Pin_0 // PB0

// ????
void Motor_Init(void);
void Motor_Stop(void);
void Motor_Forward(float left_pwm, float right_pwm);
void Motor_Back(float left_pwm, float right_pwm);
void Motor_Left(float right_pwm);
void Motor_Right(float left_pwm);
void Motor_Left_Brake(void);
void Motor_Right_Brake(void);
void Motor_MoveBack(float cm);
void Motor_MoveForward(float cm, float left_speed, float right_speed);
void Motor_TurnRight90(void);
void Motor_TurnLeft90(void);
void Motor_ResumeNormal(void);

#endif