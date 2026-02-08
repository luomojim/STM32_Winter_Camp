#include "stm32f10x.h"
#include "delay.h"
#include "motor.h"
#include "IRSensor.h"
#include "Ultrasound.h"

// ================= 宏定义参数 =================
#define STOP_DISTANCE 15.0f   // 超声波停车距离(cm)
#define WALL_ADJUST_PWM 15.0f // 巡墙纠偏时增加的PWM值

// ================= 状态变量 =================
uint8_t r1, r2, r5, r6;
float distance;

// ================= 函数声明 =================
void System_Init_All(void);
void Logic_Avoidance(void); // 避障逻辑 (场景1-5)
void Logic_Cruising(void);  // 巡航逻辑 (场景6-7)

// 动作序列封装
void Action_Case1_2(void); // 左转掉头序列
void Action_Case3(void);   // 右转掉头序列
void Action_Case4(void);   // 复杂左避障序列
void Action_Case5(void);   // 复杂右避障序列

int main(void)
{
    // 1. 系统初始化
    System_Init_All();

    // 启动延时，防止上电瞬间乱跑
    Delay_ms(1000);

    while (1)
    {
        // 2. 读取所有传感器
        // 红外: 0(IR_HAVE_OBSTACLE)表示有障碍, 1(IR_NO_OBSTACLE)表示无障碍
        r1 = IRSensor_Detect(IR_PORT, RED1_PIN); // 左前
        r2 = IRSensor_Detect(IR_PORT, RED2_PIN); // 右前
        r5 = IRSensor_Detect(IR_PORT, RED5_PIN); // 左侧
        r6 = IRSensor_Detect(IR_PORT, RED6_PIN); // 右侧

        distance = Test_Distance();

        // 3. 判断是否需要停车 (超声波触发 或 任意前方红外触发)
        // 注意：有些情况红外没撞到但超声波很近，也视为触发停车
        uint8_t stop_triggered = 0;

        if (distance > 0.1f && distance <= STOP_DISTANCE)
        {
            stop_triggered = 1;
        }
        // 为了安全，如果前方物理碰撞，强制触发停车逻辑
        if (r1 == IR_HAVE_OBSTACLE || r2 == IR_HAVE_OBSTACLE)
        {
            stop_triggered = 1;
        }

        if (stop_triggered)
        {
            Logic_Avoidance(); // 进入避障模式
        }
        else
        {
            Logic_Cruising(); // 进入巡航模式
        }

        Delay_ms(10); // 循环小延时
    }
}

/**
 * @brief 避障逻辑 (处理场景 1-5)
 */
void Logic_Avoidance(void)
{
    // 停车 1s
    Motor_Stop();
    Delay_ms(1000);

    // 重新读取传感器确认状态 (可选，这里使用循环开始时读取的值)
    // 逻辑判定：

    // === 场景 1 & 2: RED1+RED2同触 ===
    // 只要 R1和R2同时触发，且不是场景3 (R5触+R6未触)，都归为左转逻辑
    // 细分：
    // Case 1: R1+R2触, R5/R6未触
    // Case 2: R1+R2触, R5未触, R6触
    // 共同点: R1=0, R2=0, R5=1 (无障碍)
    if (r1 == IR_HAVE_OBSTACLE && r2 == IR_HAVE_OBSTACLE && r5 == IR_NO_OBSTACLE)
    {
        Action_Case1_2();
    }
    // === 场景 3: RED1+RED2同触 + R6未触 + R5触发 ===
    else if (r1 == IR_HAVE_OBSTACLE && r2 == IR_HAVE_OBSTACLE && r5 == IR_HAVE_OBSTACLE && r6 == IR_NO_OBSTACLE)
    {
        Action_Case3();
    }
    // === 场景 4: RED1单触 + 其他均未触 ===
    else if (r1 == IR_HAVE_OBSTACLE && r2 == IR_NO_OBSTACLE && r5 == IR_NO_OBSTACLE && r6 == IR_NO_OBSTACLE)
    {
        Action_Case4();
    }
    // === 场景 5: RED2单触 + 其他均未触 ===
    else if (r1 == IR_NO_OBSTACLE && r2 == IR_HAVE_OBSTACLE && r5 == IR_NO_OBSTACLE && r6 == IR_NO_OBSTACLE)
    {
        Action_Case5();
    }
    // === 兜底逻辑 ===
    // 如果超声波触发停车(例如正前方有墙)，但红外没撞到(例如R1/R2中间空隙)，
    // 为了防止死循环停车，执行默认避障(这里选Case1左转)
    else
    {
        Action_Case1_2();
    }
}

/**
 * @brief 巡航逻辑 (处理场景 6-7)
 */
void Logic_Cruising(void)
{
    // === 场景 6: RED1/RED2 均未触 ===
    // (已在外部if中通过stop_triggered过滤，这里只需处理巡墙)

    // (1) 若RED5单触(左侧有墙)：左轮加速、右轮正常（远离左墙 -> 向右偏）
    if (r5 == IR_HAVE_OBSTACLE && r6 == IR_NO_OBSTACLE)
    {
        // 限制最大PWM
        float left_speed = NORMAL_LEFT_SPEED + WALL_ADJUST_PWM;
        if (left_speed > 99)
            left_speed = 99;

        Motor_Forward(left_speed, NORMAL_RIGHT_SPEED);
    }
    // (2) 若RED6单触(右侧有墙)：右轮加速、左轮正常（远离右墙 -> 向左偏）
    else if (r5 == IR_NO_OBSTACLE && r6 == IR_HAVE_OBSTACLE)
    {
        float right_speed = NORMAL_RIGHT_SPEED + WALL_ADJUST_PWM;
        if (right_speed > 99)
            right_speed = 99;

        Motor_Forward(NORMAL_LEFT_SPEED, right_speed);
    }
    // === 场景 7: 无任何触发条件 ===
    else
    {
        Motor_ResumeNormal(); // 正常直行
    }
}

// ================= 动作序列函数 =================

// 场景1/2: 延时200ms→左转90→直行10cm→左转90→正常
void Action_Case1_2(void)
{
    Delay_ms(200);
    Motor_TurnLeft90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
    Motor_TurnLeft90();
    // 退出后回到主循环，执行Normal
}

// 场景3: 延时200ms→右转90→直行10cm→右转90→正常
void Action_Case3(void)
{
    Delay_ms(200);
    Motor_TurnRight90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
    Motor_TurnRight90();
}

// 场景4: 倒车5cm→右转90→直行10→左转90→直行10→左转90→直行10→右转90
void Action_Case4(void)
{
    Motor_MoveBack(5.0f);

    Motor_TurnRight90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnLeft90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnLeft90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnRight90();
}

// 场景5: 倒车5cm→左转90→直行10→右转90→直行10→右转90→直行10→左转90
void Action_Case5(void)
{
    Motor_MoveBack(5.0f);

    Motor_TurnLeft90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnRight90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnRight90();
    Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

    Motor_TurnLeft90();
}

void System_Init_All(void)
{
    SystemInit();
    delay_init();      // 延时初始化
    Motor_Init();      // 电机初始化 (包含TIM2和GPIO)
    IRSensor_Init();   // 红外初始化 (包含GPIO)
    Ultrasound_Init(); // 超声波初始化 (包含TIM1和GPIO)
}

// #include "stm32f10x.h"
// #include "stm32f10x_tim.h"
// #include "stm32f10x_gpio.h"
// #include "stm32f10x_rcc.h"
// #include "motor.h"
// #include "IRSensor.h"
// #include "delay.h"
// #include "Ultrasound.h"

// // ???????(5cm)
// #define ULTRASONIC_STOP_DIST 5.0f
// // ???????
// #define ULTRASONIC_INVALID_CNT 3
// // ????????
// #define WALL_ADJUST_SPEED 20.0f

// // ????????
// uint8_t ultrasonic_invalid_count = 0;

// // ???????
// uint8_t red1_state, red2_state, red5_state, red6_state;

// // ????
// void System_Init(void);
// void GPIO_Init_All(void);
// uint8_t Check_Ultrasonic_Stop(void);
// void Read_IR_Sensors(void);
// void Execute_Case1_3(void);
// void Execute_Case4(void);
// void Execute_Case5(void);
// void Adjust_Wall_Distance(void);

// int main(void)
// {
//     // ?????
//     System_Init();

//     // ?????(??SysTick)
//     delay_init();

//     // ???GPIO
// GPIO_Init_All();

//     // ????????
//     IRSensor_Init();

//     // ?????(TIM2)
//     Motor_Init();

//     // ??????(TIM1)
//     Ultrasound_Init();

//     // ??1?,?????
//     Delay_ms(1000);

//     // ????:??
//     Motor_Stop();

//     // ???
//     while (1)
//     {
//         // ???????????
//         Read_IR_Sensors();

//         // ???????????
//         uint8_t ultra_stop = Check_Ultrasonic_Stop();

//         // ==================== ??1-5:??????? ====================
//         if (ultra_stop)
//         {
//             // ??1?
//             Motor_Stop();
//             Delay_ms(1000);

//             // ??1: RED1+RED2?? + RED5/6???
//             if (red1_state == IR_HAVE_OBSTACLE && red2_state == IR_HAVE_OBSTACLE &&
//                 red5_state == IR_NO_OBSTACLE && red6_state == IR_NO_OBSTACLE)
//             {
//                 Execute_Case1_3(); // ????
//             }
//             // ??2: RED1+RED2?? + RED5??+RED6??
//             else if (red1_state == IR_HAVE_OBSTACLE && red2_state == IR_HAVE_OBSTACLE &&
//                      red5_state == IR_NO_OBSTACLE && red6_state == IR_HAVE_OBSTACLE)
//             {
//                 Execute_Case1_3(); // ????
//             }
//             // ??3: RED1+RED2?? + RED6??+RED5??
//             else if (red1_state == IR_HAVE_OBSTACLE && red2_state == IR_HAVE_OBSTACLE &&
//                      red5_state == IR_HAVE_OBSTACLE && red6_state == IR_NO_OBSTACLE)
//             {
//                 // ????
//                 Delay_ms(200); // ??200ms
//                 Motor_TurnRight90();
//                 Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
//                 Motor_TurnRight90();
//             }
//             // ??4: RED1?? + RED2/5/6???
//             else if (red1_state == IR_HAVE_OBSTACLE && red2_state == IR_NO_OBSTACLE &&
//                      red5_state == IR_NO_OBSTACLE && red6_state == IR_NO_OBSTACLE)
//             {
//                 Execute_Case4();
//             }
//             // ??5: RED2?? + RED1/5/6???
//             else if (red1_state == IR_NO_OBSTACLE && red2_state == IR_HAVE_OBSTACLE &&
//                      red5_state == IR_NO_OBSTACLE && red6_state == IR_NO_OBSTACLE)
//             {
//                 Execute_Case5();
//             }
//             // ?????????:????????
//             else
//             {
//                 Execute_Case1_3();
//             }

//             // ?????????????
//             Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
//         }
//         // ==================== ??6:RED1/RED2??? ====================
//         else if (red1_state == IR_NO_OBSTACLE && red2_state == IR_NO_OBSTACLE)
//         {
//             Adjust_Wall_Distance();
//         }
//         // ==================== ??7:??????? ====================
//         else
//         {
//             // ????
//             Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
//         }

//         // ????
//         Delay_ms(50);
//     }
// }

// /**
//  * @brief  ?????
//  */
// void System_Init(void)
// {
//     // ???????(??????)
//     SystemInit();

//     // ??NVIC?????
//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
// }

// /**
//  * @brief  ?????GPIO
//  */
// void GPIO_Init_All(void)
// {
//     // ???????IRSensor_Init????
//     // ??????????Init??????
//     // ?????????
// }

// /**
//  * @brief  ???????????
//  * @retval 1:????, 0:?????
//  */
// uint8_t Check_Ultrasonic_Stop(void)
// {
//     static uint8_t last_valid = 0;
//     static float last_distance = 0.0f;

//     // ???????
//     float distance = Test_Distance();

//     // ???????
//     if (distance <= 0)
//     {
//         ultrasonic_invalid_count++;

//         // ????????
//         if (ultrasonic_invalid_count >= ULTRASONIC_INVALID_CNT)
//         {
//             ultrasonic_invalid_count = ULTRASONIC_INVALID_CNT;

//             // ?????????,???
//             if (last_valid && last_distance > 0)
//             {
//                 distance = last_distance;
//             }
//             else
//             {
//                 return 0; // ??????,?????
//             }
//         }
//         else
//         {
//             return 0; // ????????,?????
//         }
//     }
//     else
//     {
//         // ????,?????
//         ultrasonic_invalid_count = 0;
//         last_distance = distance;
//         last_valid = 1;
//     }

//     // ????????
//     if (distance <= ULTRASONIC_STOP_DIST)
//     {
//         // ??????,????
//         Motor_Stop();
//         return 1; // ??????
//     }

//     return 0; // ?????
// }

// /**
//  * @brief  ???????????
//  */
// void Read_IR_Sensors(void)
// {
//     red1_state = IRSensor_Detect(IR_PORT, RED1_PIN);
//     red2_state = IRSensor_Detect(IR_PORT, RED2_PIN);
//     red5_state = IRSensor_Detect(IR_PORT, RED5_PIN);
//     red6_state = IRSensor_Detect(IR_PORT, RED6_PIN);
// }

// /**
//  * @brief  ????1-3?????
//  */
// void Execute_Case1_3(void)
// {
//     // ??200ms
//     Delay_ms(200);

//     // ??90?
//     Motor_TurnLeft90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnLeft90();
// }

// /**
//  * @brief  ????4???
//  */
// void Execute_Case4(void)
// {
//     // ??3cm
//     Motor_MoveBack(3.0f);

//     // ??90?
//     Motor_TurnRight90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnLeft90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnLeft90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnRight90();
// }

// /**
//  * @brief  ????5???
//  */
// void Execute_Case5(void)
// {
//     // ??3cm
//     Motor_MoveBack(3.0f);

//     // ??90?
//     Motor_TurnLeft90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnRight90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnRight90();

//     // ??10cm
//     Motor_MoveForward(10.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);

//     // ??90?
//     Motor_TurnLeft90();
// }

// /**
//  * @brief  ??????(??6)
//  */
// void Adjust_Wall_Distance(void)
// {
//     // (1)?RED5??:?????????(????)
//     if (red5_state == IR_HAVE_OBSTACLE && red6_state == IR_NO_OBSTACLE)
//     {
//         float left_speed = NORMAL_LEFT_SPEED + WALL_ADJUST_SPEED;
//         if (left_speed > 99.0f) left_speed = 99.0f;
//         Motor_Forward(left_speed, NORMAL_RIGHT_SPEED);
//     }
//     // (2)?RED6??:?????????(????)
//     else if (red5_state == IR_NO_OBSTACLE && red6_state == IR_HAVE_OBSTACLE)
//     {
//         float right_speed = NORMAL_RIGHT_SPEED + WALL_ADJUST_SPEED;
//         if (right_speed > 99.0f) right_speed = 99.0f;
//         Motor_Forward(NORMAL_LEFT_SPEED, right_speed);
//     }
//     // ????:????
//     else
//     {
//         Motor_Forward(NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
//     }
// }
