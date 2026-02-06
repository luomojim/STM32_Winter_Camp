#include "motor.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "pwm.h"

// 内部电机GPIO初始化函数
static void Motor_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 配置IN1-IN4引脚：推挽输出模式
    GPIO_InitStruct.GPIO_Pin = IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出，用于控制电机方向
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

    // 配置TT1编码器引脚：推挽输出模式
    GPIO_InitStruct.GPIO_Pin = MOTOR_TT1_A | MOTOR_TT1_B;
    GPIO_Init(MOTOR_TT1_PORT, &GPIO_InitStruct);

    // 配置TT2编码器引脚：推挽输出模式
    GPIO_InitStruct.GPIO_Pin = MOTOR_TT2_A | MOTOR_TT2_B;
    GPIO_Init(MOTOR_TT2_PORT, &GPIO_InitStruct);
}

void Motor_Init(void)
{
    // 初始化电机GPIO
    Motor_GPIO_Init();
    // 初始化软件PWM
    PWM_Init();
}

void Motor_Forward(float left_pwm, float right_pwm)
{
    // 设置电机正转方向电平（左电机正转，右电机反转）
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_SetBits(MOTOR_PORT, IN4_PIN);

    // PWM限幅（0~99），允许加速时超过90
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;

    // 设置PWM占空比（使用软件PWM）
    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);
}

void Motor_Left(float right_pwm)
{
    // 设置电机转向电平（左电机停，右电机转）
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_SetBits(MOTOR_PORT, IN4_PIN);

    // PWM限幅
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;

    // 设置PWM占空比（左电机=0，右电机=计算值）
    PWM_SetCompare1(0);
    PWM_SetCompare2((uint16_t)right_pwm);
}

void Motor_Right(float left_pwm)
{
    // 设置电机转向电平（右电机停，左电机转）
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN | IN4_PIN);

    // PWM限幅
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;

    // 设置PWM占空比（左电机=计算值，右电机=0）
    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2(0);
}

void Motor_Stop(void)
{
    // 全部引脚置低，停止电机
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);

    // PWM占空比设为0
    PWM_SetCompare1(0);
    PWM_SetCompare2(0);
}
