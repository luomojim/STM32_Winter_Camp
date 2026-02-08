#include "stm32f10x.h" // Device header
#include "Ultrasound.h"
#include "Delay.h"

#define ULTRA_PORT GPIOB
#define ULTRA_TRIG_PIN GPIO_Pin_0
#define ULTRA_ECHO_PIN GPIO_Pin_1

// 超时阈值，防止while死循环 (约20ms)
#define MAX_TIMEOUT 20000

uint16_t Cnt;
uint16_t OverCnt;

void Ultrasound_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // TRIG (PB0) -> 推挽输出
    GPIO_InitStructure.GPIO_Pin = ULTRA_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRA_PORT, &GPIO_InitStructure);

    // ECHO (PB1) -> 下拉输入 (防止悬空误触)
    GPIO_InitStructure.GPIO_Pin = ULTRA_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(ULTRA_PORT, &GPIO_InitStructure);

    // 定时器初始化 (1us计数一次)
    TIM_TimeBaseInitStructure.TIM_Period = 59999;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 71; // 72MHz/72 = 1MHz
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 默认拉低TRIG
    GPIO_ResetBits(ULTRA_PORT, ULTRA_TRIG_PIN);
}

float Test_Distance(void)
{
    uint32_t timeout = 0;

    // 1. 发送至少10us的高电平触发信号
    GPIO_SetBits(ULTRA_PORT, ULTRA_TRIG_PIN);
    Delay_us(20);
    GPIO_ResetBits(ULTRA_PORT, ULTRA_TRIG_PIN);

    // 2. 等待 ECHO 变高 (开始发送波)
    timeout = 0;
    while (GPIO_ReadInputDataBit(ULTRA_PORT, ULTRA_ECHO_PIN) == RESET)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT)
            return -1.0; // 超时，传感器未连接或损坏
        Delay_us(1);
    }

    // 3. 开始计时
    TIM_SetCounter(TIM1, 0);
    TIM_Cmd(TIM1, ENABLE);

    // 4. 等待 ECHO 变低 (收到回波)
    timeout = 0;
    while (GPIO_ReadInputDataBit(ULTRA_PORT, ULTRA_ECHO_PIN) == SET)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) // 超时 (超出最大量程)
        {
            TIM_Cmd(TIM1, DISABLE);
            return 999.0; // 返回一个大数值表示无穷远
        }
        Delay_us(1);
    }

    // 5. 停止计时并计算
    TIM_Cmd(TIM1, DISABLE);
    uint16_t count = TIM_GetCounter(TIM1);

    // 距离(cm) = 时间(us) * 340m/s / 2 / 10000
    //         = count * 0.017
    float distance = (float)count * 0.017f;

    Delay_ms(10); // 采样间隔
    return distance;
}