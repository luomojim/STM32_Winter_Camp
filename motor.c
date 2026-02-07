#include "motor.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

/* * 硬件连接 (基于提供的原理图):
 * 左电机 (A): PA0 (TIM2_CH1), PA1 (TIM2_CH2)
 * 右电机 (B): PA2 (TIM2_CH3), PA3 (TIM2_CH4)
 * 驱动器: DRV8833 (不需要独立Enable脚，直接PWM驱动方向脚)
 */

// 限制PWM范围在 0-100
static float limit_pwm(float pwm)
{
    if (pwm > 100)
        return 100;
    if (pwm < 0)
        return 0;
    return pwm;
}

// 延时辅助函数
static void move_delay(float cm, float speed)
{
    // 简单的估算：假设速度系数，具体需根据实际跑动距离校准
    float time_ms = (cm * 60) / (speed / 10);
    if (speed <= 0)
        time_ms = 0;
    Delay_ms((uint16_t)time_ms);
}

void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 1. 开启 TIM2 和 GPIOA 时钟
    // 注意：TIM2 是 APB1 总线
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. 配置 PA0, PA1, PA2, PA3 为复用推挽输出 (AF_PP)
    // 这些引脚将输出 PWM 波形
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置 TIM2 时基
    // 频率 = 72MHz / (71+1) / (99+1) = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 99;    // ARR
    TIM_TimeBaseStructure.TIM_Prescaler = 71; // PSC
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 4. 配置 TIM2 的 4 个通道为 PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure); // PA0
    TIM_OC2Init(TIM2, &TIM_OCInitStructure); // PA1
    TIM_OC3Init(TIM2, &TIM_OCInitStructure); // PA2
    TIM_OC4Init(TIM2, &TIM_OCInitStructure); // PA3

    // 使能预装载寄存器
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // 5. 开启定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 停止所有电机
void Motor_Stop(void)
{
    TIM_SetCompare1(TIM2, 0); // PA0 = 0
    TIM_SetCompare2(TIM2, 0); // PA1 = 0
    TIM_SetCompare3(TIM2, 0); // PA2 = 0
    TIM_SetCompare4(TIM2, 0); // PA3 = 0
}

// 前进: IN1/IN3 输出 PWM, IN2/IN4 保持低电平
void Motor_Forward(float left_pwm, float right_pwm)
{
    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);

    // 左轮前进 (PA0 PWM, PA1 Low)
    TIM_SetCompare1(TIM2, (uint16_t)left_pwm);
    TIM_SetCompare2(TIM2, 0);

    // 右轮前进 (PA2 PWM, PA3 Low)
    TIM_SetCompare3(TIM2, (uint16_t)right_pwm);
    TIM_SetCompare4(TIM2, 0);
}

// 后退: IN2/IN4 输出 PWM, IN1/IN3 保持低电平
void Motor_Back(float left_pwm, float right_pwm)
{
    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);

    // 左轮后退 (PA0 Low, PA1 PWM)
    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare2(TIM2, (uint16_t)left_pwm);

    // 右轮后退 (PA2 Low, PA3 PWM)
    TIM_SetCompare3(TIM2, 0);
    TIM_SetCompare4(TIM2, (uint16_t)right_pwm);
}

// 左转: 左轮不动(或后退)，右轮前进
// 这里实现为原地旋转（左轮后退，右轮前进），或者单轮转动，取决于你的习惯
// 下面代码实现的是：左轮停止，右轮前进
void Motor_Left(float right_pwm)
{
    right_pwm = limit_pwm(right_pwm);

    // 左轮停止
    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare2(TIM2, 0);

    // 右轮前进
    TIM_SetCompare3(TIM2, (uint16_t)right_pwm);
    TIM_SetCompare4(TIM2, 0);
}

// 右转: 左轮前进，右轮停止
void Motor_Right(float left_pwm)
{
    left_pwm = limit_pwm(left_pwm);

    // 左轮前进
    TIM_SetCompare1(TIM2, (uint16_t)left_pwm);
    TIM_SetCompare2(TIM2, 0);

    // 右轮停止
    TIM_SetCompare3(TIM2, 0);
    TIM_SetCompare4(TIM2, 0);
}

// 左刹车
void Motor_Left_Brake(void)
{
    TIM_SetCompare1(TIM2, 100); // DRV8833 两个脚都高电平是急刹，都低电平是滑行
    TIM_SetCompare2(TIM2, 100); // 这里为了简单，设为0滑行停止，或者设为100急刹
    // 为了保险起见，先用滑行停止
    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare2(TIM2, 0);
}

// 右刹车
void Motor_Right_Brake(void)
{
    TIM_SetCompare3(TIM2, 0);
    TIM_SetCompare4(TIM2, 0);
}

// 封装的高级功能
void Motor_MoveForward(float cm, float left_speed, float right_speed)
{
    Motor_Forward(left_speed, right_speed);
    move_delay(cm, (left_speed + right_speed) / 2);
    Motor_Stop();
}

void Motor_MoveBack(float cm)
{
    Motor_Back(BACK_SPEED, BACK_SPEED);
    move_delay(cm, BACK_SPEED);
    Motor_Stop();
}

void Motor_TurnRight90(void)
{
    // 原地右转：左轮前，右轮后
    TIM_SetCompare1(TIM2, (uint16_t)TURN_SPEED); // 左轮前
    TIM_SetCompare2(TIM2, 0);
    TIM_SetCompare3(TIM2, 0);
    TIM_SetCompare4(TIM2, (uint16_t)TURN_SPEED); // 右轮后

    Delay_ms(400); // 90度的时间需要根据实际情况调整
    Motor_Stop();
}

void Motor_TurnLeft90(void)
{
    // 原地左转：左轮后，右轮前
    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare2(TIM2, (uint16_t)TURN_SPEED); // 左轮后
    TIM_SetCompare3(TIM2, (uint16_t)TURN_SPEED); // 右轮前
    TIM_SetCompare4(TIM2, 0);

    Delay_ms(400); // 90度的时间需要根据实际情况调整
    Motor_Stop();
}

void Motor_ResumeNormal(void)
{
    Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
}
