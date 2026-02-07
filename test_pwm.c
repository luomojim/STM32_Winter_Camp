#include "stm32f10x.h"
#include "pwm.h"
#include "delay.h"

int main(void)
{
	// 初始化PWM
	PWM_Init();
	
	// 测试1：左停右转（TT1=0, TT2=70）
	PWM_SetCompare1(0);  // 左电机PWM=0
	PWM_SetCompare2(70); // 右电机PWM=70
	
	// 持续运行PWM任务
	while(1)
	{
		PWM_Task();
	}
}