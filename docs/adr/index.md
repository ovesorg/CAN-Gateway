# SPACE — CAN-Gateway 架构规格

基线来源：本仓库源码（无原始设计文档）  
分析日期：2026-08-08  

---

## 1. 架构总览

```
GSM Modem (UART2)
  → AtCmd.c → GsmCom.c (MQTT 状态机)
       ↓
BLE 模块 (UART1)
  → ble.c → Gatt.c (中心状态)
       ↓
CAN0 (整车 VCU) ←→ can.c ←→ CAN1 (BMS)
       ↓
GPS (UART4) → gps.c → Gatt.c
LCD (GPIO) → Menu.c ← Gatt.c (读取显示)
水泵 (UART3/RS-485) → Pump.c → Gatt.c
逆变器 (UART3/Modbus) → OffGrid.c → Gatt.c
库仑计 (UART3/Modbus) → coulom.c → Gatt.c
Camp BMS (UART3/UART5) → Camp.c → Gatt.c
仪表盘 (GPIO 单线) → Sif.c ← can.c
PAYG (RTC+EEPROM+SHA-1) → payg.c → Gatt.c
```

Gatt.c 是所有模块的唯一数据中心。模块只通过 `GattSetData()` / `GattGetData()` 读写 GATT，不直接访问裸内存。

---

## 2. 模块清单与职责

### 2.1 can.c — 双路 CAN 网关（1090 行）

**职责**：CAN0↔CAN1 帧转发 + BMS/MCU 数据解析 + 总线故障恢复

**解析的 CAN ID**：

| CAN 总线 | ID | 数据 |
|---|---|---|
| CAN0 (VCU) | `0x01806E600` | MCU 故障码、档位、刹车、温度 |
| CAN0 | `0x01806E601` | 电机转速、轮胎转速、实时电压电流 |
| CAN0 | `0x01806E602` | 放电/EBS 电流限制 |
| CAN0 | `0x01806E502` | 额定转速/电压/电流 |
| CAN0 | `0x01806E503` | 最大转速/电流/EBS、欠压 |
| CAN1 (BMS) | `0x01806E611` | SOC、SOH、最大充放电 |
| CAN1 | `0x01806E612` | 满充/剩余容量、循环次数 |
| CAN1 | `0x01806E613-618` | 电芯电压 1-24（每帧 4 个） |
| CAN1 | `0x01806E620` | 8 路温度传感器 |
| CAN1 | `0x01806E516` | 额定容量/电压 |
| CAN1 | `0x1806E5F4` | BMS 充电限制 |
| 双向 | `0x1806E640` | CCS 充电协商（SIMCom 生成） |

**总线恢复**：三级故障处理
- Bus-Off: 快恢复 100ms（<5次）→ 慢恢复 1000ms（≥5次）
- Miss-Ack (TEC≥96): 100ms 超时 → 禁用 TX 150ms → 恢复
- 恢复期间 `tx_disable` 门控所有发送

**CCS 充电协商**（`E_MOB48V`）：每 1.5s 发送 `0x1806E640`，取 BMS/MCU 双方电流限制中较小值，BMS 电压始终使用 BMS 值。

### 2.2 ble.c — BLE UART 数据上报（2118 行）

**职责**：通过 USART1 与外部蓝牙模块通信，上报 GATT 字段数据

**板内帧格式**：
```
AA | L | opcode | payload_len | payload[0..N] | CRC_LO | CRC_HI | 5A 5B
```
- CRC-16 覆盖 L+opcode+payload_len+payload
- 每 8 字节发送，间隔 2ms

**运行模式**：

| 模式 | 条件 | 行为 |
|---|---|---|
| 从机（Slave） | `BLE_ENABLE` 默认 | 接收蓝牙模块指令，轮询上报所有 GATT 字段 |
| 主机（Master） | `BLE_MASTER_ENABLE` + `LCD128X64` | 轮询从机 BLE 设备，读取对方 GATT 数据 |

**Token 处理**：主机模式 — 收到 PUBK token 后经 BLE 转发从机验证，最多轮询 10 秒

### 2.3 Gatt.c — GATT 字段模型（2074 行）

**职责**：所有数据的中心存储和 JSON 序列化

**字段分组**：

| Group | 含义 | 字段数（E_MOB48V） | 字段数（P10KW） |
|---|---|---|---|
| ATT | 设备属性 | 5 | 5 |
| CMD | 可写命令 | 10 | 35+ |
| STS | 运行状态 | 14 | 14 |
| DTA | 遥测数据 | 19-32* | 38+ |
| DIA | 诊断 | 16-30* | 35 |

(*取决于 BMS 类型)

**存储**：所有字段共享 `g_GattMem[MEM_GATT_SIZE]` 单字节数组，地址由 `MEM_ADDR_*` 枚举链计算

**数据类型**：`uint16`、`int16`、`string`、`float_string`、`uint32`

**更新追踪**：`GATT_UPDATE_TypeDef` 记录每个字段 dirty 状态，驱动 BLE 增量上报

### 2.4 GsmCom.c — MQTT 状态机（949 行）

**职责**：GSM 拨号→注册→MQTT 连接→订阅→发布的完整生命周期

**4G 模式状态流**（`MODULE_4G`）：
```
IDLE → INIT → AT → CPIN → CSQ → CICCID → CREG → COPS_Q
→ CGDCONT → CGACT → MQTTSTART → MQTTACCQ → MQTTCONNECT
→ MQTTSUB → MQTTSUBACK → [PUBLISH] → CHK_REQUEST → END
```

**双 Broker 切换**：Broker[0] 断开自动切到 Broker[1]

**SMS 子系统**：检测 `+CMTI` → 读取短信 → 解析 token/配置 → 回复确认

### 2.5 AtCmd.c — AT 指令引擎（2861 行）

**职责**：AT 指令表驱动调度 + MQTT 报文构造 + JSON tag 解析

**MQTT 主题**：
- 发布: `dt/V01/GPRSV2/{VCUA|OS5K}/{OPID}`
- 订阅: `cmd/V01/GPRSV2/+/+/{OPID}/#`

**MQTT 命令**：`/cmd/nbroker/`（设置 Broker）、`/cmd/napn/`（设置 APN）、`/cmd/gstw/`（休眠窗口）、`/cmd/code/`（输入 token）、`/cmd/mfrq/`（测量频率）、`/cmd/updt/`（上报模式）

### 2.6 payg.c — PAYG 付费（942 行）

**职责**：基于 SHA-1 哈希链的 token 验证 + 输出控制

**算法**：20 位数字 → 拆分为 14 字节 → SHA-1 → XOR 压缩 → 迭代哈希匹配 → 充值天数 = 迭代次数

**充值阶梯**：1096 天（自由模式）、1-1095 天（普通充值）、2192 天（清零）

**输出控制**：无剩余天数且非自由模式时，禁用 AC/DC/USB 输出

### 2.7 gps.c — GPS（395 行）

**职责**：NMEA 解析（`$GNRMC`、`$GNGGA`、`GNGLL`）

**写入 GATT**：经纬度（`DTA_SLAT/SLON`）、海拔（`DTA_SALT`）、速度（`DTA_SSPE`）、时间（`DTA_SSTM`）

**注意**：当前 `GpsProc()` 被 `#if 0` 禁用

### 2.8 ota.c — OTA 升级（234 行）

**起始地址**：`0x8000000 + 1024*148`（148KB 偏移）  
**页数**：100 页

**协议**：MQTT JSON — `{"ota":"upgrade"...}` → `{"ota":"firmware"...}`（base64 分片） → `{"ota":"complete"}`

### 2.9 辅助模块

| 模块 | 行数 | 职责 |
|---|---|---|
| Menu.c | 1320 | LCD 菜单（5 标签 GATT 浏览器 + 段码菜单 + 水泵显示） |
| Pump.c | 760 | 水泵 RS-485 控制（自定义协议 + Modbus）+ PAYG 时间控制 |
| OffGrid.c | 772 | 逆变器 Modbus RTU（107 寄存器轮询）+ 继电器控制 |
| Camp.c | 676 | Camp BMS 专用 UART 协议（`0xC5 0x6A 0x29` 帧头，CRC8） |
| Sif.c | 421 | 仪表盘单线 GPIO 驱动（12 字节帧，含速度/SOC/档位/电压） |
| coulom.c | 958 | 库仑计 Modbus + BQ40Z50 适配 + ADC 电压估算回退 |
| bms309/ | 4556 | Sinowealth AFE BMS（I2C 读写、保护、均衡、电量计、校准、EEPROM） |

---

## 3. 已经冻结的架构约束

- GATT 存储模型不更换 — `g_GattMem[]` 扁平数组保留
- BLE 帧格式不变 — `0xAA ... 0x5A5B`
- PAYG 哈希链算法不替换
- CAN ID 地址空间不重新分配
- 多产品通过编译宏选通，不引入运行时多态

---

## 4. DUCi 功能映射

### Di — 数据采集

| 数据源 | 通道 | 模块 |
|---|---|---|
| BMS (CAN1) | CAN | can.c → Gatt.c |
| VCU/电机 (CAN0) | CAN | can.c → Gatt.c |
| GPS | UART4 | gps.c → Gatt.c |
| GSM 信号 | UART2 AT | GsmCom.c → Gatt.c |
| 库仑计/逆变器/水泵/Camp | UART3 (Modbus/RS-485) | coulom/OffGrid/Pump/Camp → Gatt.c |
| 按键 | GPIO | key.c → Menu.c |
| 电池 AFE (bms309) | TWI/I2C | bms309/ → Gatt.c |
| MQTT 下行 | UART2 | AtCmd.c → GsmCom.c |

### Ui — 用户交互

| 通道 | 方向 | 模块 |
|---|---|---|
| BLE 上报 | UART1 发送 | ble.c |
| LCD 显示 | GPIO | Menu.c |
| 仪表盘 | GPIO 单线 | Sif.c |
| MQTT 遥测 | UART2 发送 | AtCmd.c → GsmCom.c |
| LED 指示 | GPIO | bms309/Led.c |

### Ci — 控制执行

| 控制 | 触发来源 | 模块 |
|---|---|---|
| 输出开关 | PAYG/远程 | payg.c → GPIO |
| 水泵启停 | PAYG/远程 | Pump.c → UART3 |
| CCS 充电限制 | BMS+VCU | can.c → CAN0 |
| OTA | MQTT | ota.c → Flash |
| 继电器 | PAYG | OffGrid.c → GPIO |
| 看门狗 | SDK | Template/ |
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

<!-- mkdocs-oves-template:reference-footnote:start -->
---
Small notes: progressive architecture decisions are tracked in [Architecture Decision Records](adr/index.md); stable lookup material lives under [Reference](reference/index.md).
<!-- mkdocs-oves-template:reference-footnote:end -->
