#include "PID.h"

// 定义PID结构体变量
PID_TypeDef PID_MotorLeft;
PID_TypeDef PID_MotorRight;

void PID_Init(void)
{
    // Left motor PID parameters (Kp:1.5-3.5, Ki:0.05-0.2, Kd:0.03-0.1)
    PID_MotorLeft.Kp = 2.5f;  // 比例系数 - 根据实际情况调整
    PID_MotorLeft.Ki = 0.1f;  // 积分系数 - 根据实际情况调整
    PID_MotorLeft.Kd = 0.05f;  // 微分系数 - 根据实际情况调整
    PID_MotorLeft.Target = 0;
    PID_MotorLeft.Actual = 0;
    PID_MotorLeft.Error = 0;
    PID_MotorLeft.LastError = 0;
    PID_MotorLeft.SumError = 0;
    PID_MotorLeft.Output = 0;
    
    // Right motor PID parameters (same as left for balance)
    PID_MotorRight.Kp = 2.5f;  // 比例系数 - 根据实际情况调整
    PID_MotorRight.Ki = 0.1f;  // 积分系数 - 根据实际情况调整
    PID_MotorRight.Kd = 0.05f;  // 微分系数 - 根据实际情况调整
    PID_MotorRight.Target = 0;
    PID_MotorRight.Actual = 0;
    PID_MotorRight.Error = 0;
    PID_MotorRight.LastError = 0;
    PID_MotorRight.SumError = 0;
    PID_MotorRight.Output = 0;
}

float PID_Calc(PID_TypeDef *pid, float target, float actual)
{
    pid->Target = target;
    pid->Actual = actual;
    pid->Error = pid->Target - pid->Actual;
    
    // Integral calculation (with anti-windup)
    pid->SumError += pid->Error;
    if(pid->SumError > 500) pid->SumError = 500;  // 积分上限 - 根据实际情况调整
    if(pid->SumError < -500) pid->SumError = -500;  // 积分下限 - 根据实际情况调整
    
    // PID output calculation
    pid->Output = pid->Kp * pid->Error + pid->Ki * pid->SumError + pid->Kd * (pid->Error - pid->LastError);
    
    // Output limit (0~100, match PWM range)
    if(pid->Output > 100) pid->Output = 100;  // 输出上限 - 根据实际情况调整
    if(pid->Output < 0) pid->Output = 0;  // 输出下限 - 根据实际情况调整
    
    // Save current error as last error for next calculation
    pid->LastError = pid->Error;
    
    return pid->Output;
}
