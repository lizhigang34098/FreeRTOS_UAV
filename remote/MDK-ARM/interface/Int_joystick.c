#include "Int_joystick.h"


uint16_t adc_buff[4] = {0};


/**
 * @brief 初始化ADC遥控  打开ADC
 */
void Int_joystick_init(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff,4);
}

/**
 * @brief 读取ADC数据 保存到结构体地址中
 * @param joystick 
 */
void Int_joystick_get(Joystick_Struct *joystick)
{
    joystick->thr = adc_buff[0];
    joystick->yaw = adc_buff[1];
    joystick->pit = adc_buff[2];
    joystick->rol = adc_buff[3];
}
