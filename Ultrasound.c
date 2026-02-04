#include "Ultrasound.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "delay.h"

// 全局变量定义
uint16_t ultrasonic_counter;  // 超声回波计数器
uint16_t ultrasonic_over_cnt; // 超声定时器溢出计数器

void Ultrasonic_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // 配置TRIG引脚：推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_TRIG_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStructure);

    // 配置ECHO引脚：上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_ECHO_PIN;
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStructure);

    // 配置定时器2时间参数
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Period = 60000 - 1; // 定时器周期 - 根据实际情况调整
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1; // 定时器预分频器 - 根据实际情况调整
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    // 配置定时器2中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 2);

    // 初始状态：关闭定时器，清空计数器
    TIM_Cmd(TIM2, DISABLE);
    TIM_SetCounter(TIM2, 0);
    ultrasonic_over_cnt = 0;
}

// 定时器2中断处理函数
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        ultrasonic_over_cnt++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

float Test_Distance(void)
{
    // 禁用中断，保护全局变量访问
    __disable_irq();
    ultrasonic_over_cnt = 0;
    __enable_irq();

    TIM_SetCounter(TIM2, 0);

    // 发送TRIG触发信号（10us高电平）
    GPIO_SetBits(ULTRASONIC_PORT, ULTRASONIC_TRIG_PIN);
    Delay_us(10); // TRIG触发信号延迟 - 根据实际情况调整
    GPIO_ResetBits(ULTRASONIC_PORT, ULTRASONIC_TRIG_PIN);

    // 等待ECHO高电平，超时处理
    uint32_t timeout = 100000; // 超时时间 - 根据实际情况调整
    while (GPIO_ReadInputDataBit(ULTRASONIC_PORT, ULTRASONIC_ECHO_PIN) == RESET && timeout--)
    {
        if (timeout == 0)
            return 0;
    }

    // 开始计时
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);

    // 等待ECHO低电平，超时处理
    timeout = 100000; // 超时时间 - 根据实际情况调整
    while (GPIO_ReadInputDataBit(ULTRASONIC_PORT, ULTRASONIC_ECHO_PIN) == SET && timeout--)
    {
        if (timeout == 0)
        {
            TIM_Cmd(TIM2, DISABLE);
            TIM_SetCounter(TIM2, 0);
            return 0;
        }
    }

    // 停止计时，计算时间
    TIM_Cmd(TIM2, DISABLE);

    // 禁用中断，保护全局变量访问
    __disable_irq();
    uint16_t over_cnt = ultrasonic_over_cnt;
    __enable_irq();

    ultrasonic_counter = TIM_GetCounter(TIM2) + over_cnt * 60000;

    // 计算距离：声速340m/s = 0.034cm/us，距离=时间×声速/2
    float distance = (ultrasonic_counter * 1.0 * 0.034) / 2; // 声速常量 - 根据实际情况调整

    // 重置计数器
    TIM_SetCounter(TIM2, 0);

    // 距离有效性判断（1cm ~ 400cm）
    if (distance > 400)
        distance = 400; // 最大距离阈值 - 根据实际情况调整
    if (distance < 1)
        distance = 0; // 最小距离阈值 - 根据实际情况调整

    // 延时，避免测量频率过高
    Delay_ms(20); // 测量间隔 - 根据实际情况调整

    return distance;
}
