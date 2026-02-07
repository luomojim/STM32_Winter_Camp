#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define PWM_PERIOD 100

void PWM_Init(void)
{
    // GPIO_InitTypeDef GPIO_InitStructure;
    // TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // TIM_OCInitTypeDef TIM_OCInitStructure;

    // // 开启 TIM2 和 GPIOA 时钟
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // // 配置 PA0, PA1, PA2, PA3 为复用推挽输出
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA, &GPIO_InitStructure);

    // // 初始化 TIM2
    // TIM_TimeBaseStructure.TIM_Period = 99; // PWM 频率配置
    // TIM_TimeBaseStructure.TIM_Prescaler = 71;
    // TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    // TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // // 初始化 4 个通道的 PWM
    // TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    // TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    // TIM_OCInitStructure.TIM_Pulse = 0;
    // TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // TIM_OC1Init(TIM2, &TIM_OCInitStructure); // PA0
    // TIM_OC2Init(TIM2, &TIM_OCInitStructure); // PA1
    // TIM_OC3Init(TIM2, &TIM_OCInitStructure); // PA2
    // TIM_OC4Init(TIM2, &TIM_OCInitStructure); // PA3

    // TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    // TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    // TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    // TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // TIM_Cmd(TIM2, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare1(TIM3, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare2(TIM3, Compare);
}

void PWM_SetCompare3(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare1(TIM4, Compare);
}

void PWM_SetCompare4(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare2(TIM4, Compare);
}

void PWM_Task(void)
{
}

uint16_t PWM_GetCompare1(void)
{
    return TIM_GetCapture1(TIM3);
}

uint16_t PWM_GetCompare2(void)
{
    return TIM_GetCapture2(TIM3);
}

uint16_t PWM_GetCompare3(void)
{
    return TIM_GetCapture1(TIM4);
}

uint16_t PWM_GetCompare4(void)
{
    return TIM_GetCapture2(TIM4);
}
