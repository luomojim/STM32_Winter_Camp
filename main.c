/**
 * @file main.c
 * @brief 智能小车避障程序（最终适配版）
 * @details 完全匹配你的硬件引脚和8级优先级避障逻辑
 */
// 必须先包含标准库头文件（解决函数未声明）
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
// 包含自定义头文件（确保宏定义/函数声明生效）
#include "motor.h"
#include "pwm.h"
#include "IRSensor.h"
#include "delay.h"
#include "Ultrasound.h"

/************************ 核心配置 ************************/
// 速度配置（0=刹车，1~99=转速）
#define NORMAL_LEFT_SPEED 80.0f  // 左电机正常速度
#define NORMAL_RIGHT_SPEED 80.0f // 右电机正常速度
#define TURN_SPEED 50.0f         // 转向速度
#define BACK_SPEED 50.0f         // 倒车速度
#define SLOW_DOWN_RATIO 0.5f     // 超声减速比例

// 超声阈值
#define ULTRASONIC_STOP_DIST 1.0f // ≤1cm 停车
#define ULTRASONIC_SLOW_DIST 5.0f // 5~1cm 减速
#define ULTRASONIC_INVALID_CNT 2  // 超声无效计数阈值

/************************ 全局变量 ************************/
uint8_t ultrasonic_invalid_count = 0; // 超声无效计数

/************************ 工具函数 ************************/
/**
 * @brief PWM限幅（0~99）
 */
float limit_pwm(float pwm)
{
    return (pwm > 99) ? 99 : (pwm < 0) ? 0
                                       : pwm;
}

/**
 * @brief 延时移动（适配速度的距离延时）
 */
void move_delay(float cm, float speed)
{
    float time_ms = (cm * 60) / (speed / 10); // 适配系数，可微调
    if (speed <= 0)
        time_ms = 0;
    Delay_ms((uint16_t)time_ms);
}

/************************ 动作函数 ************************/
/**
 * @brief 倒车指定距离
 */
void move_back(float cm)
{
    // DRV8833倒车逻辑：IN1=0, IN2=1；IN3=0, IN4=1
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_SetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_SetBits(MOTOR_PORT, IN4_PIN);

    PWM_SetCompare1((uint16_t)limit_pwm(BACK_SPEED));
    PWM_SetCompare2((uint16_t)limit_pwm(BACK_SPEED));
    move_delay(cm, BACK_SPEED);

    Motor_Stop(); // 倒车后停车
}

/**
 * @brief 直行指定距离
 */
void move_forward(float cm, float left_speed, float right_speed)
{
    // DRV8833直行逻辑：IN1=1, IN2=0；IN3=1, IN4=0
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    PWM_SetCompare1((uint16_t)limit_pwm(left_speed));
    PWM_SetCompare2((uint16_t)limit_pwm(right_speed));
    move_delay(cm, (left_speed + right_speed) / 2);

    Motor_Stop(); // 直行后停车
}

/**
 * @brief 右转90度
 */
void turn_right_90(void)
{
    // 右转：左电机转，右电机刹车
    Motor_Right_Brake();
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    PWM_SetCompare1((uint16_t)limit_pwm(TURN_SPEED));
    Delay_ms(800); // 转向延时，可微调

    Motor_Stop();
}

/**
 * @brief 左转90度
 */
void turn_left_90(void)
{
    // 左转：右电机转，左电机刹车
    Motor_Left_Brake();
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);
    PWM_SetCompare2((uint16_t)limit_pwm(TURN_SPEED));
    Delay_ms(800); // 转向延时，可微调

    Motor_Stop();
}

/**
 * @brief 恢复正常直行
 */
void resume_normal(void)
{
    // 恢复DRV8833正转电平+正常速度
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    PWM_SetCompare1((uint16_t)limit_pwm(NORMAL_LEFT_SPEED));
    PWM_SetCompare2((uint16_t)limit_pwm(NORMAL_RIGHT_SPEED));
}

/************************ 主函数（核心入口） ************************/
int main(void)
{
    /************************ 1. 系统/外设初始化 ************************/
    // 延时函数无需初始化，直接调用Delay_ms/Delay_us即可

    // 1.2 初始化TIM2（用于超声测距计时，1us精度）
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 使能TIM2时钟
    TIM_TimeBaseStruct.TIM_Period = 0xFFFF;              // 最大计数范围
    TIM_TimeBaseStruct.TIM_Prescaler = 71;               // 72MHz/72=1MHz（1us计数）
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
    TIM_Cmd(TIM2, ENABLE); // 启动TIM2

    // 1.3 初始化电机+PWM（motor.c中已初始化GPIOA/GPIOB和PWM）
    Motor_Init();

    /************************ 2. 红外/超声引脚初始化 ************************/
    GPIO_InitTypeDef GPIO_InitStruct;
    // 2.1 使能GPIO时钟（AFIO必须使能，避免引脚复用异常）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 2.2 红外引脚初始化（PA4/5/8/9/11/12 上拉输入）
    // 第一组：PA4/5/8/9
    GPIO_InitStruct.GPIO_Pin = RED1_PIN | RED2_PIN | RED3_PIN | RED4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IR_PORT, &GPIO_InitStruct);
    // 第二组：PA11/12
    GPIO_InitStruct.GPIO_Pin = RED5_PIN | RED6_PIN;
    GPIO_Init(IR_PORT, &GPIO_InitStruct);

    // 2.3 超声TRIG引脚（PB0）：推挽输出，初始低电平
    GPIO_InitStruct.GPIO_Pin = TRIG_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStruct);
    GPIO_ResetBits(ULTRASONIC_PORT, TRIG_PIN); // 初始低电平

    // 2.4 超声ECHO引脚（PB1）：上拉输入
    GPIO_InitStruct.GPIO_Pin = ECHO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStruct);

    /************************ 3. 主循环（避障逻辑） ************************/
    while (1)
    {
        // 3.1 读取传感器状态
        // 红外状态（1=有障碍，0=无障碍）
        uint8_t red1 = IRSensor_Detect(IR_PORT, RED1_PIN);
        uint8_t red2 = IRSensor_Detect(IR_PORT, RED2_PIN);
        uint8_t red3 = IRSensor_Detect(IR_PORT, RED3_PIN);
        uint8_t red4 = IRSensor_Detect(IR_PORT, RED4_PIN);
        uint8_t red5 = IRSensor_Detect(IR_PORT, RED5_PIN);
        uint8_t red6 = IRSensor_Detect(IR_PORT, RED6_PIN);

        // 超声状态
        float distance = Test_Distance();
        uint8_t ultra_stop = 0; // 超声停车标志
        uint8_t ultra_slow = 0; // 超声减速标志

        // 超声状态判断
        if (distance == -1.0f) // 超声无返回值（无效）
        {
            ultrasonic_invalid_count++;
            if (ultrasonic_invalid_count >= ULTRASONIC_INVALID_CNT)
            {
                ultra_stop = 1; // 连续无效，触发停车
            }
        }
        else // 超声有有效值
        {
            ultrasonic_invalid_count = 0;
            if (distance <= ULTRASONIC_STOP_DIST)
            {
                ultra_stop = 1; // ≤1cm，停车
            }
            else if (distance > ULTRASONIC_STOP_DIST && distance <= ULTRASONIC_SLOW_DIST)
            {
                ultra_slow = 1; // 5~1cm，减速
            }
        }

        // 3.2 优先级避障逻辑（互斥触发，从高到低）
        // 优先级1（最高）：超声停车触发（距障碍≤1cm 或 超声无返回值）
        if (ultra_stop)
        {
            Motor_Stop();
        }
        // 优先级2：超声减速触发（5~1cm）+ RED1+RED2同触 + RED3/4/5/6均未触
        else if (ultra_slow && red1 && red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_Stop();
            Delay_ms(200);
            move_back(3);
            turn_right_90();
            move_forward(10, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_right_90();
            resume_normal();
        }
        // 优先级3：超声减速触发（5~1cm）+ RED1单触 + RED2/3/4/5/6均未触
        else if (ultra_slow && red1 && !red2 && !red3 && !red4 && !red5 && !red6)
        {
            move_back(3);
            turn_right_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_left_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_left_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_right_90();
            resume_normal();
        }
        // 优先级4：超声减速触发（5~1cm）+ RED2单触 + RED1/3/4/5/6均未触
        else if (ultra_slow && !red1 && red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_Stop();
            Delay_ms(200);
            move_back(3);
            turn_left_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_right_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_right_90();
            move_forward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            turn_left_90();
            resume_normal();
        }
        // 优先级5：RED1/RED2均未触 + 左侧红外触发（RED3/5任一触）
        else if (!red1 && !red2 && (red3 || red5))
        {
            float left_speed = NORMAL_LEFT_SPEED;
            float right_speed = NORMAL_RIGHT_SPEED;

            if (red3 && red5)
                left_speed += 10; // RED3+5双触：左轮加速、右轮正常（远离左墙）
            else if (red3)
                right_speed += 10; // RED3单触：右轮加速、左轮正常（微调回正）
            else if (red5)
                left_speed += 10; // RED5单触：左轮加速、右轮正常（远离左墙）

            Motor_Forward(limit_pwm(left_speed), limit_pwm(right_speed));
        }
        // 优先级6：RED1/RED2均未触 + 右侧红外触发（RED4/6任一触）
        else if (!red1 && !red2 && (red4 || red6))
        {
            float left_speed = NORMAL_LEFT_SPEED;
            float right_speed = NORMAL_RIGHT_SPEED;

            if (red4 && red6)
                right_speed += 10; // RED4+6双触：右轮加速、左轮正常（远离右墙）
            else if (red4)
                left_speed += 10; // RED4单触：左轮加速、右轮正常（微调回正）
            else if (red6)
                right_speed += 10; // RED6单触：右轮加速、左轮正常（远离右墙）

            Motor_Forward(limit_pwm(left_speed), limit_pwm(right_speed));
        }
        // 优先级7：超声减速触发（5~1cm）+ 无任何红外触发
        else if (ultra_slow && !red1 && !red2 && !red3 && !red4 && !red5 && !red6)
        {
            // 双电机减速50%，低速直行（仅减速不停车）
            Motor_Forward(limit_pwm(NORMAL_LEFT_SPEED * SLOW_DOWN_RATIO),
                          limit_pwm(NORMAL_RIGHT_SPEED * SLOW_DOWN_RATIO));
        }
        // 优先级8（最低）：无任何触发条件
        else
        {
            resume_normal(); // 正常直行
        }

        PWM_Task();   // 必须调用，生成软件PWM波形
        Delay_us(10); // 消抖，避免频繁触发
    }
}
