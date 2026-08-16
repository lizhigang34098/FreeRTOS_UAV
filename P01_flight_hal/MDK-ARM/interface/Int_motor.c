#include "Int_motor.h"

/**
 * @brief ����Ĳ�����ʵ�ǱȽ�ֵ  ���Ϊ1000  Ĭ��ֵΪ200
 *
 * @param speed
 */
void Int_motor_set_speed(Motor_Struct *motor)
{
    if(motor->speed > 1000)
    {
        debug_printf("motor speed is too high, max is 1000\r\n");
        return;
    }
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);
}


/**
 * @brief ������� ����������Ľṹ��
 *
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, 0);
    HAL_TIM_PWM_Start(motor->tim, motor->channel);
}
