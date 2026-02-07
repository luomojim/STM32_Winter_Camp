#include "stm32f10x.h"
#include "pwm.h"
#include "delay.h"

/**
 * @brief PWM验证程序
 * 功能：验证PWM输出、占空比变化、参数传递和调速功能
 */

// 全局变量用于调试
volatile uint16_t g_pwm1 = 0;
volatile uint16_t g_pwm2 = 0;
volatile uint8_t g_test_step = 0;

/**
 * @brief 测试函数：验证PWM_SetCompare函数
 */
void test_pwm_set_compare(void)
{
	// 测试不同的PWM值
	uint16_t test_values[] = {0, 30, 50, 70, 100};
	int num_tests = sizeof(test_values) / sizeof(test_values[0]);
	
	for(int i=0; i<num_tests; i++) {
		uint16_t value = test_values[i];
		
		// 设置PWM值
		PWM_SetCompare1(value);
		PWM_SetCompare2(value);
		
		// 延迟一段时间观察
		for(int j=0; j<50; j++) {
			PWM_Task();
			Delay_ms(100);
		}
	}
}

/**
 * @brief 测试函数：验证左右电机独立控制
 */
void test_motor_independence(void)
{
	// 测试1：左停右转
	PWM_SetCompare1(0);
	PWM_SetCompare2(70);
	for(int i=0; i<150; i++) {
		PWM_Task();
		Delay_ms(20);
	}
	
	// 测试2：左转右停
	PWM_SetCompare1(70);
	PWM_SetCompare2(0);
	for(int i=0; i<150; i++) {
		PWM_Task();
		Delay_ms(20);
	}
	
	// 测试3：全停
	PWM_SetCompare1(0);
	PWM_SetCompare2(0);
	for(int i=0; i<100; i++) {
		PWM_Task();
		Delay_ms(20);
	}
}

/**
 * @brief 主验证函数
 */
int main(void)
{
	// 初始化系统
	SystemInit();
	
	// 初始化PWM
	PWM_Init();
	
	// 验证步骤1：测试PWM_SetCompare函数
	g_test_step = 1;

test_pwm_set_compare();
	
	// 验证步骤2：测试左右电机独立控制
	g_test_step = 2;

test_motor_independence();
	
	// 验证步骤3：测试占空比变化
	g_test_step = 3;
	for(int i=0; i<=100; i+=10) {
		PWM_SetCompare1(i);
		PWM_SetCompare2(i);
		for(int j=0; j<10; j++) {
			PWM_Task();
			Delay_ms(100);
		}
	}
	
	// 验证步骤4：测试参数传递
	g_test_step = 4;
	float left_pwm = 0.0f;
	float right_pwm = 70.0f;
	
	// 模拟从Motor_Forward函数传递参数
	PWM_SetCompare1((uint16_t)left_pwm);
	PWM_SetCompare2((uint16_t)right_pwm);
	
	// 持续运行以观察结果
	while(1) {
		PWM_Task();
	}
}