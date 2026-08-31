#ifndef __INT_JOYSTICK__
#define __INT_JOYSTICK__

#include "adc.h"

typedef struct
{
    int16_t thr; //油门，左摇杆上下移动
    int16_t yaw; //偏航角，左摇杆左右移动
    int16_t pit; //俯仰角，右摇杆上下移动
    int16_t rol; //横滚角，右摇杆左右移动
} Joystick_Struct;

/// @brief 初始化ADC遥控 
/// @param  
void Int_joystick_init(void);

/// @brief 读取ADC数据保存到结构体地址中
/// @param joystick 
void Int_joystick_get(Joystick_Struct *joystick);

#endif
