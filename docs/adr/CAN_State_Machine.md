# CAN 网关状态机文档

> 源文件：`Core/Src/can.c`、`Core/Inc/can.h`
> 覆盖分支：`main` + `feature/charge-flow-optimize`
> 代码平台：GD32F10x，E_MOB48V_PROJECT + CAN_TRASMITER_SUPPORT

---

## 概述

CAN 网关固件中存在 **7 个独立状态机**，涵盖总线错误恢复、充电控制、报文转发、设备标识管理等核心逻辑。以下逐一描述每个状态机的状态集、转移条件、守卫表达式和动作输出。

---

## 状态机 1：BUSOFF 恢复状态机

**所属函数：** `CanRecoveryProc()` (can.c:109)
**作用对象：** `g_CanRecoverState[canindex].busoff_state`，canindex=0 对应 CAN0，1 对应 CAN1
**触发源：** `CanProc()` 每轮调用

### 状态集

| 状态值 | 枚举名 | 含义 |
|--------|--------|------|
| 0 | `BUSOFF_NONE` | 正常运行，总线无错误 |
| 1 | `BUSOFF_QUICK` | 快恢复模式，等待 100ms (`T_BUSOFF_QUICK`) |
| 2 | `BUSOFF_SLOW` | 慢恢复模式，等待 1000ms (`T_BUSOFF_SLOW`) |

### 状态转移图

```
                    ┌─────────────────────────────────────────┐
                    │                                         │
                    ▼                                         │
              ┌───────────┐                                   │
              │ BUSOFF_NONE│ ◄─── 错误清除:                     │
              │ (正常运行) │     (ERR&0x70==0x00||0x30)         │
              └─────┬─────┘     && BOERR==RESET                │
                    │           && busoff_counter>0             │
                    │           → busoff_counter=0             │
                    │                                         │
          检测到 BOERR 且超时                                    │
          (HAL_GetTick - start_timer >= time_out)             │
                    │                                         │
                    ▼                                         │
              ┌───────────────────┐                           │
              │ busoff_counter < 5│                           │
              │  → BUSOFF_QUICK   │                           │
              │  (100ms 超时)     │                           │
              └───────┬──────────┘                           │
                      │ 下次检测到 BOERR 且超时                  │
                      │ busoff_counter++                       │
                      ▼                                       │
              ┌───────────────────┐                           │
              │ busoff_counter>=5  │                           │
              │  → BUSOFF_SLOW     │                           │
              │  (1000ms 超时)    │                           │
              └───────┬──────────┘                           │
                      │                                       │
                      │ 持续在 QUICK/SLOW 之间切换              │
                      │ (每次 BOERR 递增 counter)              │
                      └───────────────────────────────────────┘
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| BUSOFF_NONE | `CAN_ERR(dev) & BOERR` && 超时 | BUSOFF_QUICK (counter<5) 或 BUSOFF_SLOW (counter>=5) | `tx_disable=TRUE`; `CAN_TSTAT|=0x00808080`; `CAN_CTL|=INRQ` 然后 `CAN_CTL&=~INRQ`（软复位）; `busoff_counter++`; `busoff_start_timer=HAL_GetTick()` |
| BUSOFF_QUICK | 超时 (`>=100ms`) 且 `!(ERR&BOERR)` | BUSOFF_NONE | `tx_disable=FALSE`; `busoff_state=BUSOFF_NONE` |
| BUSOFF_QUICK | 超时且仍有 BOERR | BUSOFF_SLOW (若 counter>=5) 或继续 BUSOFF_QUICK | 递增 counter; 重置 start_timer |
| BUSOFF_SLOW | 超时 (`>=1000ms`) 且 `!(ERR&BOERR)` | BUSOFF_NONE | `tx_disable=FALSE`; `busoff_state=BUSOFF_NONE` |
| BUSOFF_SLOW | 超时且仍有 BOERR | BUSOFF_SLOW（counter 继续递增，上限 1000） | 重置 start_timer; counter++ |
| 任意 | `(ERR&0x70)==0x00 \|\| (ERR&0x70)==0x30) && BOERR==RESET && busoff_counter>0` | 保持当前状态 | `busoff_counter=0`（清除计数） |

### 关键设计点

- **快慢恢复切换**：连续 5 次 BUSOFF 后从 100ms 快恢复升级为 1000ms 慢恢复，防止总线持续故障时频繁重连
- **软复位操作**：每次恢复尝试通过 `INRQ` 置位/清零对 CAN 控制器进行软复位
- **计数器上限**：`busoff_counter` 最大 1000，防止溢出

---

## 状态机 2：MISS_ACK（ACK 丢失）恢复状态机

**所属函数：** `CanRecoveryProc()` (can.c:168-216)
**作用对象：** `g_CanRecoverState[canindex].miss_ack_state`
**触发条件：** `(CAN_ERR(dev) & 0x00000070) == 0x00000030` 且 `busoff_state == BUSOFF_NONE`

### 状态集

| 状态值 | 枚举名 | 含义 |
|--------|--------|------|
| 0 | `MISS_ACK_NONE` | 正常发送 |
| 1 | `MISS_ACK_TIMEOUT` | 发送超时等待，100ms (`TX_TIMEOUT`) |
| 2 | `MISS_ACK_RECOVERY` | 发送恢复等待，150ms (`TX_RECOVERY`) |

### 状态转移图

```
        ┌──────────────────────────────────────────────────┐
        │                                                  │
        ▼                                                  │
  ┌───────────────┐  错误码0x30出现                        │
  │ MISS_ACK_NONE  │ ──────────────────►  ┌────────────────┐
  │ (正常发送)     │  记录start_timer      │ MISS_ACK_TIMEOUT│
  └───────────────┘                       │ (禁止前等待     │
        ▲                                  │  100ms)        │
        │                                  └───────┬────────┘
        │                                          │
        │                                  超过100ms(TX_TIMEOUT)
        │                                          │
        │                                          ▼
        │                                  ┌────────────────┐
        │  超过150ms                        │MISS_ACK_RECOVERY│
        │  (TX_RECOVERY)                   │ (停止发送       │
        │  恢复发送                         │  等待150ms)     │
        │                                  └───────┬────────┘
        │                                          │
        └──────────────────────────────────────────┘
                         ↑                          │
                         │                    超过150ms
                         │                    重置start_timer
                         └──────────────────────┘
                         (循环：TIMEOUT ↔ RECOVERY)
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| MISS_ACK_NONE | ERR&0x70 == 0x30 且 BUSOFF_NONE | MISS_ACK_TIMEOUT | `miss_ack_start_timer=HAL_GetTick()` |
| MISS_ACK_TIMEOUT | `HAL_GetTick()-start_timer >= 100` | MISS_ACK_RECOVERY | `CAN_TSTAT|=0x00808080`; `tx_disable=TRUE`; `miss_ack_start_timer=HAL_GetTick()` |
| MISS_ACK_RECOVERY | `HAL_GetTick()-start_timer >= 150` | MISS_ACK_TIMEOUT | `tx_disable=FALSE`; `miss_ack_start_timer=HAL_GetTick()` |
| 任意 | 错误码不再是 0x30（正常） | MISS_ACK_NONE | `miss_ack_state=NONE`; `miss_ack_start_timer=HAL_GetTick()`; 若 BUSOFF_NONE 则 `tx_disable=FALSE` |

### 关键设计点

- **振荡循环**：TIMEOUT ↔ RECOVERY 循环在 ACK 持续丢失时不断交替——100ms 尝试发送，150ms 停止发送，反复直到 ACK 恢复
- **优先级**：仅在 `busoff_state == BUSOFF_NONE` 时才处理 MISS_ACK，BUSOFF 优先级更高
- **发送禁用**：RECOVERY 状态下 `tx_disable=TRUE`，`Can0Transmit()/Can1Transmit()` 会直接 return

---

## 状态机 3：充电会话状态机（断电恢复版）

**所属函数：** `CanProc()` (can.c:534, feature/charge-flow-optimize 分支)
**作用对象：** `g_UserSet.ccs_energy_limit_reached` + `g_UserSet.ccs_energy_mWh`（EEPROM 持久化）
**持久化：** 通过 `EEpUpdateEnable()` 保存到 EEPROM

### 状态集

| 状态 | 判别条件 | 含义 |
|------|----------|------|
| **IDLE** | `ccs_energy_mWh == 0` 或 `ccs_energy_limit_reached == 1` | 空闲，无充电任务 |
| **CHARGING** | `ccs_energy_mWh > 0` 且 `ccs_energy_limit_reached == 0` 且 `g_CcsEnergyLimittime > 0` | 充电进行中 |
| **LIMIT_REACHED** | `ccs_energy_limit_reached == 1` | 充电限制已到达，停止充电 |

### 状态转移图

```
    ┌─────────────────────────────────────────────────────────────┐
    │                                                             │
    │  手机APP BLE 下发                                            │
    │  ┌─────────────┐          ┌─────────────┐                    │
    │  │ BLE_CMD_SWCH │          │ BLE_CMD_RAML│                    │
    │  │ (下发时间)    │          │ (下发功率)   │                    │
    │  └──────┬──────┘          └──────┬──────┘                    │
    │         │                        │                            │
    │         ▼                        ▼                            │
    │  set_CcsEnergyLimittime()  set_CcsEnergy_mWh()                │
    │  (HAL_GetTick())           (g_UserSet.lowbat)                 │
    │  EEpUpdateEnable()         EEpUpdateEnable()                  │
    │         │                        │                            │
    │         └──────────┬─────────────┘                            │
    │                    ▼                                          │
    │             ┌──────────┐                                     │
    │             │  IDLE     │                                     │
    │             │ (空闲)    │                                     │
    │             └─────┬────┘                                     │
    │     扫码后 BLE 下发  │                                        │
    │                   │                                          │
    │                   ▼                                          │
    │             ┌──────────┐    超时: HAL_GetTick() -            │
    │             │ CHARGING │    g_CcsEnergyLimittime >           │
    │       ┌────►│ (充电中) │    g_UserSet.time*1000*60           │
    │       │     └─────┬────┘                                    │
    │       │           │              ┌──────────────────┐       │
    │       │           │              │ 能量到达:          │       │
    │       │           ├─────────────►│ bat_rcap - mWh >= │       │
    │       │           │              │ lowbat*0.9        │       │
    │       │           │              └────────┬─────────┘       │
    │       │           │                       │                   │
    │       │           ▼                       ▼                   │
    │       │     ┌──────────────┐     ┌──────────────┐            │
    │       │     │LIMIT_REACHED │◄────│ LIMIT_REACHED│            │
    │       │     │ (限制到达)    │     │ (能量限制)   │            │
    │       │     │ ccs_energy_  │     │ ccs_energy_  │            │
    │       │     │ mWh=0        │     │ mWh=0        │            │
    │       │     │ reached=1   │     │ reached=1   │            │
    │       │     │ EEpUpdate() │     │ EEpUpdate() │            │
    │       │     └──────┬─────┘     └──────────────┘            │
    │       │            │                                         │
    │       │  下次 BLE   │                                         │
    │       │  重新下发   │                                         │
    │       └────────────┘                                         │
    │                                                              │
    │  断电恢复:                                                    │
    │  上电时 charge_restored=0                                     │
    │  → 若 ccs_energy_limit_reached==0 && ccs_energy_mWh>0       │
    │    → g_CcsEnergyLimittime = HAL_GetTick() (重置计时器)        │
    │    → 回到 CHARGING 状态                                       │
    └─────────────────────────────────────────────────────────────┘
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| IDLE | BLE_CMD_SWCH 或 BLE_CMD_RAML 下发 | CHARGING | `set_CcsEnergyLimittime(HAL_GetTick())`; `set_CcsEnergy_mWh(lowbat)`; `ccs_energy_limit_reached=0`; `EEpUpdateEnable()` |
| CHARGING | `HAL_GetTick()-g_CcsEnergyLimittime > g_UserSet.time*1000*60` | LIMIT_REACHED | `ccs_energy_limit_reached=1`; `g_CcsEnergyLimittime=0`; `EEpUpdateEnable()` |
| CHARGING | `bat_rcap - ccs_energy_mWh >= lowbat*0.9` | LIMIT_REACHED | `ccs_energy_mWh=0`; `ccs_energy_limit_reached=1`; `EEpUpdateEnable()` |
| LIMIT_REACHED | BLE 重新下发 SWCH/RAML | CHARGING | 重置时间/能量; `ccs_energy_limit_reached=0`; `EEpUpdateEnable()` |
| 任意（上电） | `charge_restored==0` && `ccs_energy_limit_reached==0` && `ccs_energy_mWh>0` | CHARGING | `g_CcsEnergyLimittime=HAL_GetTick()`; `charge_restored=1` |

### CCS 合成输出（CHARGING 状态下每 ~2s）

```
每 200 次 10ms 循环（≈2s）:
  │
  ├── tempvcu_cur > 0 (VCU 有下发电流):
  │     ├── tempvcu_cur > tempbms_cur → 取 BMS 电流下发
  │     └── tempvcu_cur <= tempbms_cur → 取 VCU 电流下发
  │
  └── tempvcu_cur == 0 (VCU 无下发):
        ├── g_AC_ccsinput > 2100 → ccsvcu_cur += 100 (上限 bms_cur*0.95)
        ├── g_AC_ccsinput > 2000 → ccsvcu_cur = bms_cur * 0.5
        ├── g_AC_ccsinput > 1900 → ccsvcu_cur = bms_cur * 0.2
        └── g_AC_ccsinput <= 1900 → ccsvcu_cur = 0 (停止充电)
        
        若 ccs_energy_limit_reached == 1:
          → tempbms_cur = 0, ccsvcu_cur = 200 (限流至最小值)
        
        组装 0x1806E640 报文 → can_message_transmit(CAN0, ...)
```

### 关键设计点

- **EEPROM 持久化**：充电目标（`ccs_energy_mWh`）和限制标志（`ccs_energy_limit_reached`）写入 EEPROM，断电不丢失
- **断电恢复**：上电时检测 EEPROM 中的充电状态，若未完成则重置计时器继续充电
- **双重限制**：时间限制（分钟→毫秒换算）+ 能量限制（剩余容量减去目标容量 >= 阈值）
- **AC 输入电压三级掉电保护**：根据 `g_AC_ccsinput` 值动态调节充电电流，电压越低电流越小

---

## 状态机 4：PPID 设备序列号管理状态机

**所属函数：** `CanProc()` (can.c:730-810)
**作用对象：** `pag_watchwdg` + `g_Hm7280SerialDirty`
**周期：** 每 5000ms 做一次设备 ID 比较，pag_watchwdg==0 时连续发送

### 状态集

| 状态 | 判别条件 | 含义 |
|------|----------|------|
| **TRANSMIT_PPID** | `pag_watchwdg == 0` | 正在下发 PPID 序列号 |
| **IDLE** | `pag_watchwdg == 0xAA` | PPID 下发完成，等待设备 ID 变更 |
| **DIRTY_CHECK** | 每 5000ms 进入 | 比较设备 ID 是否变化 |

### 状态转移图

```
    上电/扫码触发
    pag_watchwdg = 0 (clear_pag_watchwdg)
         │
         ▼
  ┌──────────────┐     每次循环发送:
  │ TRANSMIT_PPID │     0x1806E55E (SerialHigh) → CAN0
  │ (下发中)      │     0x1806E55F (SerialLow)  → CAN0
  └──────┬───────┘     pag_count++
         │
         │ pag_count >= 5
         ▼
  ┌──────────────┐
  │    IDLE      │ ◄── pag_watchwdg = 0xAA
  │ (等待ID变更) │     pag_count = 0
  └──────┬───────┘
         │
         │ 每 5000ms:
         │ CanHm7280BuildSerialPayload()
         │ 比较 cache vs 当前 DevidH/DevidL
         │
         ├── ID 相同 → g_Hm7280SerialDirty = FALSE
         │           (继续发心跳 0x1806E641)
         │
         └── ID 不同 → g_Hm7280SerialDirty = TRUE
                       (停止心跳，等待外部触发 clear)
                       
     外部触发 clear_pag_watchwdg()
         │
         ▼
  回到 TRANSMIT_PPID
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| TRANSMIT_PPID | `pag_count >= 5` | IDLE | `pag_watchwdg=0xAA`; `pag_count=0` |
| IDLE | 每 5000ms 检测到设备 ID 变化 | DIRTY_CHECK | `g_Hm7280SerialDirty=TRUE`（停止心跳发送） |
| IDLE | 每 5000ms 设备 ID 未变 | IDLE（保持） | `g_Hm7280SerialDirty=FALSE`（继续心跳发送） |
| DIRTY_CHECK | 外部调用 `clear_pag_watchwdg()` | TRANSMIT_PPID | `pag_watchwdg=0` |

### 关键设计点

- **心跳与 PPID 互斥**：`g_Hm7280SerialDirty==FALSE` 时才发心跳 0x1806E641，TRUE 时停止心跳
- **5 次冗余发送**：PPID 连续发送 5 次确保可靠性，之后进入 IDLE
- **缓存比较**：5000ms 周期缓存 `DevidH/DevidL`，重新 `BuildSerialPayload()` 后比较

---

## 状态机 5：CAN FIFO 转发状态机

**所属函数：** `CanProc()` (can.c:570-598)
**作用对象：** `g_CanTransmitState.can0_count` / `can1_count`
**周期：** 每 10ms

### 状态集

| 状态 | 判别条件 | 含义 |
|------|----------|------|
| **FIFO_EMPTY** | `can0_count == 0` / `can1_count == 0` | 无待发报文 |
| **FIFO_HAS_MSG** | `can0_count > 0` / `can1_count > 0` | 有报文待转发 |

### 状态转移图

```
  CAN0_RX0_IRQ (BMS侧)                    CAN1_RX0_IRQ (VCU侧)
       │                                       │
       ▼                                       ▼
  Can1RxProc()                             Can0RxProc()
  存入 g_can1TxMessage[]                   存入 g_can0TxMessage[]
  can1_count++                             can0_count++
       │                                       │
       ▼                                       ▼
  ┌──────────────┐                        ┌──────────────┐
  │FIFO_HAS_MSG  │                        │FIFO_HAS_MSG  │
  │(CAN1待发)    │                        │(CAN0待发)    │
  └──────┬───────┘                        └──────┬───────┘
         │                                       │
    每10ms:                                 每10ms:
    Can1Transmit()                          Can0Transmit()
    CanFifoProc() (前移)                     CanFifoProc() (前移)
    can1_count--                             can0_count--
         │                                       │
         ▼                                       ▼
  ┌──────────────┐                        ┌──────────────┐
  │ FIFO_EMPTY   │ ◄── count 到 0         │ FIFO_EMPTY   │
  └──────────────┘                        └──────────────┘
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| FIFO_EMPTY | CAN RX 中断存入报文 | FIFO_HAS_MSG | `can_count++`; 修改 `msg_addr=g_CanAddr`; `ms|=0x01` |
| FIFO_HAS_MSG | 每 10ms 且 `tx_disable==FALSE` | FIFO_HAS_MSG（或 EMPTY 若 count 减到 0） | `CanXTransmit()`; `CanFifoProc()`; `can_count--` |
| FIFO_HAS_MSG | `tx_disable==TRUE` | FIFO_HAS_MSG（保持，不发送） | LogPrintf 输出 tx disable; 不发送 |

### 关键设计点

- **发送前置检查**：`Can0Transmit()` / `Can1Transmit()` 首先检查 `tx_disable` 和 `tx_efid==0x00`
- **FIFO 大小**：`CAN_TX_BUF_SIZE = 100`，溢出时丢弃新报文
- **地址标记**：转发时修改 CAN ID 的 `msg_addr` 字段为 `g_CanAddr`（来自 `g_UserSet.canid_cnt`）

---

## 状态机 6：AC 输入电压掉电检测状态机

**所属函数：** `CanProc()` CCS 合成分支 (can.c:642-660, feature/charge-flow-optimize)
**作用对象：** `g_AC_ccsinput`（从 BMS 实时状态1 获取）
**触发条件：** `tempvcu_cur == 0`（VCU 未下发充电电流）且每 ~2s 周期

### 状态集

| 状态 | 判别条件 | 输出电流(ccsvcu_cur) | 含义 |
|------|----------|----------------------|------|
| **NORMAL_CHARGE** | `g_AC_ccsinput > 2100` | `bms_cur*0.95`（递增+100） | 输入电压正常，接近满功率充电 |
| **DERATED_50** | `2000 < g_AC_ccsinput <= 2100` | `bms_cur * 0.5` | 轻度掉电，降功率至 50% |
| **DERATED_20** | `1900 < g_AC_ccsinput <= 2000` | `bms_cur * 0.2` | 中度掉电，降功率至 20% |
| **STOP** | `g_AC_ccsinput <= 1900` | `0` | 严重掉电，停止充电 |

### 状态转移图

```
  g_AC_ccsinput (来自 BmsRtState1Event 解析)
       │
       ▼
  ┌──────────────────────────────────────────┐
  │          AC 输入电压三级判断               │
  └──────────────────────────────────────────┘
       │
       ├── > 2100 → NORMAL_CHARGE
       │            ccsvcu_cur += 100
       │            上限: bms_cur * 0.95
       │
       ├── > 2000 → DERATED_50
       │            ccsvcu_cur = bms_cur * 0.5
       │
       ├── > 1900 → DERATED_20
       │            ccsvcu_cur = bms_cur * 0.2
       │
       └── <= 1900 → STOP
                     ccsvcu_cur = 0
       
  叠加: 若 ccs_energy_limit_reached == 1
       → tempbms_cur = 0
       → ccsvcu_cur = 200 (最小维持电流)
```

### 转移条件表

| 当前状态 | 条件 | 目标状态 | 动作 |
|----------|------|----------|------|
| NORMAL_CHARGE | `g_AC_ccsinput` 降至 `<=2100` | DERATED_50 | ccsvcu_cur = bms_cur*0.5 |
| DERATED_50 | `g_AC_ccsinput` 升至 `>2100` | NORMAL_CHARGE | ccsvcu_cur += 100 |
| DERATED_50 | `g_AC_ccsinput` 降至 `<=2000` | DERATED_20 | ccsvcu_cur = bms_cur*0.2 |
| DERATED_20 | `g_AC_ccsinput` 升至 `>2000` | DERATED_50 | ccsvcu_cur = bms_cur*0.5 |
| DERATED_20 | `g_AC_ccsinput` 降至 `<=1900` | STOP | ccsvcu_cur = 0 |
| STOP | `g_AC_ccsinput` 升至 `>1900` | DERATED_20 | ccsvcu_cur = bms_cur*0.2 |
| 任意 | `ccs_energy_limit_reached == 1` | (覆盖) | tempbms_cur=0; ccsvcu_cur=200 |

### 关键设计点

- **滞后无设计**：当前代码为即时阈值切换，无滞后区间（hysteresis），可能在阈值附近产生抖动
- **优先级覆盖**：`ccs_energy_limit_reached` 标志优先于 AC 电压判断
- **递增式恢复**：NORMAL_CHARGE 状态下每次 +100 递增，有上限保护

---

## 状态机 7：GATT 事件处理状态机

**所属函数：** `CanProc()` 事件处理段 (can.c:889+)
**作用对象：** `g_CanMcuEvent` / `g_CanBmsEvent` / `g_CanBms1Event` 位域
**触发源：** CAN RX 中断中的 `CanMcuParse()` / `CanBmsParse()` 置位事件标志

### 状态集

每个事件标志位是一个独立的两态状态机：

| 状态 | 含义 |
|------|------|
| **SET** (1) | 报文已接收并解析，等待 CanProc 处理 |
| **CLEARED** (0) | CanProc 已处理，等待下次报文触发 |

### 转移图

```
  CAN RX 中断
  CanMcuParse() / CanBmsParse()
       │
       ▼
  ┌─────────┐  memcpy 数据到结构体    ┌─────────┐
  │ CLEARED │ ─────────────────────► │   SET   │
  │  (0)    │  置位 Event=TRUE       │  (1)    │
  └─────────┘                        └────┬────┘
       ▲                                  │
       │                                  │ CanProc() 主循环
       │                                  │ 处理事件
       │                                  │ GattSetData() 上报
       │                                  ▼
       │                                  ┌─────────┐
       └──────────────────────────────────│ CLEARED │
            Event=FALSE                    │  (0)    │
                                          └─────────┘
```

### 事件-动作映射表

| 事件标志 | 触发源 CAN ID | CanProc 中的处理动作 |
|----------|---------------|---------------------|
| `McuFaultEvent` | 0x01806E600 | GattSetData: DTA_CTMP(控制器温度), DTA_MTPM(电机温度); SetDashBoardData: 档位/倒车/刹车/故障 |
| `McuRunInforEvent` | 0x01806E601 | GattSetData: DTA_MTRD(转速), DTA_TSPD(胎速), DTA_RVLT(电压), DTA_RCUR(电流) |
| `McuPwrOutEvent` | 0x01806E602 | (清位，暂无实质处理) |
| `McuSysInfor1Event` | 0x01806E502 | GattSetData: DTA_RMAX(额定最大输入电流) |
| `McuSysInfor2Event` | 0x01806E503 | GattSetData: DTA_CMXS(最大速度), DTA_CMXC(最大电流) |
| `McuCCSEvent` | 0x01806E640 | 提取 VCU 充电限制电流/电压 → tempvcu_cur/tempvcu_vol |
| `BmsRtChangEvent` | 0x1806E5F4 | 提取 BMS 充电限制电流/电压 → tempbms_cur/tempbms_vol |
| `BmsRtState1Event` | 0x18FF50E6 | 提取 AC 输入电压 → g_AC_ccsinput |
| `BmsRtState2Event` | 0x01806E611 | GattSetData: DTA_RSOC(电池SOC); SetDashBoardData: LIST_soc |
| `BmsRtState3Event` | 0x01806E612 | GattSetData: DTA_ACYC(循环), DTA_FCCP(满充容量), DTA_RCAP(剩余容量), DTA_AENG(已充能量) |
| `BmsCellVolt1~6Event` | 0x01806E613~618 | GattSetData: DIA_CV01~CV23(电芯电压1~23) |
| `BmsRtTempEvent` | 0x01806E620 | GattSetData: DIA_TEMP1~TEMP6(温度1~6，减40℃偏移) |
| `BmsSysInforEvent` | 0x01806E516 | (清位，暂无实质处理) |

### 关键设计点

- **一次性处理**：每个事件在 CanProc 中处理后立即清位，保证不会重复上报
- **事件驱动**：非轮询式，仅在 CAN RX 中断置位后才处理，降低 CPU 占用
- **位域联合体**：3 个字节（g_CanMcuEvent/g_CanBmsEvent/g_CanBms1Event）共 24 个标志位

---

## 状态机优先级与互斥关系

```
  优先级（高 → 低）:
  
  1. BUSOFF 恢复     — 最高，发生总线关闭时立即禁用发送
  2. MISS_ACK 恢复   — 仅在 BUSOFF_NONE 时处理
  3. 充电限制到达     — 覆盖 AC 输入电压判断，强制最小电流
  4. AC 输入电压检测   — 仅在 VCU 未下发电流时生效
  5. CCS 参数合成     — 每 ~2s 执行一次
  6. FIFO 转发       — 每 10ms 执行，受 tx_disable 门控
  7. GATT 事件处理   — 每轮 CanProc 检查，事件驱动
```

**互斥关系：**
- `tx_disable=TRUE` 时，FIFO 转发状态机进入空转（不发送但仍接收）
- `g_Hm7280SerialDirty=TRUE` 时，心跳报文停止发送
- `ccs_energy_limit_reached=1` 时，CCS 合成输出被覆盖为最小值
- BUSOFF 和 MISS_ACK 不会同时激活（MISS_ACK 仅在 BUSOFF_NONE 时检测）

---

## 定时常量一览

| 常量名 | 值 | 单位 | 用途 |
|--------|-----|------|------|
| `T_BUSOFF_QUICK` | 100 | ms | BUSOFF 快恢复超时 |
| `T_BUSOFF_SLOW` | 1000 | ms | BUSOFF 慢恢复超时 |
| `TX_TIMEOUT` | 100 | ms | MISS_ACK 发送超时 |
| `TX_RECOVERY` | 150 | ms | MISS_ACK 恢复等待 |
| `CAN_TX_BUF_SIZE` | 100 | 个 | FIFO 缓冲区大小 |
| 10ms | 10 | ms | FIFO 转发轮询周期 |
| 200×10ms | ~2000 | ms | CCS 合成周期 |
| 5000ms | 5000 | ms | 心跳 + 设备 ID 比较周期 |
| 5 次 | 5 | 次 | PPID 冗余发送次数 |

---

*文档基于 CAN-Gateway 仓库 main + feature/charge-flow-optimize 分支代码生成*
