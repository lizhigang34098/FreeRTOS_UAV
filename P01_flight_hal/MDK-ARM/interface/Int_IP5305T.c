#include "Int_IP5305T.h"

/**
 * @file  Int_IP5305T.c
 * @brief IP5305T 电源管理驱动：通过 KEY 脚模拟按键，控制整板上电/关机
 *
 * 硬件连接（P01 飞控板）：
 *   - MCU PB2 --R17(10k)-- IP5305T KEY 脚(PIN5)，物理按键 SW1 与 R17 并联
 *   - IP5305T VOUT(8) -> +5V -> AMS1117-3.3 -> +3.3V 给整板供电，
 *     因此双击关机后整板（含 MCU）一并掉电；若 MCU 跑飞不再踢脚，
 *     芯片会在低载约 32s 后自动关机，天然形成掉电保护
 *
 * 按键时序依据数据手册 V1.4 第 11 节「按键方式」：
 *   - 短按：按压 >30ms 且 <2s  => 打开升压输出与电量灯
 *   - 长按：按压 >2s          => 开关照明 LED（本板未接照明灯，本模块不提供）
 *   - 双击：1s 内连续两次短按  => 关闭升压输出与电量灯
 *
 * 说明：IP5305T 为纯模拟 SOC，无 I2C/寄存器可读；电量灯由芯片内部
 * 驱动 LED1~3 脚（本板经 Q1~Q4 点亮 VBAT 指示灯），MCU 无法读取，
 * 故本模块只提供开机/防关机/关机三类 KEY 操作。
 */

// 实际模拟的按键按压时长：100ms，处于短按窗口 [30ms, 2s] 内
#define IP5305T_KEY_PRESS_MS        100
// 双击两击之间的间隔：200ms，两次按键总跨度 400ms < 1s 关机窗口
#define IP5305T_DOUBLE_CLICK_GAP_MS 200

/**
 * @brief 短按 KEY：开机 / 防自动关机
 */
void Int_IP5305T_start(void)
{
    // 按下：KEY 拉低，模拟物理按键导通
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(IP5305T_KEY_PRESS_MS);
    // 松开
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}

/**
 * @brief 双击 KEY：关机
 *
 * 1s 内两次有效短按 => IP5305T 关闭升压输出，整板断电。
 * 注意：若整板被 USB/SWD 独立供电，MCU 不会随关机掉电，调用方
 * 需自行避免在关机后仍周期调用 start() 造成复开机。
 */
void Int_IP5305T_shutdown(void)
{
    // 第一击
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(IP5305T_KEY_PRESS_MS);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);

    // 间隔，确保两次按键落在 1s 窗口内
    vTaskDelay(IP5305T_DOUBLE_CLICK_GAP_MS);

    // 第二击
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(IP5305T_KEY_PRESS_MS);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}
