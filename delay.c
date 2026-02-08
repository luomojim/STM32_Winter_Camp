#include "delay.h"

void delay_init()
{
    // 选择外部时钟  HCLK/8
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    // 系统时钟一般为72MHz，HCLK/8 = 9MHz
    // 比如72MHz系统时钟，SysTick频率为9MHz，即1s震动9M次
    // 1us震动9次
    fac_us = SystemCoreClock / 8000000;

    // 1ms震动次数 = 1us震动次数 * 1000
    fac_ms = (uint16_t)fac_us * 1000;
}

void Delay_us(uint32_t xus)
{
    if (xus == 0)
        return;
    SysTick->LOAD = 72 * xus - 1;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x00000005;
    while (!(SysTick->CTRL & 0x00010000))
        ;
    SysTick->CTRL = 0x00000004;
    SysTick->VAL = 0x00;
}

void Delay_ms(uint32_t xms)
{
    while (xms--)
    {
        Delay_us(1000);
    }
}

void Delay_s(uint32_t xs)
{
    while (xs--)
    {
        Delay_ms(1000);
    }
}
