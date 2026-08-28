#include "App_freeRTOS_Task.h"

// 电源管理任务
void power_task(void *args);
#define POWER_TASK_STACK_SIZE 128
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t com_task_handle;
#define COM_TASK_PERIOD 6

/// @brief 启动FreeRTOS任务
/// @param  
void APP_freeRTOS_start(void){
    xTaskCreate(power_task,"power_task",POWER_TASK_STACK_SIZE,NULL,POWER_TASK_PRIORITY,&power_task_handle);
    
     // 创建通讯任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);


    vTaskStartScheduler();
}

void power_task(void *args){
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1){
        //10s启动一次IP5305T电源
        vTaskDelayUntil(&xLastWakeTime, 10000);
        Int_IP5305T_start();
    }
}

void com_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        // 将打包完成的数据发送到飞机
        App_transmit_data();
        // 6ms执行一次
        vTaskDelayUntil(&xLastWakeTime, COM_TASK_PERIOD);
    }
}
