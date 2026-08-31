#ifndef __APP_PROCESS_DATA_H
#define __APP_PROCESS_DATA_H

#include "Int_key.h"
#include "Int_joystick.h"
#include "Com_debug.h"
#include "Com_tool.h"

typedef struct 
{
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;
    uint8_t shutdown; //1关；0不关
    uint8_t fix_height; // 1. 切换定高和不定高 0: 不切换
}Remote_Data;

/// @brief 处理按键数据
/// @param  
void App_process_key_data(void);

/// @brief 处理摇杆数据
/// @param  
void App_process_joystick_data(void);
#endif
