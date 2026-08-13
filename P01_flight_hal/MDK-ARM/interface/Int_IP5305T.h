#ifndef __INT_IP5305T__
#define __INT_IP5305T__

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 短按 KEY：开机 / 防自动关机
 *
 * 模拟一次有效短按（按压 100ms，处于芯片识别窗口 30ms~2s 内）。
 * 首次上电时用于打开 IP5305T 升压输出；运行期间由 power_task
 * 每 10s 周期调用，防止芯片低载约 32s 后自动关机。
 */
void Int_IP5305T_start(void);

/**
 * @brief 双击 KEY：关机
 *
 * 模拟 1s 内连续两次短按，关闭 IP5305T 升压输出，整板断电。
 */
void Int_IP5305T_shutdown(void);

#endif // __INT_IP5305T__
