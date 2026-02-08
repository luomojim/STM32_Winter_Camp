#include "motor.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

/* 
 * DRV8833双PWM模式控制逻辑：
 * 右电机 (IN1/IN2, PA0/PA1):
 *   - 前进: IN1 = PWM, IN2 = 0
 *   - 后退: IN1 = 0, IN2 = PWM
 *   - 停止: IN1 = 0, IN2 = 0
 * 
 * 左电机 (IN3/IN4, PA2/PA3):
 *   - 前进: IN3 = 0, IN4 = PWM
 *   - 后退: IN3 = PWM, IN4 = 0
 *   - 停止: IN3 = 0, IN4 = 0
 * 
 * 注意：PA0=IN1, PA1=IN2, PA2=IN3, PA3=IN4
 */

// 限制PWM范围在 0-99
static float limit_pwm(float pwm)
{
    if (pwm > 99) return 99;
    if (pwm < 0) return 0;
    return pwm;
}

// 延时辅助函数（修正计算）
static void move_delay(float cm, float speed_cm_per_sec)
{
    if (speed_cm_per_sec <= 0) return;
    
    // 时间(ms) = 距离(cm) * 1000 / 速度(cm/s)
    float time_ms = (cm * 1000.0f) / speed_cm_per_sec;
    
    // 假设速度70对应约15cm/s（需要实际校准）
    float calibrated_speed = speed_cm_per_sec * 0.2f; // 调整系数
    
    // 限制最小延迟
    if (time_ms < 10) time_ms = 10;
    if (time_ms > 10000) time_ms = 10000;
    
    Delay_ms((uint16_t)time_ms);
}

// 速度转PWM（DRV8833需要）
static uint16_t speed_to_pwm(float speed_percent)
{
    float pwm = speed_percent;
    if (pwm > 99) pwm = 99;
    if (pwm < 0) pwm = 0;
    return (uint16_t)pwm;
}

void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能TIM2和GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置PA0~PA3为复用推挽输出（PWM输出）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置TIM2基础参数
    TIM_TimeBaseStructure.TIM_Period = 99;          // PWM周期为100
    TIM_TimeBaseStructure.TIM_Prescaler = 71;       // 预分频72
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 配置TIM2通道参数（PWM模式1）
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  // 高电平有效

    // 初始化4个通道
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);  // CH1 -> PA0 (右电机IN1)
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);  // CH2 -> PA1 (右电机IN2)
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);  // CH3 -> PA2 (左电机IN3)
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // CH4 -> PA3 (左电机IN4)

    // 启用预装载
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // 使能TIM2
    TIM_Cmd(TIM2, ENABLE);
    
    // 初始状态：停止所有电机
    Motor_Stop();
}

// 停止所有电机
void Motor_Stop(void)
{
    TIM_SetCompare1(TIM2, 0);  // IN1 = 0
    TIM_SetCompare2(TIM2, 0);  // IN2 = 0
    TIM_SetCompare3(TIM2, 0);  // IN3 = 0
    TIM_SetCompare4(TIM2, 0);  // IN4 = 0
}

// 前进（双PWM模式）
void Motor_Forward(float left_pwm, float right_pwm)
{
    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);
    
    // 右电机前进：IN1=PWM, IN2=0
    TIM_SetCompare1(TIM2, (uint16_t)right_pwm);  // PA0 (IN1)
    TIM_SetCompare2(TIM2, 0);                     // PA1 (IN2)
    
    // 左电机前进：IN3=0, IN4=PWM
    TIM_SetCompare3(TIM2, 0);                     // PA2 (IN3)
    TIM_SetCompare4(TIM2, (uint16_t)left_pwm);   // PA3 (IN4)
}

// 后退（双PWM模式）
void Motor_Back(float left_pwm, float right_pwm)
{
    left_pwm = limit_pwm(left_pwm);
    right_pwm = limit_pwm(right_pwm);
    
    // 右电机后退：IN1=0, IN2=PWM
    TIM_SetCompare1(TIM2, 0);                     // PA0 (IN1)
    TIM_SetCompare2(TIM2, (uint16_t)right_pwm);  // PA1 (IN2)
    
    // 左电机后退：IN3=PWM, IN4=0
    TIM_SetCompare3(TIM2, (uint16_t)left_pwm);   // PA2 (IN3)
    TIM_SetCompare4(TIM2, 0);                     // PA3 (IN4)
}

// 原地左转（左轮后退，右轮前进）
void Motor_TurnLeft90(void)
{
    // 左轮后退
    TIM_SetCompare3(TIM2, (uint16_t)TURN_SPEED);  // IN3 = PWM
    TIM_SetCompare4(TIM2, 0);                     // IN4 = 0
    
    // 右轮前进
    TIM_SetCompare1(TIM2, (uint16_t)TURN_SPEED);  // IN1 = PWM
    TIM_SetCompare2(TIM2, 0);                     // IN2 = 0
    
    Delay_ms(500);  // 调整时间以匹配90度转弯
    Motor_Stop();
}

// 原地右转（左轮前进，右轮后退）
void Motor_TurnRight90(void)
{
    // 左轮前进
    TIM_SetCompare3(TIM2, 0);                     // IN3 = 0
    TIM_SetCompare4(TIM2, (uint16_t)TURN_SPEED);  // IN4 = PWM
    
    // 右轮后退
    TIM_SetCompare1(TIM2, 0);                     // IN1 = 0
    TIM_SetCompare2(TIM2, (uint16_t)TURN_SPEED);  // IN2 = PWM
    
    Delay_ms(500);  // 调整时间以匹配90度转弯
    Motor_Stop();
}

// 右转（左轮前进，右轮停）
void Motor_Right(float left_pwm)
{
    left_pwm = limit_pwm(left_pwm);
    
    // 左轮前进
    TIM_SetCompare3(TIM2, 0);                    // IN3 = 0
    TIM_SetCompare4(TIM2, (uint16_t)left_pwm);  // IN4 = PWM
    
    // 右轮停止
    TIM_SetCompare1(TIM2, 0);                    // IN1 = 0
    TIM_SetCompare2(TIM2, 0);                    // IN2 = 0
}

// 左转（左轮停，右轮前进）
void Motor_Left(float right_pwm)
{
    right_pwm = limit_pwm(right_pwm);
    
    // 左轮停止
    TIM_SetCompare3(TIM2, 0);                     // IN3 = 0
    TIM_SetCompare4(TIM2, 0);                     // IN4 = 0
    
    // 右轮前进
    TIM_SetCompare1(TIM2, (uint16_t)right_pwm);  // IN1 = PWM
    TIM_SetCompare2(TIM2, 0);                    // IN2 = 0
}

// 向前移动指定距离
void Motor_MoveForward(float cm, float left_speed, float right_speed)
{
    Motor_Forward(left_speed, right_speed);
    move_delay(cm, (left_speed + right_speed) / 2.0f);
    Motor_Stop();
}

// 向后移动指定距离
void Motor_MoveBack(float cm)
{
    Motor_Back(BACK_SPEED, BACK_SPEED);
    move_delay(cm, BACK_SPEED);
    Motor_Stop();
}

// 恢复正常直行
void Motor_ResumeNormal(void)
{
    Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
}

// 左电机刹车
void Motor_Left_Brake(void)
{
    TIM_SetCompare3(TIM2, 0);
    TIM_SetCompare4(TIM2, 0);
}

// 右电机刹车
void Motor_Right_Brake(void)
{
    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare2(TIM2, 0);
}