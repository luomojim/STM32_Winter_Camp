#include "stm32f10x.h"
#include "delay.h"

// PWM????
#define PWM_PERIOD 100										 // PWM??(100???)
#define PWM_FREQUENCY 1000									 // PWM??(Hz)
#define PWM_UNIT_TIME (1000000 / PWM_FREQUENCY / PWM_PERIOD) // ??PWM??????

// PWM?????
static uint16_t pwm_compare1 = 0;
static uint16_t pwm_compare2 = 0;
static uint8_t pwm_running = 0;

void PWM_Init(void)
{
	// ??GPIO??
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	// ??GPIO???????
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ?????????
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	GPIO_ResetBits(GPIOA, GPIO_Pin_9);

	// ??PWM????
	pwm_running = 1;
}

void PWM_SetCompare1(uint16_t Compare)
{
	// ???????
	if (Compare > PWM_PERIOD)
	{
		Compare = PWM_PERIOD;
	}
	pwm_compare1 = Compare;
}

void PWM_SetCompare2(uint16_t Compare)
{
	// ???????
	if (Compare > PWM_PERIOD)
	{
		Compare = PWM_PERIOD;
	}
	pwm_compare2 = Compare;
}

// PWM????,?????????
void PWM_Task(void)
{
	if (!pwm_running)
	{
		return;
	}

	// ??PWM??
	if (pwm_compare1 > 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
	if (pwm_compare2 > 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_9);
	}

	// ???????
	Delay_us(pwm_compare1 * PWM_UNIT_TIME);
	if (pwm_compare1 > 0)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	}

	Delay_us(pwm_compare2 * PWM_UNIT_TIME);
	if (pwm_compare2 > 0)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	}

	// ??????,??????
	uint16_t max_compare = (pwm_compare1 > pwm_compare2) ? pwm_compare1 : pwm_compare2;
	if (max_compare < PWM_PERIOD)
	{
		Delay_us((PWM_PERIOD - max_compare) * PWM_UNIT_TIME);
	}
}
