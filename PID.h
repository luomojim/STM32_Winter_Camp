#ifndef __PID_H
#define __PID_H

#include "stm32f10x.h"

// PID结构体定义
typedef struct
{
    float Kp; // Proportional gain
    float Ki; // Integral gain
    float Kd; // Derivative gain

    float Target;    // Target value
    float Actual;    // Actual value
    float Error;     // Current error
    float LastError; // Last error
    float SumError;  // Sum of error (for integral)
    float Output;    // PID output value
} PID_TypeDef;

// 外部使用的PID结构体（实际应用）
extern PID_TypeDef PID_MotorLeft;
extern PID_TypeDef PID_MotorRight;

// 函数声明
void PID_Init(void);
float PID_Calc(PID_TypeDef *pid, float target, float actual);

#endif
