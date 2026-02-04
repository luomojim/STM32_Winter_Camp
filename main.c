#include "stm32f10x.h"
#include "delay.h"
#include "IRSensor.h"
#include "motor.h"
#include "PID.h"
#include "Ultrasound.h"

// #define ULTRASONIC_STOP_TRIGGER 1.0  // 超声停车触发距离(cm) - 根据实际情况调整
// #define ULTRASONIC_DELAY_TRIGGER 5.0 // 超声减速触发距离(cm) - 根据实际情况调整
#define ULTRASONIC_INVALID_LIMIT 2   // 超声无效值计数阈值 - 根据实际情况调整

uint8_t ultrasonic_invalid_count = 0; // 超声传感器无效读数计数器
#define SPEED_STEP 5.0f               // 速度调整步长
#define MAX_SPEED 99.0f               // 最大速度PWM值
#define MIN_SPEED 5.0f                // 最小速度PWM值
float current_speed_pwm = MAX_SPEED;  // 当前速度PWM值，初始为最大速度

// 延迟函数，根据移动距离计算所需时间（1cm对应20ms，实际需根据电机转速调整）
void delay_cm(float cm)
{
    float time_ms = cm * 20; // 转换系数 - 根据实际情况调整
    Delay_ms((uint16_t)time_ms);
}

/**
 * @brief 后退指定距离
 * @param cm 后退距离（厘米）
 * @param target_pwm 目标PWM值（0-99）
 * @note 通过PID算法计算实际PWM值，确保电机转速稳定
 */
void move_back_cm(float cm, float target_pwm)
{
    // 设置电机反转方向电平
    GPIO_ResetBits(MOTOR_PORT, IN1_PIN);
    GPIO_SetBits(MOTOR_PORT, IN2_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN3_PIN);
    GPIO_SetBits(MOTOR_PORT, IN4_PIN);

    // 通过PID计算实际PWM值
    float left_pwm = PID_Calc(&PID_MotorLeft, target_pwm, TIM_GetCapture1(MOTOR_TIM));
    float right_pwm = PID_Calc(&PID_MotorRight, target_pwm, TIM_GetCapture2(MOTOR_TIM));

    // PWM限幅（0-99）
    left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                     : left_pwm;
    right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                        : right_pwm;

    // 设置PWM占空比
    TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
    TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);

    // 根据距离计算延迟时间
    delay_cm(cm);

    // 停止电机
    Motor_Stop();
}

/**
 * @brief 后退指定距离（使用默认PWM值）
 * @param cm 后退距离（厘米）
 * @note 默认PWM值为最大速度，适用于大多数场景
 */
void move_back_cm_default(float cm)
{
    move_back_cm(cm, MAX_SPEED); // 使用默认PWM值为最大速度
}

/**
 * @brief 前进指定距离
 * @param cm 前进距离（厘米）
 * @param target_pwm 目标PWM值（0-99）
 * @note 通过PID算法计算实际PWM值，确保电机转速稳定
 */
void move_forward_cm(float cm, float target_pwm)
{
    // 设置电机正转方向电平
    GPIO_SetBits(MOTOR_PORT, IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN2_PIN);
    GPIO_SetBits(MOTOR_PORT, IN3_PIN);
    GPIO_ResetBits(MOTOR_PORT, IN4_PIN);

    // 通过PID计算实际PWM值
    float left_pwm = PID_Calc(&PID_MotorLeft, target_pwm, TIM_GetCapture1(MOTOR_TIM));
    float right_pwm = PID_Calc(&PID_MotorRight, target_pwm, TIM_GetCapture2(MOTOR_TIM));

    // PWM限幅（0-99）
    left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                     : left_pwm;
    right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                        : right_pwm;

    // 设置PWM占空比
    TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
    TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);

    // 根据距离计算延迟时间
    delay_cm(cm);

    // 停止电机
    Motor_Stop();
}

/**
 * @brief 前进指定距离（使用默认PWM值）
 * @param cm 前进距离（厘米）
 * @note 默认PWM值为最大速度，适用于大多数场景
 */
void move_forward_cm_default(float cm)
{
    move_forward_cm(cm, MAX_SPEED); // 使用默认PWM值为最大速度
}

// 右转（旋转约90度，实际需根据转向时间调整）
void turn_right(void)
{
    Motor_Right();
    Delay_ms(800); // 转向延迟时间 - 根据实际情况调整
    Motor_Stop();
}

// 左转（旋转约90度，实际需根据转向时间调整）
void turn_left(void)
{
    Motor_Left();
    Delay_ms(800); // 转向延迟时间 - 根据实际情况调整
    Motor_Stop();
}

int main(void)
{
    // 初始化所有模块
    Motor_Init();
    IRSensor_Init();
    Ultrasonic_Init();
    PID_Init();

    while (1)
    {
        // 1. 读取所有红外传感器状态
        uint8_t red1 = IRSensor_Detect(IR_PORT, RED1_PIN);
        uint8_t red2 = IRSensor_Detect(IR_PORT, RED2_PIN);
        uint8_t red3 = IRSensor_Detect(IR_PORT, RED3_PIN);
        uint8_t red4 = IRSensor_Detect(IR_PORT, RED4_PIN);
        uint8_t red5 = IRSensor_Detect(IR_PORT, RED5_PIN);
        uint8_t red6 = IRSensor_Detect(IR_PORT, RED6_PIN);

        // 2. 读取超声传感器并判断减速/停车状态
        float distance = Test_Distance();
        uint8_t ultrasonic_trigger = 0; // 减速标志（距离5~1cm）
        uint8_t ultrasonic_stop = 0;    // 停车标志（距离<=1cm或无效值）

        if (distance > 0) // 超声传感器有效读数
        {
            ultrasonic_invalid_count = 0;
            if (distance <= ULTRASONIC_STOP_TRIGGER)
            {
                ultrasonic_stop = 1;
            }
            else if (distance > ULTRASONIC_STOP_TRIGGER && distance <= ULTRASONIC_DELAY_TRIGGER)
            {
                ultrasonic_trigger = 1; // 进入减速区域（5~1cm）
            }
        }
        else // 超声传感器无效读数
        {
            ultrasonic_invalid_count++;
            if (ultrasonic_invalid_count >= ULTRASONIC_INVALID_LIMIT)
            {
                ultrasonic_stop = 1;
            }
        }

        // 3. 执行主逻辑，按优先级从高到低处理（if-else if确保唯一执行路径）
        // 优先级7：停车处理（最高优先级）
        if (ultrasonic_stop)
        {
            Motor_Stop();
        }
        // 优先级1：减速+RED1+RED2同时检测到障碍物+其他未检测到
        else if (ultrasonic_trigger && (red1 == IR_HAVE_OBSTACLE) && (red2 == IR_HAVE_OBSTACLE) && (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) && (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            Motor_Stop();
            Delay_ms(200);
            move_back_cm_default(3);
            turn_right();
            move_forward_cm_default(10);
            turn_right();
            current_speed_pwm = MAX_SPEED; // 恢复默认速度
            Motor_Forward();               // 恢复直线行驶
        }
        // 优先级2：减速+RED1检测到障碍物+其他未检测到
        else if (ultrasonic_trigger && (red1 == IR_HAVE_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) && (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            move_back_cm_default(3);
            turn_right();
            move_forward_cm_default(14);
            turn_left();
            move_forward_cm_default(14);
            turn_left();
            move_forward_cm_default(14);
            turn_right();
            current_speed_pwm = MAX_SPEED; // 恢复默认速度
            Motor_Forward();               // 恢复直线行驶
        }
        // 优先级3：减速+RED2检测到障碍物+其他未检测到
        else if (ultrasonic_trigger && (red1 == IR_NO_OBSTACLE) && (red2 == IR_HAVE_OBSTACLE) && (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) && (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            Motor_Stop();
            Delay_ms(200);
            move_back_cm_default(3);
            turn_left();
            move_forward_cm_default(14);
            turn_right();
            move_forward_cm_default(14);
            turn_right();
            move_forward_cm_default(14);
            turn_left();
            current_speed_pwm = MAX_SPEED; // 恢复默认速度
            Motor_Forward();               // 恢复直线行驶
        }
        // 优先级4：RED1/RED2未检测到+其他区域检测到障碍物（RED3/5至少一个）
        else if ((red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red3 || red5))
        {
            // 平滑加速：逐渐恢复速度至最大速度
            if (current_speed_pwm < MAX_SPEED)
            {
                current_speed_pwm += SPEED_STEP;
                if (current_speed_pwm > MAX_SPEED)
                {
                    current_speed_pwm = MAX_SPEED;
                }
            }

            float left_pwm, right_pwm;
            if (red3 == IR_HAVE_OBSTACLE && red5 == IR_HAVE_OBSTACLE)
            {
                // RED3+RED5同时检测到障碍物，轻微右转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));
            }
            else if (red3 == IR_HAVE_OBSTACLE)
            {
                // RED3检测到障碍物，右转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCapture2(MOTOR_TIM));
            }
            else if (red5 == IR_HAVE_OBSTACLE)
            {
                // RED5检测到障碍物，左转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));
            }
            else
            {
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));
            }

            // PWM限幅
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
            TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
            Motor_Forward();
        }
        // 优先级5：RED1/RED2未检测到+其他区域检测到障碍物（RED4/6至少一个）
        else if ((red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red4 || red6))
        {
            // 平滑加速：逐渐恢复速度至最大速度
            if (current_speed_pwm < MAX_SPEED)
            {
                current_speed_pwm += SPEED_STEP;
                if (current_speed_pwm > MAX_SPEED)
                {
                    current_speed_pwm = MAX_SPEED;
                }
            }

            float left_pwm, right_pwm;
            if (red4 == IR_HAVE_OBSTACLE && red6 == IR_HAVE_OBSTACLE)
            {
                // RED4+RED6同时检测到障碍物，轻微左转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCapture2(MOTOR_TIM));
            }
            else if (red4 == IR_HAVE_OBSTACLE)
            {
                // RED4检测到障碍物，左转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));
            }
            else if (red6 == IR_HAVE_OBSTACLE)
            {
                // RED6检测到障碍物，右转调整
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCapture2(MOTOR_TIM));
            }
            else
            {
                left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
                right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));
            }

            // PWM限幅
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
            TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
            Motor_Forward();
        }
        // 优先级6：减速+无任何障碍物
        else if (ultrasonic_trigger && (red1 == IR_NO_OBSTACLE) && (red2 == IR_NO_OBSTACLE) && (red3 == IR_NO_OBSTACLE) && (red4 == IR_NO_OBSTACLE) && (red5 == IR_NO_OBSTACLE) && (red6 == IR_NO_OBSTACLE))
        {
            // 平滑减速：逐渐降低速度至最小速度
            if (current_speed_pwm > MIN_SPEED)
            {
                current_speed_pwm -= SPEED_STEP;
                if (current_speed_pwm < MIN_SPEED)
                {
                    current_speed_pwm = MIN_SPEED;
                }
            }

            // 使用当前速度PWM值
            float left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
            float right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));

            // PWM限幅
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
            TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
            Motor_Forward();
        }
        // 优先级8：无任何障碍物，正常直线行驶（最低优先级）
        else
        {
            // 平滑加速：逐渐恢复速度至最大速度
            if (current_speed_pwm < MAX_SPEED)
            {
                current_speed_pwm += SPEED_STEP;
                if (current_speed_pwm > MAX_SPEED)
                {
                    current_speed_pwm = MAX_SPEED;
                }
            }

            // 使用当前速度PWM值
            float left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCapture1(MOTOR_TIM));
            float right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCapture2(MOTOR_TIM));

            // PWM限幅
            left_pwm = (left_pwm > 99) ? 99 : (left_pwm < 0) ? 0
                                                             : left_pwm;
            right_pwm = (right_pwm > 99) ? 99 : (right_pwm < 0) ? 0
                                                                : right_pwm;

            TIM_SetCompare1(MOTOR_TIM, (uint16_t)left_pwm);
            TIM_SetCompare2(MOTOR_TIM, (uint16_t)right_pwm);
            Motor_Forward();
        }
    }
}
