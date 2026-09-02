#ifndef __COM_CONFIG_H
#define __COM_CONFIG_H

#include "main.h"
//连接状态
typedef enum
{
    REMOTE_CONNECTED = 0,
    REMOTE_DISCONNECTED,
}Remote_State;

// 飞行状态
typedef enum
{
    IDLE = 0,// 飞机处于空闲状态
    NORMAL,// 飞机处于正常飞行状态
    FIX_HEIGHT,// 飞机处于定高状态
    FAIL,// 飞机处于故障失联状态
}Flight_State;

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown;   // 1: 关闭  0: 不关机
    uint8_t fix_height; // 1. 切换定高和不定高 0: 不切换
} Remote_Data;

typedef enum
{
    FREE=0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
}Thr_state;
#endif
