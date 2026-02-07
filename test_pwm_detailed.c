#include "stm32f10x.h"
#include "pwm.h"
#include "delay.h"

/**
 * @brief 测试PWM输出
 * 功能：验证PWM输出是否生效，占空比是否随参数变化
 */
void test_pwm_output(void)
{
	// 初始化PWM
	PWM_Init();
	
	// 测试序列
	while(1)
	{
		// 测试1：左停右转（TT1=0, TT2=70）- 3秒
		PWM_SetCompare1(0);
		PWM_SetCompare2(70);
		for(int i=0; i<300; i++) {
			PWM_Task();
			Delay_ms(10);
		}
		
		// 测试2：左转右停（TT1=70, TT2=0）- 3秒
		PWM_SetCompare1(70);
		PWM_SetCompare2(0);
		for(int i=0; i<300; i++) {
			PWM_Task();
			Delay_ms(10);
		}
		
		// 测试3：全停（TT1=0, TT2=0）- 2秒
		PWM_SetCompare1(0);
		PWM_SetCompare2(0);
		for(int i=0; i<200; i++) {
			PWM_Task();
			Delay_ms(10);
		}
		
		// 测试4：占空比变化（TT1=30, TT2=30）- 2秒
		PWM_SetCompare1(30);
		PWM_SetCompare2(30);
		for(int i=0; i<200; i++) {
			PWM_Task();
			Delay_ms(10);
		}
		
		// 测试5：占空比变化（TT1=100, TT2=100）- 2秒
		PWM_SetCompare1(100);
		PWM_SetCompare2(100);
		for(int i=0; i<200; i++) {
			PWM_Task();
			Delay_ms(10);
		}
	}
}

/**
 * @brief 测试参数传递
 * 功能：验证从Motor_Forward到PWM_SetCompare的参数传递
 */
void test_parameter_passing(void)
{
	// 初始化PWM
	PWM_Init();
	
	// 模拟Motor_Forward函数的参数传递
	float left_pwm = 0.0f;  // 左电机PWM=0
	float right_pwm = 70.0f; // 右电机PWM=70
	
	// 限制占空比范围
	if (left_pwm > 100) left_pwm = 100;
	if (left_pwm < 0) left_pwm = 0;
	if (right_pwm > 100) right_pwm = 100;
	if (right_pwm < 0) right_pwm = 0;
	
	// 设置PWM值
	PWM_SetCompare1((uint16_t)left_pwm);
	PWM_SetCompare2((uint16_t)right_pwm);
	
	// 持续运行
	while(1)
	{
		PWM_Task();
	}
}

int main(void)
{
	// 选择测试模式
	test_pwm_output();
	// test_parameter_passing();
}