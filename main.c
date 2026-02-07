#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "motor.h"
#include "pwm.h"
#include "IRSensor.h"
#include "delay.h"
#include "Ultrasound.h"

#define ULTRASONIC_STOP_DIST 15.0f
#define ULTRASONIC_SLOW_DIST 30.0f
#define ULTRASONIC_INVALID_CNT 2
#define SLOW_DOWN_RATIO 0.5f

uint8_t ultrasonic_invalid_count = 0;

int main(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStruct.TIM_Period = 0xFFFF;
    TIM_TimeBaseStruct.TIM_Prescaler = 71;
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
    TIM_Cmd(TIM2, ENABLE);

    Motor_Init();

    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStruct.GPIO_Pin = RED1_PIN | RED2_PIN | RED3_PIN | RED4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = RED5_PIN | RED6_PIN;
    GPIO_Init(IR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = TRIG_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStruct);
    GPIO_ResetBits(ULTRASONIC_PORT, TRIG_PIN);

    GPIO_InitStruct.GPIO_Pin = ECHO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ULTRASONIC_PORT, &GPIO_InitStruct);

    while (1)
    {
        uint8_t red1 = IRSensor_Detect(IR_PORT, RED1_PIN);
        uint8_t red2 = IRSensor_Detect(IR_PORT, RED2_PIN);
        uint8_t red3 = IRSensor_Detect(IR_PORT, RED3_PIN);
        uint8_t red4 = IRSensor_Detect(IR_PORT, RED4_PIN);
        uint8_t red5 = IRSensor_Detect(IR_PORT, RED5_PIN);
        uint8_t red6 = IRSensor_Detect(IR_PORT, RED6_PIN);

        float distance = Test_Distance();
        uint8_t ultra_stop = 0;
        uint8_t ultra_slow = 0;

        if (distance == -1.0f)
        {
            ultrasonic_invalid_count++;
            if (ultrasonic_invalid_count >= ULTRASONIC_INVALID_CNT)
            {
                ultra_stop = 1;
            }
        }
        else
        {
            ultrasonic_invalid_count = 0;
            if (distance <= ULTRASONIC_STOP_DIST)
            {
                ultra_stop = 1;
            }
            else if (distance > ULTRASONIC_STOP_DIST && distance <= ULTRASONIC_SLOW_DIST)
            {
                ultra_slow = 1;
            }
        }

        if (ultra_stop)
        {
            Motor_Stop();
            Delay_ms(200);
            Motor_MoveBack(10);
            Delay_ms(100);
        }
        else if (ultra_slow && red1 && red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_Stop();
            Delay_ms(200);
            Motor_MoveBack(3);
            Motor_TurnRight90();
            Motor_MoveForward(10, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnRight90();
            Motor_ResumeNormal();
        }
        else if (ultra_slow && red1 && !red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_MoveBack(3);
            Motor_TurnRight90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnLeft90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnLeft90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnRight90();
            Motor_ResumeNormal();
        }
        else if (ultra_slow && !red1 && red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_Stop();
            Delay_ms(200);
            Motor_MoveBack(3);
            Motor_TurnLeft90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnRight90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnRight90();
            Motor_MoveForward(14, NORMAL_LEFT_SPEED, NORMAL_RIGHT_SPEED);
            Motor_TurnLeft90();
            Motor_ResumeNormal();
        }
        else if (!red1 && !red2 && (red3 || red5))
        {
            float left_speed = NORMAL_LEFT_SPEED;
            float right_speed = NORMAL_RIGHT_SPEED;

            if (red3 && red5)
                left_speed += 10;
            else if (red3)
                right_speed += 10;
            else if (red5)
                left_speed += 10;

            Motor_Forward(left_speed, right_speed);
        }
        else if (!red1 && !red2 && (red4 || red6))
        {
            float left_speed = NORMAL_LEFT_SPEED;
            float right_speed = NORMAL_RIGHT_SPEED;

            if (red4 && red6)
                right_speed += 10;
            else if (red4)
                left_speed += 10;
            else if (red6)
                right_speed += 10;

            Motor_Forward(left_speed, right_speed);
        }
        else if (ultra_slow && !red1 && !red2 && !red3 && !red4 && !red5 && !red6)
        {
            Motor_Stop();
            Delay_ms(200);
            Motor_MoveBack(10);
            Delay_ms(100);
        }
        else
        {
            Motor_ResumeNormal();
        }

        PWM_Task();
        Delay_us(10);
    }
}
