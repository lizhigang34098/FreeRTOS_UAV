#ifndef __INT_MOTOR__
#define __INT_MOTOR__

#include "tim.h"
#include "Com_debug.h"

//电机结构体，定时器，通道，速度
typedef struct
{
    TIM_HandleTypeDef *tim;
    uint32_t channel;
    uint16_t speed;
} Motor_Struct;

/**
 * @brief 传入的参数其实是比较值  最大为1000  默认值为200
 *
 * @param speed
 */
void Int_motor_set_speed(Motor_Struct *motor);

/**
 * @brief 启动电机 传入具体电机的结构体
 *
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor);

#endif // __INT_MOTOR__
