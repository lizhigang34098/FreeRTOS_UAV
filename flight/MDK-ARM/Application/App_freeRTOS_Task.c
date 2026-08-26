#include "App_freeRTOS_Task.h"


void task1(void *args);
#define TASK1_STACK_SIZE 128
#define TASK1_PRIORITY 1
TaskHandle_t task1_Handle;


/// @brief 启动FreeRTOS任务
/// @param  
void APP_freeRTOS_start(void){
    xTaskCreate(task1,"task1",TASK1_STACK_SIZE,NULL,TASK1_PRIORITY,&task1_Handle);
    vTaskStartScheduler();
}

void task1(void *args){
    while(1){
        debug_printf("task1 is running\r\n");
        vTaskDelay(1000);
    }
}
