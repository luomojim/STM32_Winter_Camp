#include "IRSensor.h"
#include "delay.h"

void IRSensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置所有红外传感器引脚为上拉输入
    GPIO_InitStruct.GPIO_Pin = RED1_PIN | RED2_PIN | RED3_PIN | RED4_PIN | RED5_PIN | RED6_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入，无障碍物时高电平，有障碍物时低电平
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IR_PORT, &GPIO_InitStruct);
}

uint8_t IRSensor_Detect(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    // 消抖处理：连续读取两次，值相同则有效
    uint8_t val1 = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    Delay_us(10);  // 采样延迟 - 根据实际情况调整
    uint8_t val2 = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    
    uint8_t result = (val1 == val2) ? val1 : 1;
    
    // 低电平表示检测到障碍物(IR_HAVE_OBSTACLE)，高电平表示未检测到(IR_NO_OBSTACLE)
    return result == 0 ? IR_HAVE_OBSTACLE : IR_NO_OBSTACLE;
}
