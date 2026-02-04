#include "delay.h"

void Delay_us(uint32_t xus)
{
    if(xus == 0) return;
    SysTick->LOAD = 72 * xus - 1;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x00000005;
    while(!(SysTick->CTRL & 0x00010000));
    SysTick->CTRL = 0x00000004;
    SysTick->VAL = 0x00;
}

void Delay_ms(uint32_t xms)
{
    while(xms--)
    {
        Delay_us(1000);
    }
}

void Delay_s(uint32_t xs)
{
    while(xs--)
    {
        Delay_ms(1000);
    }
}
