# CAN 网关操作流程与逻辑流程文档

> 源文件：`Core/Src/can.c`、`Core/Inc/can.h`、`Template/gd32f10x_it.c`、`Template/main.c`
> 适用工程：CAN-Gateway（GD32F10x 平台，E_MOB48V_PROJECT + CAN_TRASMITER_SUPPORT）

---

## 一、系统概述

本 CAN 网关是一个双通道 CAN 中继设备，核心功能是将两路 CAN 总线上的报文进行透明转发，同时解析 BMS（电池管理系统）和 MCU/VCU（电机控制器）的关键数据，供 BLE/GATT 上报和充电控制使用。

**硬件拓扑：**

```
┌─────────────┐     CAN0      ┌───────────────────┐     CAN1      ┌─────────────┐
│  BMS 侧设备  │◄──────────────►│   CAN-Gateway     │◄─────────────►│  VCU/IOT 侧  │
│ (电池/充电桩) │    RX0中断      │   (GD32F10x)     │    RX1中断     │ (电机/逆变器) │
└─────────────┘                └───────┬───────────┘                └─────────────┘
                                       │ BLE UART
                                       ▼
                                ┌──────────────┐
                                │  手机 APP    │
                                └──────────────┘
```

- **CAN0**：连接 BMS 侧，中断 `CAN0_RX0_IRQHandler`，接收到的报文转发至 CAN1
- **CAN1**：连接 VCU/IOT 侧，中断 `CAN1_RX0_IRQHandler`，接收到的报文转发至 CAN0
- **BLE**：手机 APP 通过蓝牙下发参数（时间、功率、CAN 地址等）

---

## 二、核心数据结构

### 2.1 事件标志位

使用三个位域联合体 `g_CanMcuEvent`、`g_CanBmsEvent`、`g_CanBms1Event` 标记各路报文的更新事件，在解析时置位，在 `CanProc()` 主循环中处理并清除。

| 事件宏 | 位位置 | 含义 |
|--------|--------|------|
| `McuFaultEvent` | g_CanMcuEvent.B0 | MCU 故障信息更新 |
| `McuRunInforEvent` | g_CanMcuEvent.B1 | MCU 运行信息更新 |
| `McuPwrOutEvent` | g_CanMcuEvent.B2 | MCU 功率输出更新 |
| `McuSysInfor1Event` | g_CanMcuEvent.B3 | MCU 系统信息1更新 |
| `McuSysInfor2Event` | g_CanMcuEvent.B4 | MCU 系统信息2更新 |
| `McuCCSEvent` | g_CanMcuEvent.B5 | MCU 充电参数(CCS)更新 |
| `BmsRtChangEvent` | g_CanBmsEvent.B0 | BMS 充电参数更新 |
| `BmsRtState2Event` | g_CanBmsEvent.B1 | BMS 实时状态2更新 |
| `BmsRtState3Event` | g_CanBmsEvent.B2 | BMS 实时状态3更新 |
| `BmsCellVolt1~6Event` | g_CanBmsEvent.B3~B7, g_CanBms1Event.B0 | BMS 电芯电压1~6更新 |
| `BmsRtTempEvent` | g_CanBms1Event.B1 | BMS 温度更新 |
| `BmsSysInforEvent` | g_CanBms1Event.B2 | BMS 系统信息更新 |

### 2.2 CAN 报文数据结构

| 结构体 | CAN ID | 方向 | 说明 |
|--------|--------|------|------|
| `MCU_FAULT_TypeDef` | 0x01806E600 | BMS→网关 | MCU 故障状态（温度、故障位、档位等） |
| `MCU_RUNINFOR_TypeDef` | 0x01806E601 | BMS→网关 | MCU 运行信息（转速、电压、电流） |
| `MCU_POWEROUT_TypeDef` | 0x01806E602 | BMS→网关 | MCU 功率输出限制 |
| `MCU_SYSINFOR1_TypeDef` | 0x01806E502 | BMS→网关 | MCU 额定参数 |
| `MCU_SYSINFOR2_TypeDef` | 0x01806E503 | BMS→网关 | MCU 设置参数 |
| `MCUCCS_TypeDef` | 0x01806E640 | VCU→网关 | VCU 充电限制（电压/电流） |
| `BMSCCS_TypeDef` | 0x1806E5F4 | BMS→网关 | BMS 充电限制（电压/电流） |
| `BMS_RT_STATUS2_TypeDef` | 0x01806E611 | BMS→网关 | BMS 实时状态2（SOC/SOH/充放电电流） |
| `BMS_RT_STATUS3_TypeDef` | 0x01806E612 | BMS→网关 | BMS 实时状态3（容量/循环次数/充电检测） |
| `BMS_CELLVOLT1~6_TypeDef` | 0x01806E613~618 | BMS→网关 | BMS 电芯电压1~24（mV） |
| `BMS_RTTEMP_TypeDef` | 0x01806E620 | BMS→网关 | BMS 温度1~8（偏移+40℃） |
| `BMS_SYSINFOR_TypeDef` | 0x01806E516 | BMS→网关 | BMS 额定容量/电压 |
| `HM7280_CCS_DEVID_TypeDef` | 0x18FF50E7/E8 | 设备→网关 | 设备序列号高/低字节 |

### 2.3 CAN 发送状态

`CAN_TXSTATE_TypeDef g_CanTransmitState` 管理发送 FIFO 缓冲区和定时器：

- `can0_count` / `can1_count`：CAN0/CAN1 发送 FIFO 中的待发报文数
- `t10ms` / `t100ms` ... `t5000ms`：各周期定时基准

### 2.4 总线恢复状态

`CAN_RECOVERY_TypeDef g_CanRecoverState[2]` 管理 CAN0/CAN1 的错误恢复：

- `busoff_state`：BUSOFF 状态（NONE/QUICK/SLOW）
- `miss_ack_state`：ACK 丢失状态（NONE/TIMEOUT/RECOVERY）
- `tx_disable`：发送禁用标志
- `busoff_counter`：BUSOFF 连续计数

---

## 三、操作流程（Operation Flow）

### 3.1 系统初始化流程

```
main()
  ├── EEpInit()              — EEPROM 初始化，加载 g_UserSet 持久化参数
  ├── TimerInit()            — 定时器初始化
  ├── CanInit()              — CAN 控制器硬件初始化（波特率、滤波器）
  ├── CanRamInit()           — CAN 软件变量清零，设置 CAN 地址
  │     ├── 清零所有 g_Mcu*/g_Bms* 结构体
  │     ├── 清零事件标志 g_CanMcuEvent/g_CanBmsEvent/g_CanBms1Event
  │     ├── 清零发送状态 g_CanTransmitState
  │     ├── 清零恢复状态 g_CanRecoverState[0/1]
  │     └── g_CanAddr = g_UserSet.canid_cnt + 1  （CAN 转发地址）
  ├── GpsInit() / BleUartInit() / GattInit() / PaygInit()
  └── 进入主循环 while(1)
```

**CAN 中断初始化（在 CanInit 中完成）：**
- `nvic_irq_enable(CAN0_RX0_IRQn, 0, 0)` — CAN0 接收中断，优先级最高
- `nvic_irq_enable(CAN1_RX0_IRQn, 1, 1)` — CAN1 接收中断

### 3.2 CAN 接收中断流程

当 CAN 控制器收到报文时，触发硬件中断：

```
┌──────────────────────────────────────────────────────────────────┐
│  CAN0_RX0_IRQHandler()  (BMS 侧 → 网关)                          │
│  ┌─────────────────────────────────────────────────────┐          │
│  │ can_message_receive(CAN0, CAN_FIFO0, &receive_msg)  │          │
│  │         │                                           │          │
│  │    ┌─────▼──────────────────────────┐               │          │
│  │    │ CAN_TRASMITER_SUPPORT 已定义？  │               │          │
│  │    └─────┬──────────┬───────────────┘               │          │
│  │      是  │          否                               │          │
│  │    ┌─────▼─────┐    ┌──▼────────────────────────┐   │          │
│  │    │Can1RxProc │    │按 ID 过滤后调用:            │   │          │
│  │    │(&receive)│    │ CanMcuParse(id,data,len)   │   │          │
│  │    │           │    │ CanBmsParse(id,data,len)   │   │          │
│  │    └───────────┘    └────────────────────────────┘   │          │
│  └─────────────────────────────────────────────────────┘          │
└──────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────┐
│  CAN1_RX0_IRQHandler()  (VCU/IOT 侧 → 网关)                      │
│  ┌─────────────────────────────────────────────────────┐          │
│  │ can_message_receive(CAN1, CAN_FIFO0, &receive1_msg) │          │
│  │         │                                           │          │
│  │    ┌─────▼──────────────────────────┐               │          │
│  │    │ CAN_TRASMITER_SUPPORT 已定义？  │               │          │
│  │    └─────┬──────────┬───────────────┘               │          │
│  │      是  │          否                               │          │
│  │    ┌─────▼─────┐    ┌──▼────────────────────────┐   │          │
│  │    │Can0RxProc │    │ (无处理)                   │   │          │
│  │    │(&receive1)│    │                            │   │          │
│  │    └───────────┘    └────────────────────────────┘   │          │
│  └─────────────────────────────────────────────────────┘          │
└──────────────────────────────────────────────────────────────────┘
```

**转发机制说明（CAN_TRASMITER_SUPPORT 模式）：**

CAN0 收到 BMS 侧报文后，调用 `Can1RxProc()` 将报文存入 `g_can1TxMessage[]` FIFO，等待转发到 CAN1（VCU 侧）；CAN1 收到 VCU 侧报文后，调用 `Can0RxProc()` 将报文存入 `g_can0TxMessage[]` FIFO，等待转发到 CAN0（BMS 侧）。

转发时修改 CAN ID 的 `msg_addr` 字段为 `g_CanAddr`，并置 `ms` 位为 1，实现地址标记。

同时，特定 ID 的报文会同步调用 `CanBmsParse()` / `CanMcuParse()` 进行数据解析：
- CAN0 侧：`0x1806E5F4`（BMS 充电参数）、`0x1806E612`（BMS 状态3）
- CAN0 侧直接解析：`0x1806E640`（MCU CCS）

### 3.3 主循环处理流程

`CanProc()` 在 `while(1)` 主循环中被持续调用，执行以下处理：

```
CanProc()
  │
  ├── 1. 总线恢复处理
  │     ├── CanRecoveryProc(0, CAN0)  — 检查 CAN0 错误状态
  │     └── CanRecoveryProc(1, CAN1)  — 检查 CAN1 错误状态
  │
  ├── 2. 更新 CAN 地址
  │     └── g_CanAddr = g_UserSet.canid_cnt
  │
  ├── 3. 睡眠定时器管理
  │     └── 若两路 CAN 均未禁用发送 → TimerSet(TIMER_SLEEP, SLEEP_PRIOD)
  │
  ├── 4. 10ms 周期任务（FIFO 转发 + CCS 合成）
  │     ├── 若 can0_count > 0:
  │     │     └── Can0Transmit(&g_can0TxMessage[0]) → CanFifoProc() → can0_count--
  │     ├── 若 can1_count > 0:
  │     │     └── Can1Transmit(&g_can1TxMessage[0]) → CanFifoProc() → can1_count--
  │     ├── LED 状态复位
  │     └── 每 150 次（~1.5s）合成 CCS 充电参数报文并发往 CAN0
  │           ├── 比较 VCU 电流 vs BMS 电流，取较小值下发
  │           └── 组装 0x1806E640 报文 → can_message_transmit(CAN0, ...)
  │
  ├── 5. PPID 序列号下发（pag_watchwdg == 0 时）
  │     ├── 组装 0x1806E55E（序列号高字节）→ 发 CAN0
  │     ├── 组装 0x1806E55F（序列号低字节）→ 发 CAN0
  │     └── 连续发送 5 次后置 pag_watchwdg = 0xAA（停止）
  │
  ├── 6. 5000ms 周期任务（心跳 + 设备ID更新）
  │     ├── 组装 0x1806E641 心跳报文（含 PAYG 状态）→ 发 CAN0
  │     └── CanHm7280BuildSerialPayload() — 读取 PPID，比较设备ID是否变化
  │           ├── ID 变化 → g_Hm7280SerialDirty = TRUE
  │           └── ID 未变 → g_Hm7280SerialDirty = FALSE
  │
  ├── 7. 充电事件处理
  │     ├── BmsRtChangEvent → 解析 BMS 充电限制电压/电流
  │     └── McuCCSEvent → 解析 VCU 充电限制电压/电流
  │
  ├── 8. 充电超时判断
  │     └── 若充电时间超过 g_UserSet.time（分钟）→ CcsEnergyLimitReached = 1
  │
  ├── 9. 剩余容量读取
  │     └── bat_rcap = BMS 剩余容量（用于能量限制判断）
  │
  └── 10. GATT 数据上报（事件驱动）
        ├── McuFaultEvent → 上报温度、档位、刹车、故障状态
        ├── McuRunInforEvent → 上报转速、车速、电压、电流
        ├── McuSysInfor1Event → 上报额定最大电流
        ├── McuSysInfor2Event → 上报设置的最大速度/电流
        ├── BmsRtState2Event → 上报 SOC
        ├── BmsRtState3Event → 上报循环次数、满充容量、剩余容量、已充能量
        ├── BmsCellVolt1~6Event → 上报电芯电压 1~24
        └── BmsRtTempEvent → 上报温度 1~6（减 40℃ 偏移）
```

---

## 四、逻辑流程（Logic Flow）

### 4.1 CAN 报文转发逻辑

```
                    ┌─────────────────────────────────────────────────┐
                    │              CAN 报文透明转发                     │
                    └─────────────────────────────────────────────────┘

  BMS 侧 ──CAN0──► [CAN0_RX0_IRQ] ──► Can1RxProc()
                                          │
                                    ┌─────▼─────┐
                                    │ ID 过滤?   │
                                    │ 0x1806E*  │
                                    └─────┬─────┘
                                          │ 是
                              ┌───────────▼───────────────┐
                              │ 存入 g_can1TxMessage[]     │
                              │ (修改 msg_addr = g_CanAddr) │
                              │ can1_count++               │
                              └───────────┬───────────────┘
                                          │
                              ┌───────────▼───────────────┐
                              │ 特定ID同步解析:             │
                              │ 0x1806E5F4 → CanBmsParse() │
                              │ 0x1806E612 → CanBmsParse() │
                              └───────────────────────────┘
                                          │
  主循环 10ms ──► Can1Transmit() ──► CAN1 ──► VCU/IOT 侧

  VCU/IOT 侧 ──CAN1──► [CAN1_RX0_IRQ] ──► Can0RxProc()
                                          │
                              ┌───────────▼───────────────┐
                              │ 存入 g_can0TxMessage[]     │
                              │ (修改 msg_addr = g_CanAddr) │
                              │ can0_count++               │
                              └───────────┬───────────────┘
                                          │
                              ┌───────────▼───────────────┐
                              │ 特定ID直接解析:             │
                              │ 0x1806E640 → CanMcuParse() │
                              └───────────────────────────┘
                                          │
  主循环 10ms ──► Can0Transmit() ──► CAN0 ──► BMS 侧
```

### 4.2 CAN 报文解析逻辑

**CanMcuParse() — MCU/VCU 报文解析（来自 VCU 侧）：**

```
CanMcuParse(id, data, len)
  │
  ├── 0x01806E502 → g_McuSysInfor1 ← data  → McuSysInfor1Event = TRUE
  ├── 0x01806E503 → g_McuSysInfor2 ← data  → McuSysInfor2Event = TRUE
  ├── 0x01806E600 → g_McuFaultInfor ← data → McuFaultEvent = TRUE
  ├── 0x01806E601 → g_McuRunInfor  ← data  → McuRunInforEvent = TRUE
  ├── 0x01806E602 → g_McuPowerOut ← data  → McuPwrOutEvent = TRUE
  └── 0x01806E640 → g_McuCCSOut   ← data  → McuCCSEvent = TRUE
```

**CanBmsParse() — BMS 报文解析（来自 BMS 侧）：**

```
CanBmsParse(id, data, len)
  │
  ├── 0x01806E516 → g_BmsSysInfor    ← data → BmsSysInforEvent = TRUE
  ├── 0x01806E611 → g_BmsRtStatus2   ← data → BmsRtState2Event = TRUE
  ├── 0x01806E612 → g_BmsRtStatus3   ← data → BmsRtState3Event = TRUE
  ├── 0x01806E613 → g_BmsCellVolt1   ← data → BmsCellVolt1Event = TRUE
  ├── 0x01806E614 → g_BmsCellVolt2   ← data → BmsCellVolt2Event = TRUE
  ├── 0x01806E615 → g_BmsCellVolt3   ← data → BmsCellVolt3Event = TRUE
  ├── 0x01806E616 → g_BmsCellVolt4   ← data → BmsCellVolt4Event = TRUE
  ├── 0x01806E617 → g_BmsCellVolt5   ← data → BmsCellVolt5Event = TRUE
  ├── 0x01806E618 → g_BmsCellVolt6   ← data → BmsCellVolt6Event = TRUE
  ├── 0x01806E620 → g_BmsRtTemp      ← data → BmsRtTempEvent = TRUE
  ├── 0x1806E5F4 → g_Bms_Charge      ← data → BmsRtChangEvent = TRUE
  ├── 0x18FF50E7 → g_Devid.DevidH    ← data (设备序列号高)
  └── 0x18FF50E8 → g_Devid.DevidL    ← data (设备序列号低)
```

### 4.3 充电控制逻辑

```
                    ┌─────────────────────────────────┐
                    │         充电参数下发流程          │
                    └─────────────────────────────────┘

  手机APP ──BLE──► BleCmdProc()
                     │
          ┌──────────┼──────────────────────┐
          │          │                      │
    BLE_CMD_SWCH   BLE_CMD_RAML         BLE_CMD_ADDR
    (下发时间)      (下发功率)            (设置CAN地址)
          │          │                      │
          ▼          ▼                      ▼
  g_UserSet.time  g_UserSet.lowbat    g_UserSet.canid_cnt
  set_CcsEnergy   set_CcsEnergy_mWh  EEpUpdateEnable()
  Limittime()     (g_UserSet.lowbat)
  (HAL_GetTick())       │
          │              │
          ▼              ▼
  ┌───────────────────────────────────────┐
  │  CanProc() 主循环中持续判断:            │
  │                                       │
  │  1. 充电超时判断:                       │
  │     g_CcsEnergyLimittime_count =      │
  │         g_UserSet.time * 1000 * 60    │
  │     若 HAL_GetTick() - 开始时间 > 超时  │
  │     → CcsEnergyLimitReached = 1       │
  │     → 停止充电                         │
  │                                       │
  │  2. CCS 充电参数合成（每~1.5s）:        │
  │     从 BMS 获取充电限制电压/电流         │
  │     从 VCU 获取充电限制电压/电流         │
  │     比较 VCU 电流 vs BMS 电流           │
  │     取较小值作为实际充电电流             │
  │     组装 0x1806E640 报文 → 发 CAN0     │
  └───────────────────────────────────────┘
```

### 4.4 总线错误恢复逻辑

`CanRecoveryProc()` 处理两类 CAN 总线错误：

```
CanRecoveryProc(canindex, candev)
  │
  ├── 1. BUSOFF 恢复（总线关闭）
  │     │
  │     ├── 检测 CAN_ERR_BOERR 标志
  │     │
  │     ├── 若 BUSOFF 状态 ≠ NONE:
  │     │     └── 超时后检查错误是否清除
  │     │         ├── 已清除 → tx_disable = FALSE, state = NONE
  │     │         └── 未清除 → 继续等待
  │     │
  │     └── 若检测到 BOERR 且超时:
  │           ├── 清除错误标志
  │           ├── INRQ 请求 → 退出初始化（软复位）
  │           ├── busoff_counter++
  │           ├── 若 counter >= 5:
  │           │     └── 慢恢复 (1000ms)
  │           └── 若 counter < 5:
  │                 └── 快恢复 (100ms)
  │
  └── 2. MISS_ACK 恢复（ACK 丢失）
        │
        ├── 检测错误码 0x30（无应答）
        │
        ├── 状态机:
        │     ├── MISS_ACK_NONE:
        │     │     └── 记录时间 → TIMEOUT
  │     ├── MISS_ACK_TIMEOUT:
        │     │     └── 超过 100ms → 禁止发送 → RECOVERY
        │     └── MISS_ACK_RECOVERY:
        │           └── 超过 150ms → 恢复发送 → TIMEOUT
        │
        └── 若错误消失:
              └── miss_ack_state = NONE, tx_disable = FALSE
```

### 4.5 PPID / 设备序列号管理逻辑

```
CanHm7280BuildSerialPayload()
  │
  ├── 从 GATT 读取 PPID (GattGetPpid)
  ├── 拷贝到 g_Hm7280Serial[14]
  ├── 分割为:
  │     ├── g_Hm7280IotSerialHigh[8] (前6字节)
  │     └── g_Hm7280IotSerialLow[8]  (后8字节)
  │
  └── 在 CanProc() 5000ms 周期中:
        ├── 缓存上次 DevidH/DevidL
        ├── 重新调用 BuildSerialPayload()
        ├── 比较缓存 vs 当前:
        │     ├── 不同 → g_Hm7280SerialDirty = TRUE (需重新下发)
        │     └── 相同 → g_Hm7280SerialDirty = FALSE
        │
        └── pag_watchwdg == 0 时:
              ├── 发送 0x1806E55E (SerialHigh) → CAN0
              └── 发送 0x1806E55F (SerialLow) → CAN0
              └── 连续5次后 pag_watchwdg = 0xAA (停止下发)
```

---

## 五、CAN ID 完整映射表

### 5.1 BMS → 网关（CAN0 接收）

| CAN ID | 数据结构 | 周期 | 说明 |
|--------|----------|------|------|
| 0x01806E516 | BMS_SYSINFOR_TypeDef | 事件 | 额定容量/电压 |
| 0x01806E5F4 | BMSCCS_TypeDef | 事件 | 充电限制电压/电流 |
| 0x01806E611 | BMS_RT_STATUS2_TypeDef | 事件 | SOC/SOH/最大充放电电流 |
| 0x01806E612 | BMS_RT_STATUS3_TypeDef | 事件 | 容量/循环次数/充电检测 |
| 0x01806E613~618 | BMS_CELLVOLT1~6 | 事件 | 电芯电压1~24 (mV) |
| 0x01806E620 | BMS_RTTEMP_TypeDef | 事件 | 温度1~8 (偏移+40) |

### 5.2 VCU/IOT → 网关（CAN1 接收）

| CAN ID | 数据结构 | 说明 |
|--------|----------|------|
| 0x01806E502 | MCU_SYSINFOR1_TypeDef | 额定速度/电压/电流/油门范围 |
| 0x01806E503 | MCU_SYSINFOR2_TypeDef | 设置参数（最大速度/电流/欠压等） |
| 0x01806E600 | MCU_FAULT_TypeDef | 故障状态/温度/档位/刹车 |
| 0x01806E601 | MCU_RUNINFOR_TypeDef | 运行信息（转速/车速/电压/电流） |
| 0x01806E602 | MCU_POWEROUT_TypeDef | 功率输出限制 |
| 0x01806E640 | MCUCCS_TypeDef | VCU 充电限制参数 |

### 5.3 网关 → CAN0（主动发送）

| CAN ID | 周期 | 说明 |
|--------|------|------|
| 0x1806E55E | 事件(PID下发) | 设备序列号高字节 |
| 0x1806E55F | 事件(PID下发) | 设备序列号低字节 |
| 0x1806E640 | ~1.5s | CCS 合成充电参数（取BMS/VCU较小电流） |
| 0x1806E641 | 5s | 心跳报文（含PAYG看门狗状态） |
| 0x18FF50E5 | 事件 | 看门狗告警状态（代码中注释，未启用） |

---

## 六、BLE 下发参数与 CAN 的交互

| BLE 命令 | 功能 | 对 CAN 的影响 |
|----------|------|---------------|
| `BLE_CMD_SWCH` | 下发充电时间 | `set_CcsEnergyLimittime(HAL_GetTick())` — 启动充电计时 |
| `BLE_CMD_RAML` | 下发功率限制 | `set_CcsEnergy_mWh(g_UserSet.lowbat)` — 设置能量目标 |
| `BLE_CMD_ADDR` | 设置 CAN 地址 | `g_UserSet.canid_cnt` — 修改转发报文的 msg_addr |
| `BLE_CMD_HBFQ` | 心跳频率 | 影响 GATT 上报频率 |
| `BLE_CMD_RPTM` | 自动上报间隔 | 影响 GATT 自动上报周期 |

---

## 七、GATT 数据上报映射

`CanProc()` 中各事件触发后，解析的数据通过 `GattSetData()` 上报至 BLE：

| 事件源 | GATT 数据项 | 说明 |
|--------|-------------|------|
| McuFaultEvent | DTA_CTMP, DTA_MTPM | 控制器温度、电机温度 |
| | LIST_gearlevel, LIST_reverse, LIST_check | 档位、倒车、刹车 |
| | LIST_electrical, LIST_handle | 电气故障、手柄故障 |
| McuRunInforEvent | DTA_MTRD, DTA_TSPD | 电机转速、轮胎转速 |
| | DTA_RVLT, DTA_RCUR | 实时电压、实时电流 |
| McuSysInfor1Event | DTA_RMAX | 额定最大输入电流 |
| McuSysInfor2Event | DTA_CMXS, DTA_CMXC | 最大速度设置、最大电流设置 |
| BmsRtState2Event | DTA_RSOC | 电池SOC |
| BmsRtState3Event | DTA_ACYC, DTA_FCCP, DTA_RCAP, DTA_AENG | 循环次数、满充容量、剩余容量、已充能量 |
| BmsCellVolt1~6Event | DIA_CV01~CV23 | 电芯电压1~23 |
| BmsRtTempEvent | DIA_TEMP1~TEMP6 | 温度1~6 |

---

## 八、关键函数索引

| 函数名 | 文件:行 | 功能 |
|--------|---------|------|
| `CanRamInit()` | can.c:62 | 初始化所有 CAN 变量，设置 CAN 地址 |
| `CanRecoveryProc()` | can.c:109 | CAN 总线错误恢复（BUSOFF/ACK丢失） |
| `Can0RxProc()` | can.c:258 | CAN0 接收处理（存入 CAN1 发送 FIFO） |
| `Can1RxProc()` | can.c:301 | CAN1 接收处理（存入 CAN0 发送 FIFO） |
| `CanMcuParse()` | can.c:353 | MCU/VCU 报文解析 |
| `CanBmsParse()` | can.c:387 | BMS 报文解析 |
| `CanTransmit()` | can.c:452 | 同时向 CAN0/CAN1 发送报文 |
| `Can0Transmit()` | can.c:468 | 向 CAN0 发送（检查发送禁用） |
| `Can1Transmit()` | can.c:482 | 向 CAN1 发送（检查发送禁用） |
| `CanFifoProc()` | can.c:249 | 发送 FIFO 前移（弹出队首） |
| `CanProc()` | can.c:526 | 主循环处理（恢复/FIFO转发/CCS合成/事件处理） |
| `CanHm7280BuildSerialPayload()` | can.c:220 | 构建 PPID 序列号载荷 |
| `set_CcsEnergyLimittime()` | can.c:512 | 设置充电时间限制 |
| `set_CcsEnergy_mWh()` | can.c:519 | 设置充电能量目标 |
| `CAN0_RX0_IRQHandler()` | gd32f10x_it.c:299 | CAN0 接收中断（BMS侧） |
| `CAN1_RX0_IRQHandler()` | gd32f10x_it.c:346 | CAN1 接收中断（VCU侧） |

---

*文档基于 CAN-Gateway 仓库 main 分支代码生成，对应 commit: 首次版本*
