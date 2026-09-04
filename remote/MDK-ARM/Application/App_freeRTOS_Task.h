#ifndef __APP_FREERTOS_TASK_H
#define __APP_FREERTOS_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_IP5305T.h"
#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "App_transmit_data.h"
#include "App_display.h"

/// @brief Æô¶¯FreeRTOSÈÎÎñ
/// @param  
void APP_freeRTOS_start(void);

#endif
