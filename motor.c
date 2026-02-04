#include "motor.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "PID.h"

// 定义PWM相关常量
#define PWM_ARR 99  // PWM周期值
#define PWM_PSC 719 // PWM预分频值

// 内部PWM初始化函数
static void PWM_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能时钟：定时器3 + GPIOA + GPIOB + 复用功能
    RCC_APB1PeriphClockCmd(MOTOR_TIM_CLK, ENABLE);
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

    // 配置定时器3时间参数
    TIM_TimeBaseStruct.TIM_Period = arr;
    TIM_TimeBaseStruct.TIM_Prescaler = psc;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(MOTOR_TIM, &TIM_TimeBaseStruct);

    // 配置PWM输出模式（PWM1模式）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(MOTOR_TIM, &TIM_OCInitStruct);
    TIM_OC2Init(MOTOR_TIM, &TIM_OCInitStruct);

    // 启用PWM预加载和ARR预加载
    TIM_OC1PreloadConfig(MOTOR_TIM, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(MOTOR_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(MOTOR_TIM, ENABLE);

    // 启用定时器3
    TIM_Cmd(MOTOR_TIM, ENABLE);
}

void Motor_Init(void)
{
    // PWM初始化（ARR=99, PSC=719 -> PWM频率=72MHz/(719+1)/(99+1)=1kHz）
    PWM_Init(PWM_ARR, PWM_PSC); // 使用常量定义，提高可维护性
}

void Motor_Forward(void)
{
    // 设置电机正转方向电平
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    // 通过PID计算，设置直线行驶PWM=99（最大速度）
    float left_pwm = PID_Calc(&PID_MotorLeft, 99, TIM_GetCompare1(MOTOR_TIM));   // 目标PWM值 - 最大速度
    float right_pwm = PID_Calc(&PID_MotorRight, 99, TIM_GetCompare2(MOTOR_TIM)); // 目标PWM值 - 最大速度

    // PWM限幅（0~99）
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;

    // 设置PWM占空比
    TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
    TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
}

void Motor_Left(void)
{
    // 设置电机转向电平（左电机停，右电机转）
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    // 通过PID计算，设置转向速度PWM=50
    float right_pwm = PID_Calc(&PID_MotorRight, 50, TIM_GetCompare2(MOTOR_TIM)); // 目标PWM值 - 根据实际情况调整

    // PWM限幅
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;

    // 设置PWM占空比（左电机=0，右电机=计算值）
    TIM_SetCompare1(MOTOR_TIM, 0);
    TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
}

void Motor_Right(void)
{
    // 设置电机转向电平（右电机停，左电机转）
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN | IN4_PIN);

    // 通过PID计算，设置转向速度PWM=50
    float left_pwm = PID_Calc(&PID_MotorLeft, 50, TIM_GetCompare1(MOTOR_TIM)); // 目标PWM值 - 根据实际情况调整

    // PWM限幅
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;

    // 设置PWM占空比（左电机=计算值，右电机=0）
    TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
    TIM_SetCompare2(MOTOR_TIM, 0);
}

void Motor_Stop(void)
{
    // 全部引脚置低，停止电机
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);

    // PWM占空比设为0
    TIM_SetCompare1(MOTOR_TIM, 0);
    TIM_SetCompare2(MOTOR_TIM, 0);
}
