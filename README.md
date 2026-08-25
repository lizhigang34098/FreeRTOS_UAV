# FreeRTOS_UAV 开发指南

## 项目概述

这是一个基于 STM32F103C8T6 的无人机系统，包含两个独立的 Keil MDK 工程：

- **P01_flight_hal**: 飞控板（接收遥控指令、姿态解算、PID控制、电机输出）
- **P02_remote_hal**: 遥控器（摇杆采集、按键输入、OLED显示、无线发送）

两者通过 SI24R1 2.4GHz 无线模块通信，使用自定义 17 字节帧协议。

## 工具链与构建

- **IDE**: Keil MDK-ARM (uVision)
- **工程文件**: `P01_flight_hal/MDK-ARM/P01_flight_hal.uvprojx` 和 `P02_remote_hal/MDK-ARM/P02_remote_hal.uvprojx`
- **MCU**: STM32F103C8T6 (Cortex-M3, 72MHz, 20KB SRAM, 64KB Flash)
- **代码生成**: STM32CubeMX (`.ioc` 文件)
- **无命令行构建**: 本项目必须在 Keil MDK 中编译，无 Makefile 或 CMake

## 项目结构

```
├── Core/                    # CubeMX 生成的 HAL 初始化代码 (勿手动修改)
│   ├── Inc/                 # 外设头文件 (main.h, gpio.h, spi.h, ...)
│   └── Src/                 # 外设初始化 (main.c, gpio.c, spi.c, ...)
├── Drivers/                 # ST 官方 HAL 驱动 (CubeMX 自动生成)
└── MDK-ARM/                 # ★ 自定义代码全部在此目录
    ├── Application/         # 应用层 (业务逻辑)
    ├── interface/           # 硬件驱动层 (外设封装)
    ├── common/              # 通用算法层 (PID, IMU, 滤波, 调试)
    └── freeRTOS/            # FreeRTOS 内核源码
```

## 三层架构

### Application 层 (MDK-ARM/Application/)

| 文件              | 飞控 (P01)                      | 遥控器 (P02)            |
| ----------------- | ------------------------------- | ----------------------- |
| App_freeRTOS_Task | FreeRTOS 任务创建与调度         | FreeRTOS 任务创建与调度 |
| App_flight        | 飞行控制核心 (MPU6050→PID→电机) | -                       |
| App_receive_data  | 遥控数据接收与协议解析          | -                       |
| App_transmit_data | -                               | 数据打包与发送          |
| App_process_data  | -                               | 摇杆数据处理与按键事件  |
| App_display       | -                               | OLED 显示控制           |

### interface 层 (MDK-ARM/interface/)

| 文件         | 功能                                     |
| ------------ | ---------------------------------------- |
| Int_SI24R1   | SI24R1 无线模块 SPI 驱动 (兼容 nRF24L01) |
| Int_motor    | PWM 电机驱动 (TIM1/TIM2/TIM3/TIM4)       |
| Int_led      | LED GPIO 控制 (低电平点亮)               |
| Int_mpu6050  | MPU6050 六轴 IMU I2C 驱动                |
| Int_bat_ADC  | 电池电压 ADC 采样 (分压比 1:2)           |
| Int_IP5305T  | IP5305T 电源管理芯片控制                 |
| Int_VL53L1X  | VL53L1X 激光测距 ToF 传感器              |
| Int_joystick | ADC 摇杆采集 (DMA)                       |
| Int_key      | 6 按键检测 (含消抖与长按)                |
| Inf_OLED     | 0.96 寸 OLED SPI 驱动                    |

### common 层 (MDK-ARM/common/)

| 文件       | 功能                                        |
| ---------- | ------------------------------------------- |
| Com_config | 全局类型定义 (Remote_Data, Flight_State 等) |
| Com_debug  | printf 重定向到 UART + debug_printf 宏      |
| Com_pid    | PID 控制器 + 串级 PID                       |
| Com_imu    | 四元数姿态解算 (Mahony 互补滤波)            |
| Com_filter | 角速度一阶低通滤波 + 加速度卡尔曼滤波       |
| Com_tool   | 通用工具函数 (限幅)                         |

## 无线通信协议

### 硬件配置

- **模块**: SI24R1 (兼容 nRF24L01)
- **频段**: 2.440GHz (Channel 40)
- **速率**: 1Mbps
- **功率**: -4dBm (飞控) / 0dBm (遥控器)
- **CRC**: 16 位
- **地址**: 5 字节 `{0x0A, 0x01, 0x06, 0x1E, 0x01}`

### 帧格式 (17 字节)

```
字节偏移  内容                    说明
[0]      0x73 ('s')             帧头校验1
[1]      0x67 ('g')             帧头校验2
[2]      0x67 ('g')             帧头校验3
[3-4]    thr (大端序)           油门 0-1000
[5-6]    yaw (大端序)           偏航 0-1000
[7-8]    pit (大端序)           俯仰 0-1000
[9-10]   rol (大端序)           横滚 0-1000
[11]     shutdown               0=正常, 1=关机命令 (脉冲)
[12]     fix_height             0=无切换, 1=切换定高模式 (脉冲)
[13-16]  sum (32位大端序)       字节[0-12]累加和
```

### 通信流程

1. 遥控器 (TX 主动方): 打包数据 → SI24R1 发送 → 等待回传 (500 次轮询) → 切回 RX
2. 飞控 (RX 被动方): RX 等待 → 收到数据 → TX 回传电池电压 → 切回 RX

## FreeRTOS 任务

### 飞控任务 (P01)

| 任务        | 优先级   | 堆栈      | 周期  | 功能                             |
| ----------- | -------- | --------- | ----- | -------------------------------- |
| power_task  | 4 (最高) | 128 words | 10s   | IP5305T 电源管理，接收通知关机   |
| flight_task | 3        | 128 words | 6ms   | MPU6050 采集→姿态解算→PID→电机   |
| com_task    | 4        | 128 words | 10ms  | 遥控数据接收、状态管理、电池回传 |
| led_task    | 1 (最低) | 128 words | 100ms | LED 状态指示 (连接/飞行状态)     |

### 遥控器任务 (P02)

| 任务       | 优先级 | 堆栈      | 周期  | 功能                   |
| ---------- | ------ | --------- | ----- | ---------------------- |
| power_task | 4      | 128 words | 10s   | IP5305T 电源维持       |
| com_task   | 3      | 128 words | 10ms  | 数据打包发送与回传接收 |
| key_task   | 2      | 128 words | 20ms  | 按键检测与事件处理     |
| joy_task   | 2      | 128 words | 20ms  | 摇杆 ADC 采集与校准    |
| oled_task  | 1      | 128 words | 100ms | OLED 显示刷新          |

## 关键配置

### FreeRTOS

- **堆大小**: 12KB (`configTOTAL_HEAP_SIZE = 12 * 1024`)
- **Tick 频率**: 1000Hz (1ms)
- **优先级数**: 5 级
- **堆管理算法**: heap_4 (支持碎片合并)

### 飞控参数

- **MPU6050 量程**: 陀螺仪 ±2000°/s, 加速度 ±2g
- **采样率**: 500Hz
- **低通滤波**: 184Hz
- **PID 周期**: 6ms
- **Mahony 滤波**: Kp=0.8, Ki=0.0003

### 电机映射

- TIM3_CH1: 左上电机
- TIM4_CH4: 右上电机
- TIM2_CH2: 左下电机
- TIM1_CH3: 右下电机
- PWM 范围: 0-1000

## 开发规范

### 代码风格

- **注释语言**: 中文
- **函数命名**: `模块_动作` 格式 (如 `Int_motor_set_speed`)
- **结构体**: `模块_Struct` 格式 (如 `LED_Struct`, `Motor_Struct`)
- **枚举**: `状态_值` 格式 (如 `REMOTE_CONNECTED`, `IDLE`)
- **全局变量**: 直接使用小写下划线 (如 `remote_state`, `flight_state`)

### 文件组织

- CubeMX 生成的代码放在 `Core/` 目录，**不要手动修改**
- 自定义代码全部放在 `MDK-ARM/` 下的三层目录中
- 每个模块有独立的 `.h/.c` 文件对

### 调试

- 使用 `debug_printf()` 输出调试信息 (自动附带文件名和行号)
- 通过 UART2 输出 (飞控) 或 UART1 输出 (遥控器)
- 可通过 `DEBUG_LOG_ENABLE` 宏全局开关调试输出

### 状态管理

- **遥控连接状态**: `Remote_State` 枚举 (CONNECTED/DISCONNECTED)
- **飞行状态**: `Flight_State` 枚举 (IDLE/NORMAL/FIX_HEIGHT/FAIL)
- **解锁状态机**: FREE→MAX→LEAVE_MAX→MIN→UNLOCK (油门杆操作序列)

## 注意事项

1. **CubeMX 重新生成**: 如果修改了 `.ioc` 文件并重新生成代码，`Core/` 目录会被覆盖，确保自定义代码在 `MDK-ARM/` 中
2. **内存限制**: STM32F103C8T6 只有 20KB SRAM，FreeRTOS 堆占 12KB，任务堆栈每个 512 字节
3. **MPU6050 校准**: 启动时必须静止，自动执行 100 次偏移校准
4. **脉冲信号**: `shutdown` 和 `fix_height` 是脉冲信号，读取后必须清零
5. **半双工通信**: SI24R1 不能同时收发，必须严格时分切换
6. **无命令行构建**: 必须使用 Keil MDK-ARM 编译，无法通过命令行构建

## 常见任务

### 添加新的 FreeRTOS 任务

1. 在 `App_freeRTOS_Task.c` 中定义任务函数、堆栈大小、优先级
2. 在 `App_freeRTOS_start()` 中调用 `xTaskCreate()` 创建任务
3. 注意总堆栈不能超过 12KB 限制

### 修改 PID 参数

1. 编辑 `MDK-ARM/common/Com_pid.h` 中的 PID 结构体初始化
2. 或在 `App_flight.c` 中直接修改 PID 增量计算

### 调试无线通信

1. 使用 `debug_printf()` 在飞控和遥控器两端输出收发数据
2. 检查 SI24R1 初始化时的芯片校验 (读回 TX_ADDR 对比)
3. 确认帧头 `'s','g','g'` 和累加和校验正确

### 添加新的传感器

1. 在 `MDK-ARM/interface/` 中创建新的 `Int_xxx.h/.c` 文件
2. 参考 `Int_mpu6050.c` 的 I2C 驱动模式
3. 在 CubeMX 中配置对应的外设 (I2C/SPI/GPIO)