/**
 * @file main.c
 * @brief 智能小车避障系统主程序
 * @details 实现基于超声波和红外传感器的多场景避障逻辑
 *          包含8种避障场景，按优先级从高到低处理
 * @author STM32 Winter Camp
 * @date 2026-02-06
 */

#include "stm32f10x.h"
#include "delay.h"
#include "IRSensor.h"
#include "motor.h"
#include "Ultrasound.h"
#include "pwm.h"

/**
 * @brief 超声传感器配置参数
 */
#define ULTRASONIC_INVALID_LIMIT 2 // 超声无效值计数阈值（连续无效次数）

/**
 * @brief 速度控制参数
 */
#define SPEED_STEP 5.0f // 速度调整步长
#define MIN_SPEED 5.0f  // 最小速度PWM值

/**
 * @brief 全局变量定义
 */
uint8_t ultrasonic_invalid_count = 0; // 超声传感器无效读数计数器

/**
 * @brief 延迟函数，根据移动距离计算所需时间
 * @param cm 移动距离（厘米）
 * @note 1cm对应20ms，实际需根据电机转速调整
 */
void delay_cm(float cm)
{
    float time_ms = cm * 20; // 转换系数 - 根据实际情况调整
    Delay_ms((uint16_t)time_ms);
}

/**
 * @brief 后退指定距离
 * @param cm 后退距离（厘米）
 * @note 通过PID算法计算实际PWM值，确保电机转速稳定
 */
void move_back_cm(float cm)
{
    // 设置电机反转方向电平（左电机正转，右电机反转）
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_SetBits(MOTOR_PORT, IN4_PIN);

    // 直接使用目标PWM值（TT1=80, TT2=70）
    float left_pwm = 80.0f;
    float right_pwm = 70.0f;

    // PWM限幅（0-99）
    left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                     : left_pwm;
    right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                        : right_pwm;

    // 设置PWM占空比（使用软件PWM）
    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);

    // 根据距离计算延迟时间
    delay_cm(cm);

    // 停止电机
    Motor_Stop();
}

/**
 * @brief 前进指定距离
 * @param cm 前进距离（厘米）
 * @note 通过PID算法计算实际PWM值，确保电机转速稳定
 */
void move_forward_cm(float cm)
{
    // 设置电机正转方向电平（左电机反转，右电机正转）
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_SetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    // 直接使用目标PWM值（TT1=80, TT2=70）
    float left_pwm = 80.0f;
    float right_pwm = 70.0f;

    // PWM限幅（0-99）
    left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                     : left_pwm;
    right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                        : right_pwm;

    // 设置PWM占空比（使用软件PWM）
    PWM_SetCompare1((uint16_t)left_pwm);
    PWM_SetCompare2((uint16_t)right_pwm);

    // 根据距离计算延迟时间
    delay_cm(cm);

    // 停止电机
    Motor_Stop();
}

/**
 * @brief 右转（旋转约90度）
 * @note 转向时间可根据实际情况调整
 */
void turn_right(void)
{
    Motor_Right(50.0f);
    Delay_ms(800); // 转向延迟时间 - 根据实际情况调整
    Motor_Stop();
}

/**
 * @brief 左转（旋转约90度）
 * @note 转向时间可根据实际情况调整
 */
void turn_left(void)
{
    Motor_Left(50.0f);
    Delay_ms(800); // 转向延迟时间 - 根据实际情况调整
    Motor_Stop();
}

/**
 * @brief 主函数
 * @return 无
 * @details 实现智能小车的避障逻辑，按优先级处理不同的障碍物场景
 */
int main(void)
{
    // 初始化所有模块
    Motor_Init();      // 电机驱动初始化
    IRSensor_Init();   // 红外传感器初始化
    Ultrasonic_Init(); // 超声波传感器初始化
    PWM_Init();        // PWM初始化（软件PWM）

    while (1)
    {
        // 1. 读取所有红外传感器状态
        uint8_t red1 = IRSensor_Detect(IR_PORT, RED1_PIN); // 左前红外
        uint8_t red2 = IRSensor_Detect(IR_PORT, RED2_PIN); // 右前红外
        uint8_t red3 = IRSensor_Detect(IR_PORT, RED3_PIN); // 左侧红外1
        uint8_t red4 = IRSensor_Detect(IR_PORT, RED4_PIN); // 右侧红外1
        uint8_t red5 = IRSensor_Detect(IR_PORT, RED5_PIN); // 左侧红外2
        uint8_t red6 = IRSensor_Detect(IR_PORT, RED6_PIN); // 右侧红外2

        // 2. 读取超声传感器并判断减速/停车状态
        float distance = Test_Distance();
        uint8_t ultrasonic_trigger = 0; // 减速标志（距离5~1cm）
        uint8_t ultrasonic_stop = 0;    // 停车标志（距离<=1cm或无效值）

        if (distance > 0) // 超声传感器有效读数
        {
            ultrasonic_invalid_count = 0;
            if (distance <= ULTRASONIC_STOP_TRIGGER)
            {
                ultrasonic_stop = 1; // 距离过近，触发停车
            }
            else if (distance > ULTRASONIC_STOP_TRIGGER && distance <= ULTRASONIC_DELAY_TRIGGER)
            {
                ultrasonic_trigger = 1; // 进入减速区域（5~1cm）
            }
        }
        else // 超声传感器无效读数
        {
            ultrasonic_stop = 1; // 无效读数，触发停车
        }

        // 3. 执行主逻辑，按优先级从高到低处理（if-else if确保唯一执行路径）

        /**
         * 优先级1：超声停车触发（最高优先级）
         * 触发条件：距离≤1cm或无返回值
         * 处理逻辑：立即停车
         */
        if (ultrasonic_stop)
        {
            Motor_Stop();
        }

        /**
         * 优先级2：超声减速触发（距障碍5~1cm）+ RED1+RED2同触 + RED3/4/5/6均未触
         * 处理逻辑：停车→延时200ms→倒车3cm→右转→直行10cm→右转→恢复直行
         */
        else if (ultrasonic_trigger && (red1 == IR_HAVE_OBSTACLE) && (red2 == IR_HAVE_OBSTACLE) &&
                 (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) &&
                 (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            Motor_Stop();
            Delay_ms(200);
            move_back_cm(3);
            turn_right();
            move_forward_cm(10);
            turn_right();
            Motor_Forward(80.0f, 70.0f); // 恢复直线行驶
        }

        /**
         * 优先级3：超声减速触发（距障碍5~1cm）+ RED1单触 + RED2/3/4/5/6均未触
         * 处理逻辑：倒车3cm→右转→直行14cm→左转→直行14cm→左转→直行14cm→右转→恢复直行
         */
        else if (ultrasonic_trigger && (red1 == IR_HAVE_OBSTACLE) && (red2 == IR_NO_OBSTACLE) &&
                 (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) &&
                 (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            move_back_cm(3);
            turn_right();
            move_forward_cm(14);
            turn_left();
            move_forward_cm(14);
            turn_left();
            move_forward_cm(14);
            turn_right();
            Motor_Forward(80.0f, 70.0f); // 恢复直线行驶
        }

        /**
         * 优先级4：超声减速触发（距障碍5~1cm）+ RED2单触 + RED1/3/4/5/6均未触
         * 处理逻辑：停车→延时200ms→倒车3cm→左转→直行14cm→右转→直行14cm→右转→直行14cm→左转→恢复直行
         */
        else if (ultrasonic_trigger && (red1 == IR_NO_OBSTACLE) && (red2 == IR_HAVE_OBSTACLE) &&
                 (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) &&
                 (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            Motor_Stop();
            Delay_ms(200);
            move_back_cm(3);
            turn_left();
            move_forward_cm(14);
            turn_right();
            move_forward_cm(14);
            turn_right();
            move_forward_cm(14);
            turn_left();
            Motor_Forward(80.0f, 70.0f); // 恢复直线行驶
        }

        /**
         * 优先级5：RED1/RED2均未触 + 左侧红外触发（RED3/5任一触）
         * 处理逻辑：分场景调速避障后回直行
         */
        else if ((red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red3 || red5))
        {
            float left_pwm, right_pwm;
            if (red3 == IR_HAVE_OBSTACLE && red5 == IR_HAVE_OBSTACLE)
            {
                // RED3+RED5双触：左轮加速、右轮正常（远离左墙）
                left_pwm = 90.0f;
                right_pwm = 70.0f;
            }
            else if (red3 == IR_HAVE_OBSTACLE)
            {
                // RED3单触：右轮加速、左轮正常（微调回正）
                left_pwm = 80.0f;
                right_pwm = 80.0f;
            }
            else if (red5 == IR_HAVE_OBSTACLE)
            {
                // RED5单触：左轮加速、右轮正常（远离左墙）
                left_pwm = 90.0f;
                right_pwm = 70.0f;
            }
            else
            {
                // 默认情况：恢复正常直行
                Motor_Forward(80.0f, 70.0f);
                continue;
            }

            // PWM限幅（0-99）
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            // 使用传入的PWM值前进
            Motor_Forward(left_pwm, right_pwm);
        }

        /**
         * 优先级6：RED1/RED2均未触 + 右侧红外触发（RED4/6任一触）
         * 处理逻辑：分场景调速避障后回直行
         */
        else if ((red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red4 || red6))
        {
            float left_pwm, right_pwm;
            if (red4 == IR_HAVE_OBSTACLE && red6 == IR_HAVE_OBSTACLE)
            {
                // RED4+RED6双触：右轮加速、左轮正常（远离右墙）
                left_pwm = 80.0f;
                right_pwm = 90.0f;
            }
            else if (red4 == IR_HAVE_OBSTACLE)
            {
                // RED4单触：左轮加速、右轮正常（微调回正）
                left_pwm = 80.0f;
                right_pwm = 80.0f;
            }
            else if (red6 == IR_HAVE_OBSTACLE)
            {
                // RED6单触：右轮加速、左轮正常（远离右墙）
                left_pwm = 80.0f;
                right_pwm = 90.0f;
            }
            else
            {
                // 默认情况：恢复正常直行
                Motor_Forward(80.0f, 70.0f);
                continue;
            }

            // PWM限幅（0-99）
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            // 使用传入的PWM值前进
            Motor_Forward(left_pwm, right_pwm);
        }

        /**
         * 优先级7：超声减速触发（距障碍5~1cm）+ 无任何红外触发
         * 处理逻辑：低速直行（仅减速不停车）
         */
        else if (ultrasonic_trigger && (red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) &&
                 (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) &&
                 (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            // 低速直行（TT1=40, TT2=30）
            float left_pwm = 40.0f;
            float right_pwm = 30.0f;

            // PWM限幅（0-99）
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            // 使用传入的PWM值前进
            Motor_Forward(left_pwm, right_pwm);
        }

        /**
         * 优先级8：无任何触发条件（最低优先级）
         * 处理逻辑：正常直行
         */
        else
        {
            Motor_Forward(80.0f, 70.0f); // 正常直行（TT1=80, TT2=70）
        }

        // 运行PWM任务，生成软件PWM波形
        PWM_Task();
    }
}
