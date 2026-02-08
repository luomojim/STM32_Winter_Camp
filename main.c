#include "stm32f10x.h"
#include "delay.h"
#include "motor.h"
#include "IRSensor.h"
#include "Ultrasound.h"

// ================= 宏定义参数 =================
#define STOP_DISTANCE 15.0f   // 超声波停车距离(cm)
#define WALL_ADJUST_PWM 15.0f // 巡墙纠偏时增加的PWM值
#define STRAIGHT_TIMEOUT 12000 // 直行超时时间(ms)

// ================= 状态变量 =================
uint8_t r1, r2, r5, r6;
float distance;
uint32_t straight_time = 0; // 直行时间计数器 (ms)
uint8_t straight_mode = 0;  // 直行状态标记: 0=非直行, 1=正常直行

// ================= 函数声明 =================
void System_Init_All(void);
void Check_Straight_Timeout(void);

int main(void)
{
    // 1. 系统初始化
    System_Init_All();

    // 启动延时，防止上电瞬间乱跑
    Delay_ms(1000);

    while (1)
    {
        // 2. 读取所有传感器
        r1 = IRSensor_Detect(IR_PORT, RED1_PIN); // 左前
        r2 = IRSensor_Detect(IR_PORT, RED2_PIN); // 右前
        r5 = IRSensor_Detect(IR_PORT, RED5_PIN); // 左侧
        r6 = IRSensor_Detect(IR_PORT, RED6_PIN); // 右侧
        distance = Test_Distance();

        // 3. 判断是否需要停车 (超声波触发 或 任意前方红外触发)
        uint8_t ultra_stop = 0;
        if (distance > 0.1f && distance <= STOP_DISTANCE)
        {
            ultra_stop = 1;
        }
        uint8_t front_obstacle = (r1 == IR_HAVE_OBSTACLE || r2 == IR_HAVE_OBSTACLE);

        // 4. 执行场景逻辑
        if (ultra_stop || front_obstacle)
        {
            // 前方有障碍或超声波触发，退出直行模式
            straight_mode = 0;
            straight_time = 0;
            
            // 场景1-5: 超声停车触发或前方红外触发
            Motor_Stop();
            Delay_ms(1000); // 停车1s

            // 场景1: RED1+RED2同触 + RED5/6均未触
            if (r1 == IR_HAVE_OBSTACLE && r2 == IR_HAVE_OBSTACLE && 
                r5 == IR_NO_OBSTACLE && r6 == IR_NO_OBSTACLE)
            {
                Delay_ms(200);
                Motor_MoveBack(10.0f);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_ResumeNormal();
            }
            // 场景2: RED1+RED2同触 + RED5未触+RED6触发
            else if (r1 == IR_HAVE_OBSTACLE && r2 == IR_HAVE_OBSTACLE && 
                     r5 == IR_NO_OBSTACLE && r6 == IR_HAVE_OBSTACLE)
            {
                Delay_ms(200);
                Motor_MoveBack(10.0f);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_ResumeNormal();
            }
            // 场景3: RED1+RED2同触 + RED6未触+RED5触发
            else if (r1 == IR_HAVE_OBSTACLE && r2 == IR_HAVE_OBSTACLE && 
                     r5 == IR_HAVE_OBSTACLE && r6 == IR_NO_OBSTACLE)
            {
                Delay_ms(200);
                Motor_MoveBack(10.0f);
                Motor_TurnRight90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnRight90();
                Motor_ResumeNormal();
            }
            // 场景4: RED1单触 + RED2/5/6均未触
            else if (r1 == IR_HAVE_OBSTACLE && r2 == IR_NO_OBSTACLE && 
                     r5 == IR_NO_OBSTACLE && r6 == IR_NO_OBSTACLE)
            {
                Motor_MoveBack(10.0f);
                Motor_TurnRight90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnRight90();
                Motor_ResumeNormal();
            }
            // 场景5: RED2单触 + RED1/5/6均未触
            else if (r1 == IR_NO_OBSTACLE && r2 == IR_HAVE_OBSTACLE && 
                     r5 == IR_NO_OBSTACLE && r6 == IR_NO_OBSTACLE)
            {
                Motor_MoveBack(10.0f);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnRight90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnRight90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_ResumeNormal();
            }
            // 兜底逻辑: 不符合以上任何情况时
            else
            {
                Delay_ms(200);
                Motor_MoveBack(10.0f);
                Motor_TurnLeft90();
                Motor_MoveForward(5.0f, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
                Motor_TurnLeft90();
                Motor_ResumeNormal();
            }
        }
        else
        {
            // 前方无障碍，进入或保持在直行模式
            if (straight_mode == 0)
            {
                // 新开始直行，重置计时器
                straight_mode = 1;
                straight_time = 0;
            }
            
            // 场景6: RED1/RED2均未触，根据左右传感器调整
            // (1) 若RED5单触: 左轮加速、右轮正常（远离左墙）
            if (r5 == IR_HAVE_OBSTACLE && r6 == IR_NO_OBSTACLE)
            {
                float left_speed = NORMAL_LEFT_SPEED + WALL_ADJUST_PWM;
                if (left_speed > 99)
                    left_speed = 99;
                Motor_Forward(left_speed, NORMAL_RIGHT_SPEED);
                // 巡墙也累积时间
                straight_time += 10;
            }
            // (2) 若RED6单触: 右轮加速、左轮正常（远离右墙）
            else if (r5 == IR_NO_OBSTACLE && r6 == IR_HAVE_OBSTACLE)
            {
                float right_speed = NORMAL_RIGHT_SPEED + WALL_ADJUST_PWM;
                if (right_speed > 99)
                    right_speed = 99;
                Motor_Forward(NORMAL_LEFT_SPEED, right_speed);
                // 巡墙也累积时间
                straight_time += 10;
            }
            // 场景7: 无任何触发条件，正常直行
            else
            {
                Motor_ResumeNormal();
                straight_time += 10;
            }
            
            // 检查直行超时
            Check_Straight_Timeout();
        }

        Delay_ms(10); // 循环小延时
    }
}

// 检查直行超时函数
void Check_Straight_Timeout(void)
{
    if (straight_mode == 1 && straight_time >= STRAIGHT_TIMEOUT)
    {
        // 执行倒车10cm→右转90度→正常直行
        Motor_MoveBack(10.0f);
        Motor_TurnRight90();
        Motor_ResumeNormal();
        
        // 重置计时器，保持直行模式
        straight_time = 0;
    }
}

void System_Init_All(void)
{
    SystemInit();
    delay_init();      // 延时初始化
    Motor_Init();      // 电机初始化 (包含TIM2和GPIO)
    IRSensor_Init();   // 红外初始化 (包含GPIO)
    Ultrasound_Init(); // 超声波初始化 (包含TIM1和GPIO)
}