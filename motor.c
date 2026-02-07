#include "motor.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "pwm.h"
#include "delay.h"

static uint8_t motor_direction_config = MOTOR_DIRECTION_FORWARD;

static void Motor_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStruct.GPIO_Pin = IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = MOTOR_TT1_A | MOTOR_TT1_B;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MOTOR_TT1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = MOTOR_TT2_A | MOTOR_TT2_B;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MOTOR_TT2_PORT, &GPIO_InitStruct);

    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
}

void Motor_Init(void)
{
    Motor_GPIO_Init();
    PWM_Init();
}

void Motor_SetDirection(uint8_t direction)
{
    motor_direction_config = direction;
}

uint8_t Motor_GetDirection(void)
{
    return motor_direction_config;
}

static float limit_pwm(float pwm)
{
    return (pwm > 99) ? 99 : (pwm < 0) ? 0 : pwm;
}

static void move_delay(float cm, float speed)
{
    float time_ms = (cm * 60) / (speed / 10);
    if (speed <= 0)
        time_ms = 0;
    Delay_ms((uint16_t)time_ms);
}

void Motor_Forward(float left_pwm, float right_pwm)
{
    if (motor_direction_config == MOTOR_DIRECTION_FORWARD)
    {
        GPIO_SetBits(MOTOR_PORT, IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
        GPIO_SetBits(MOTOR_PORT, IN4_PIN);
    }
    else
    {
        GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
        GPIO_SetBits(MOTOR_PORT, IN2_PIN);
        GPIO_SetBits(MOTOR_PORT, IN3_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    }

    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);

    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);
}

void Motor_Back(float left_pwm, float right_pwm)
{
    if (motor_direction_config == MOTOR_DIRECTION_FORWARD)
    {
        GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
        GPIO_SetBits(MOTOR_PORT, IN2_PIN);
        GPIO_SetBits(MOTOR_PORT, IN3_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    }
    else
    {
        GPIO_SetBits(MOTOR_PORT, IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
        GPIO_SetBits(MOTOR_PORT, IN4_PIN);
    }

    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);

    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);
}

void Motor_Left(float right_pwm)
{
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    PWM_SetCompare1(0);

    if (motor_direction_config == MOTOR_DIRECTION_FORWARD)
    {
        GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
        GPIO_SetBits(MOTOR_PORT, IN4_PIN);
    }
    else
    {
        GPIO_SetBits(MOTOR_PORT, IN3_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    }

    right_pwm = limit_pwm(right_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);
}

void Motor_Right(float left_pwm)
{
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    PWM_SetCompare2(0);

    if (motor_direction_config == MOTOR_DIRECTION_FORWARD)
    {
        GPIO_SetBits(MOTOR_PORT, IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    }
    else
    {
        GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
        GPIO_SetBits(MOTOR_PORT, IN2_PIN);
    }

    left_pwm = limit_pwm(left_pwm);
    PWM_SetCompare1((uint16_t)left_pwm);
}

void Motor_Stop(void)
{
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
    PWM_SetCompare1(0);
    PWM_SetCompare2(0);
}

void Motor_Left_Brake(void)
{
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    PWM_SetCompare1(0);
}

void Motor_Right_Brake(void)
{
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    PWM_SetCompare2(0);
}

void Motor_MoveBack(float cm)
{
    Motor_Back(BACK_SPEED, BACK_SPEED);
    move_delay(cm, BACK_SPEED);
    Motor_Stop();
}

void Motor_MoveForward(float cm, float left_speed, float right_speed)
{
    Motor_Forward(left_speed, right_speed);
    move_delay(cm, (left_speed + right_speed) / 2);
    Motor_Stop();
}

void Motor_TurnRight90(void)
{
    Motor_Right_Brake();
    Motor_Right(TURN_SPEED);
    Delay_ms(800);
    Motor_Stop();
}

void Motor_TurnLeft90(void)
{
    Motor_Left_Brake();
    Motor_Left(TURN_SPEED);
    Delay_ms(800);
    Motor_Stop();
}

void Motor_ResumeNormal(void)
{
    Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
}
