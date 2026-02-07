#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define PWM_PERIOD 100

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 启用TIM1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // 配置PA8和PA9为复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置TIM1时基
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;  // 72MHz / 72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置TIM1通道1 (PA8) 为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 配置TIM1通道2 (PA9) 为PWM模式
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 启用TIM1主输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    // 启用TIM1
    TIM_Cmd(TIM1, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare1(TIM1, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
    if (Compare > PWM_PERIOD)
    {
        Compare = PWM_PERIOD;
    }
    TIM_SetCompare2(TIM1, Compare);
}

void PWM_Task(void)
{
    // 硬件PWM不需要任务处理
}

uint16_t PWM_GetCompare1(void)
{
    return TIM_GetCapture1(TIM1);
}

uint16_t PWM_GetCompare2(void)
{
    return TIM_GetCapture2(TIM1);
}
