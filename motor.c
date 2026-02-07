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

    // 配置IN1-IN4引脚：推挽输出模式（DRV8833方向控制）
    GPIO_InitStruct.GPIO_Pin = IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStruct);

    // 配置TT1编码器引脚：上拉输入模式（编码器为输入设备，不能用输出模式）
    GPIO_InitStruct.GPIO_Pin = MOTOR_TT1_A | MOTOR_TT1_B;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入，适配编码器的输出信号
    GPIO_Init(MOTOR_TT1_PORT, &GPIO_InitStruct);

    // 配置TT2编码器引脚：上拉输入模式
    GPIO_InitStruct.GPIO_Pin = MOTOR_TT2_A | MOTOR_TT2_B;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(MOTOR_TT2_PORT, &GPIO_InitStruct);

    // 初始化所有方向引脚为低电平（DRV8833刹车状态）
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
}

void Motor_Init(void)
{
    // 初始化电机GPIO
    Motor_GPIO_Init();
    // 初始化软件PWM
    PWM_Init();
}

// 适配DRV8833的正转函数：左电机IN1=1，IN2=0；右电机IN3=1，IN4=0
void Motor_Forward(float left_pwm, float right_pwm)
{
    // 设置DRV8833正转电平
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    
    // PWM限幅，确保在0-99范围内
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;

    // 设置PWM占空比
    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);
}

// 适配DRV8833的左转函数：左电机刹车，右电机正转
void Motor_Left(float right_pwm)
{
    // 左电机刹车：IN1=0，IN2=0
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    PWM_SetCompare1(0);

    // 右电机正转：IN3=1，IN4=0
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    if (right_pwm > 99)
        right_pwm = 99;
    if (right_pwm < 0)
        right_pwm = 0;
    PWM_SetCompare2((uint16_t)right_pwm);
}

// 适配DRV8833的右转函数：右电机刹车，左电机正转
void Motor_Right(float left_pwm)
{
    // 右电机刹车：IN3=0，IN4=0
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    PWM_SetCompare2(0);

    // 左电机正转：IN1=1，IN2=0
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    if (left_pwm > 99)
        left_pwm = 99;
    if (left_pwm < 0)
        left_pwm = 0;
    PWM_SetCompare1((uint16_t)left_pwm);
}

// 适配DRV8833的停止函数：双电机刹车
void Motor_Stop(void)
{
    // DRV8833刹车：所有方向引脚置低，PWM置0
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
    PWM_SetCompare1(0);
    PWM_SetCompare2(0);
}

// 左电机单独刹车函数（DRV8833专属）
void Motor_Left_Brake(void)
{
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    PWM_SetCompare1(0);
}

// 右电机单独刹车函数（DRV8833专属）
void Motor_Right_Brake(void)
{
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    PWM_SetCompare2(0);
}
