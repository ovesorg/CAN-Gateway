# CAN-Gateway

GD32F107 (ARM Cortex-M3) IoT 网关固件。双路 CAN 桥接 + 独立物联网控制器。

## 项目定位

- **网关模式** — CAN0↔CAN1 帧转发，桥接 BMS 网络与整车控制器网络
- **独立 IoT 模式** — 不依赖外部 MCU，自带 GPRS+GPS+BLE+LCD+PAYG+OTA

## 产品线（编译时切换）

| 编译宏 | 产品 |
|---|---|
| `E_MOB48V_PROJECT` | 48V 电动车网关 |
| `P10KW_PROJECT` | 10kW 离网逆变器 |
| `UI1K_V13_PROJECT` | UI 1K V1.3 太阳能系统 |
| `UI1K_V2_PROJECT` | UI 1K V2 |
| `PUMP_PROJECT` / `DC_PUMP_SUPPORT` | 太阳能水泵 |
| `CAMP_PROJECT` / `BMS_CAMP_SUPPORT` | Camp 户外电源 |

## 硬件平台

- **MCU**: GD32F107 (ARM Cortex-M3)
- **GSM**: SIM800C (2G) 或 EC200U (4G, `MODULE_4G`)
- **BLE**: UART1 外挂蓝牙模块
- **GPS**: UART4，ATGM336H 或同类
- **显示**: LCD128x64 图形屏 或 段码 LCD
- **CAN**: CAN0（整车侧）+ CAN1（BMS 侧）
- **存储**: DS1302 RTC + AT24Cxx EEPROM
- **调试**: SWD + USB VCOM

## 目录结构

```
CAN-Gateway/
├── Core/Inc/          # 头文件（40个模块）
├── Core/Src/          # 源码（40个模块 + bms309子目录）
│   ├── can.c          # 双路CAN网关 + 总线恢复
│   ├── ble.c          # BLE UART 数据上报
│   ├── Gatt.c         # GATT 字段模型（ATT/CMD/STS/DTA/DIA）
│   ├── GsmCom.c       # GSM/MQTT AT 状态机
│   ├── mqtt.c         # MQTT 层（桩代码，逻辑在 GsmCom/AtCmd）
│   ├── AtCmd.c        # AT 指令调度 + MQTT 报文构造
│   ├── gps.c          # GPS NMEA 解析
│   ├── payg.c         # PAYG 哈希链 token 验证
│   ├── ota.c          # 固件 OTA
│   ├── Menu.c         # LCD 菜单系统
│   ├── Pump.c         # 水泵控制
│   ├── OffGrid.c      # 离网逆变器 Modbus 通信
│   ├── Camp.c         # Camp BMS 专用协议
│   ├── Sif.c          # 仪表盘驱动（单线 GPIO）
│   ├── coulom.c       # 库仑计 / BMS Modbus
│   └── bms309/        # Sinowealth BMS AFE 子系统（20个文件）
├── Firmware/           # GD32 标准外设库 + CMSIS + USB
├── Template/           # GD32 SDK 模板（main.c + 启动文件）
└── Utilities/          # GD32 评估板驱动 + LCD 字体 + FatFs
```

## 构建

依赖 GD32F10x SDK + Keil MDK。`Template/` 下的 `main.c` 是入口。通过 `gatt.h` 中的 `#ifdef` 宏选择产品。

## 关键资源

- **CAN TX FIFO**: 每条 CAN 总线 100 条
- **BLE 缓冲**: 128 字节
- **GATT JSON 缓冲**: 4 KB（`g_pub_json`）
- **OTA 起始地址**: `0x8000000 + 1024*148`

## 相关仓库

- `edge-bcu` — SIMCom 平台 IoT 控制器
- `Edge-AC-Charging-Point` — 充电桩控制器（同 DUCi-II 层级）
