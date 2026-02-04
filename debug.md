*** Using Compiler 'V5.06 update 5 (build 528)', folder: 'E:\Keil_v5\ARM\ARMCC\Bin'
Build target 'Target 1'
Clean started - Project: 'project'
   deleting intermediate output files for target 'Target 1'
compiling core_cm3.c...
assembling startup_stm32f10x_md.s...
compiling system_stm32f10x.c...
start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
start\system_stm32f10x.c: 1 warning, 1 error
compiling stm32f10x_it.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
user\stm32f10x_it.c: 1 warning, 1 error
compiling main.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
main.c(40): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
      float left_pwm = PID_Calc(&PID_MotorLeft, target_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(41): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
      float right_pwm = PID_Calc(&PID_MotorRight, target_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(85): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
      float left_pwm = PID_Calc(&PID_MotorLeft, target_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(86): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
      float right_pwm = PID_Calc(&PID_MotorRight, target_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(240): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCompare1(MOTOR_TIM));
main.c(241): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(246): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(247): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCompare2(MOTOR_TIM));
main.c(252): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCompare1(MOTOR_TIM));
main.c(253): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(257): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(258): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(288): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(289): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCompare2(MOTOR_TIM));
main.c(294): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm + 10, TIM_GetCompare1(MOTOR_TIM));
main.c(295): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(300): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(301): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm + 10, TIM_GetCompare2(MOTOR_TIM));
main.c(305): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
                  left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(306): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
                  right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(333): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
              float left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(334): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
              float right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c(360): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
              float left_pwm = PID_Calc(&PID_MotorLeft, current_speed_pwm, TIM_GetCompare1(MOTOR_TIM));
main.c(361): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
              float right_pwm = PID_Calc(&PID_MotorRight, current_speed_pwm, TIM_GetCompare2(MOTOR_TIM));
main.c: 25 warnings, 1 error
compiling misc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\misc.c: 1 warning, 1 error
compiling stm32f10x_adc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_adc.c: 1 warning, 1 error
compiling stm32f10x_bkp.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_bkp.c: 1 warning, 1 error
compiling stm32f10x_can.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_can.c: 1 warning, 1 error
compiling stm32f10x_cec.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_cec.c: 1 warning, 1 error
compiling stm32f10x_crc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_crc.c: 1 warning, 1 error
compiling stm32f10x_dac.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_dac.c: 1 warning, 1 error
compiling stm32f10x_dbgmcu.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_dbgmcu.c: 1 warning, 1 error
compiling stm32f10x_dma.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_dma.c: 1 warning, 1 error
compiling stm32f10x_exti.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_exti.c: 1 warning, 1 error
compiling stm32f10x_flash.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_flash.c: 1 warning, 1 error
compiling stm32f10x_fsmc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_fsmc.c: 1 warning, 1 error
compiling stm32f10x_gpio.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_gpio.c: 1 warning, 1 error
compiling stm32f10x_i2c.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_i2c.c: 1 warning, 1 error
compiling stm32f10x_iwdg.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_iwdg.c: 1 warning, 1 error
compiling stm32f10x_pwr.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_pwr.c: 1 warning, 1 error
compiling stm32f10x_rcc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_rcc.c: 1 warning, 1 error
compiling stm32f10x_rtc.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_rtc.c: 1 warning, 1 error
compiling stm32f10x_sdio.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_sdio.c: 1 warning, 1 error
compiling stm32f10x_spi.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_spi.c: 1 warning, 1 error
compiling stm32f10x_tim.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_tim.c: 1 warning, 1 error
compiling stm32f10x_usart.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_usart.c: 1 warning, 1 error
compiling stm32f10x_wwdg.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
library\stm32f10x_wwdg.c: 1 warning, 1 error
compiling pwm.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
pwm.c: 1 warning, 1 error
compiling delay.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
delay.c: 1 warning, 1 error
compiling led.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
led.c: 1 warning, 1 error
compiling oled.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
oled.c(92): error:  #20: identifier "OLED_F8x16" is undefined
        for (i = 0; i < 8; i++) OLED_WriteData(OLED_F8x16[Char - ' '][i]);
oled.c(94): error:  #20: identifier "OLED_F8x16" is undefined
        for (i = 0; i < 8; i++) OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
oled.c(101): error:  #20: identifier "OLED_chinese" is undefined
        for (i = 0; i < 8; i++) OLED_WriteData(OLED_chinese[num][i]);
oled.c(103): error:  #20: identifier "OLED_chinese" is undefined
        for (i = 0; i < 8; i++) OLED_WriteData(OLED_chinese[num][i + 8]);
oled.c: 1 warning, 5 errors
compiling motor.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
motor.c(75): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
      float left_pwm = PID_Calc(&PID_MotorLeft, 99, TIM_GetCompare1(MOTOR_TIM));   // ç›®æ ‡PWMå€? - æœ€å¤§é€Ÿåº¦
motor.c(76): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
      float right_pwm = PID_Calc(&PID_MotorRight, 99, TIM_GetCompare2(MOTOR_TIM)); // ç›®æ ‡PWMå€? - æœ€å¤§é€Ÿåº¦
motor.c(101): warning:  #223-D: function "TIM_GetCompare2" declared implicitly
      float right_pwm = PID_Calc(&PID_MotorRight, 50, TIM_GetCompare2(MOTOR_TIM)); // ç›®æ ‡PWMå€? - æ ¹æ®å®žé™…æƒ…å†µè°ƒæ•´
motor.c(122): warning:  #223-D: function "TIM_GetCompare1" declared implicitly
      float left_pwm = PID_Calc(&PID_MotorLeft, 50, TIM_GetCompare1(MOTOR_TIM)); // ç›®æ ‡PWMå€? - æ ¹æ®å®žé™…æƒ…å†µè°ƒæ•´
motor.c: 5 warnings, 1 error
compiling Ultrasound.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
Ultrasound.c: 1 warning, 1 error
compiling IRSensor.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
IRSensor.c: 1 warning, 1 error
compiling PID.c...
.\start\stm32f10x.h(252): error:  #67: expected a "}"
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
.\start\stm32f10x.h(480): warning:  #12-D: parsing restarts here after previous syntax error
  } IRQn_Type;
PID.c: 1 warning, 1 error
".\Objects\project.axf" - 38 Error(s), 62 Warning(s).

*** Performing Cross-Module-Optimization:
*** Feedback file '.\Objects\project.fed' not found.
Target not created.
Build Time Elapsed:  00:00:06