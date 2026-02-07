#include "stm32f10x.h"
#include "delay.h"
// PWM配置参数
#define PWM_PERIOD 100										 // PWM周期（100个单位）
#define PWM_FREQUENCY 1000									 // PWM频率（Hz）
#define PWM_UNIT_TIME (1000000 / PWM_FREQUENCY / PWM_PERIOD) // 每个PWM单位的微秒数
// PWM占空比变量
static uint16_t pwm_compare1 = 0;
static uint16_t pwm_compare2 = 0;
static uint8_t pwm_running = 0;
void PWM_Init(void)
{
	// 启用GPIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// 配置GPIO为推挽输出模式
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 初始化输出为低电平
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	// 标记PWM已初始化
	pwm_running = 1;
}
void PWM_SetCompare1(uint16_t Compare)
{
	// 限制占空比范围
	if (Compare > PWM_PERIOD)
	{
		Compare = PWM_PERIOD;
	}
	pwm_compare1 = Compare;
}
void PWM_SetCompare2(uint16_t Compare)
{
	// 限制占空比范围
	if (Compare > PWM_PERIOD)
	{
		Compare = PWM_PERIOD;
	}
	pwm_compare2 = Compare;
}
// PWM任务函数，需要在主循环中调用
void PWM_Task(void)
{
	if (!pwm_running)
	{
		return;
	}
	// 生成PWM波形，同时设置PA8和PA9的高电平
	if (pwm_compare1 > 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8); // 显式设置为低电平
	}
	if (pwm_compare2 > 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_9);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_9); // 显式设置为低电平
	}
	// 延时高电平时间，取最大的占空比
	uint16_t max_compare = (pwm_compare1 > pwm_compare2) ? pwm_compare1 : pwm_compare2;
	Delay_us(max_compare * PWM_UNIT_TIME);
	// 同时置低PA8和PA9
	if (pwm_compare1 > 0)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	}
	if (pwm_compare2 > 0)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	}
	// 延时剩余时间，保持周期稳定
	if (max_compare < PWM_PERIOD)
	{
		Delay_us((PWM_PERIOD - max_compare) * PWM_UNIT_TIME);
	}
}
