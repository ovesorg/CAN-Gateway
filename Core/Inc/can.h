
#ifndef __CAN_H__

#define __CAN_H__


#define CAN_TX_BUF_SIZE  100

union BitGroup_TypeDef{
    unsigned char BYTE;
    struct {
        unsigned char B0:1;
        unsigned char B1:1;
        unsigned char B2:1;
        unsigned char B3:1;
        unsigned char B4:1;
        unsigned char B5:1;
        unsigned char B6:1;
        unsigned char B7:1;
    }BIT;
};


typedef struct
{

	uint8_t MotorOverTemp_fault:1;
	uint8_t MotorBlock_fault:1;
	uint8_t Throttle_fault:1;
	uint8_t Heartbeat_fault:1;

	uint8_t WdgOver_fault:1;
	uint8_t reserved1:3;

	uint8_t Mos_fault:1;
	uint8_t Drive_fault:1;
	uint8_t OverCurt_fault:1;
	uint8_t OverVolt_fault:1;
	
	uint8_t OverTemp_fault:1;
	uint8_t UnderVolt_fault:1;
	uint8_t MotorPhaseLost_fault:1;
	uint8_t MotorHall_fault:1;
	
	uint8_t EBSState:1;
	uint8_t HillHolderState:1;
	uint8_t HDCState:1;
	uint8_t TempleProtectState:1;
	
	uint8_t ReadState:1;
	uint8_t ChargeProtectState:1;
	uint8_t TcsFunState:1;
	uint8_t PauseDisableStae:1;

	uint8_t GearState:3;
	uint8_t BrakeState:1;
	
	uint8_t AntiTheftState:1;
	uint8_t SleepLimtState:1;
	uint8_t CruisingState:1;
	uint8_t ReverseState:1;	

	uint8_t ThrottleValue;
	uint8_t BrakeValue;
	uint8_t ContrlTemp;
	uint8_t MotorTemp;
}MCU_FAULT_TypeDef;

typedef struct
{
	uint8_t MotorSpeedH;
	uint8_t MotorSpeedL;
	uint8_t TyreSpeedH;
	uint8_t TyreSpeedL;
	uint8_t RtVoltageH;
	uint8_t RtVoltageL;
	uint8_t RtCurrentH;
	uint8_t RtCurrentL;
}MCU_RUNINFOR_TypeDef;



typedef struct
{
	uint8_t RtDischargeCurtLimitH;
	uint8_t RtDischargeCurtLimitL;
	uint8_t RtEBSCurtLimitH;
	uint8_t RtEBSCurtLimitL;
	
	uint8_t DualPackCurtLimit:3;
	uint8_t reserved:5;
	
	uint8_t reseved1[3];
}MCU_POWEROUT_TypeDef;


typedef struct
{
	uint8_t chargeVolLimitH;
	uint8_t chargeVolLimitL;
	uint8_t chargeCurtLimitH;
	uint8_t chargeCurtLimitL;
	
	 
	
	uint8_t reseved1[4];
}BMSCCS_TypeDef;

typedef struct
{
	uint8_t chargeVolLimitH;
	uint8_t chargeVolLimitL;
	uint8_t chargeCurtLimitH;
	uint8_t chargeCurtLimitL;
	
	 
	
	uint8_t reseved1[4];
}MCUCCS_TypeDef;

typedef struct
{
	uint8_t DevidH[8];
	uint8_t DevidL[8];
	
}HM7280_CCS_DEVID_TypeDef;

typedef struct
{
	uint8_t RateSpeedH;
	uint8_t RateSpeedL;
	uint8_t RateVoltH;
	uint8_t RateVoltL;
	uint8_t RateMaxInputCurtH;
	uint8_t RateMaxInputCurtL;
	
	uint8_t ThrottleMaxVolt;
	uint8_t ThrottleMinVolt;
}MCU_SYSINFOR1_TypeDef;

typedef struct
{
	uint8_t MaxSpeedSetH;
	uint8_t MaxSpeedSetL;
	uint8_t MaxInputCurtSetH;
	uint8_t MaxInputCurtSetL;
	uint8_t MaxEBSCurtSetH;
	uint8_t MaxEBSCurtSetL;
	uint8_t UnderVoltSet;
	
	uint8_t FunSwtich_HillHoldSet:1;
	uint8_t FunSwtich_HDCSet:1;
	uint8_t FunSwtich_TCSSet:1;
	uint8_t FunSwtich_WDGSet:1;
	uint8_t reserved:4;
}MCU_SYSINFOR2_TypeDef;

typedef struct
{
	uint8_t HillHolderFun;
	uint8_t HdcFun;
	uint8_t TcsFun;

	uint8_t Reserved[5];
}MCU_FUNSWCH_TypeDef;


typedef struct
{
/*
	uint8_t PackOpenload:1;
	uint8_t DischrgShortLoad:1;
	uint8_t ChrgOverTempProtect:1;
	uint8_t ChrgLowTempProtect:1;

	uint8_t PcbMosOverTempProtect:1;
	uint8_t TempSensorFault:1;
	uint8_t ChargingMosOn:1;
	uint8_t DisChargeMosOn:1;

	uint8_t NormalDisCharge:1;
	uint8_t NormalCharge:1;
	uint8_t ChrgOverProtect:1;
	uint8_t LowVoltProtect:1;
	
	uint8_t ChrgOverCurrentProtect:1;
	uint8_t DischrgOverCurrentProtect:1;
	uint8_t DischrgOverTempProtect:1;
	uint8_t DischrgLowTempProtect:1;
	*/

	uint8_t temp1;
	uint8_t temp2;

	uint8_t acinputh;
	uint8_t acinputl;

	uint8_t PackMaxTemp;
	uint8_t PackMinTemp;
}BMS_RT_STATUS1_TypeDef;

typedef struct
{

	uint8_t Soc;
	uint8_t Soh;

	uint8_t MaxPremDischargeCurrentH;
	uint8_t MaxPremDischargeCurrentL;

	uint8_t MaxChargeCurrentH;
	uint8_t MaxChargeCurrentL;
	
	uint8_t MaxChargeVoltageH;
	uint8_t MaxChargeVoltageL;

}BMS_RT_STATUS2_TypeDef;

typedef struct
{

	uint8_t FullBatCapH;
	uint8_t FullBatCapL;

	uint8_t RemainBatCapH;
	uint8_t RemainBatCapL;

	uint8_t CycleNumbH;
	uint8_t CycleNumbL;

	uint8_t ChargerDetected:1;
	uint8_t DisChargerDetected:1;
	uint8_t BleHeartbeat:1;
	uint8_t BatWatchdogEnable:1;
	uint8_t RefreshWatchdog:1;
	uint8_t BatWarn:1;
	uint8_t BatDisable:1;
	uint8_t reserved:1;
	
	uint8_t RemainChargingTime;  // 0.1h

}BMS_RT_STATUS3_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage1H;
	uint8_t CellVoltage1L;

	uint8_t CellVoltage2H;
	uint8_t CellVoltage2L;

	uint8_t CellVoltage3H;
	uint8_t CellVoltage3L;

	uint8_t CellVoltage4H;
	uint8_t CellVoltage4L;

}BMS_CELLVOLT1_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage5H;
	uint8_t CellVoltage5L;

	uint8_t CellVoltage6H;
	uint8_t CellVoltage6L;

	uint8_t CellVoltage7H;
	uint8_t CellVoltage7L;

	uint8_t CellVoltage8H;
	uint8_t CellVoltage8L;

}BMS_CELLVOLT2_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage9H;
	uint8_t CellVoltage9L;

	uint8_t CellVoltage10H;
	uint8_t CellVoltage10L;

	uint8_t CellVoltage11H;
	uint8_t CellVoltage11L;

	uint8_t CellVoltage12H;
	uint8_t CellVoltage12L;

}BMS_CELLVOLT3_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage13H;
	uint8_t CellVoltage13L;

	uint8_t CellVoltage14H;
	uint8_t CellVoltage14L;

	uint8_t CellVoltage15H;
	uint8_t CellVoltage15L;

	uint8_t CellVoltage16H;
	uint8_t CellVoltage16L;

}BMS_CELLVOLT4_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage17H;
	uint8_t CellVoltage17L;

	uint8_t CellVoltage18H;
	uint8_t CellVoltage18L;

	uint8_t CellVoltage19H;
	uint8_t CellVoltage19L;

	uint8_t CellVoltage20H;
	uint8_t CellVoltage20L;

}BMS_CELLVOLT5_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage21H;
	uint8_t CellVoltage21L;

	uint8_t CellVoltage22H;
	uint8_t CellVoltage22L;

	uint8_t CellVoltage23H;
	uint8_t CellVoltage23L;

	uint8_t CellVoltage24H;
	uint8_t CellVoltage24L;

}BMS_CELLVOLT6_TypeDef;



typedef struct
{
   //mV
	uint8_t BmsTemperature1;
	uint8_t BmsTemperature2;

	uint8_t BmsTemperature3;
	uint8_t BmsTemperature4;

	uint8_t BmsTemperature5;
	uint8_t BmsTemperature6;

	uint8_t BmsTemperature7;
	uint8_t BmsTemperature8;

}BMS_RTTEMP_TypeDef;

typedef struct
{
   //mV
	uint8_t RatedCapH;
	uint8_t RatedCapL;
	uint8_t Reserved1[2];

	uint8_t RatedVoltageH;
	uint8_t RatedVoltageL;

	uint8_t Reserved[2];

}BMS_SYSINFOR_TypeDef;

typedef struct
{
	uint8_t WatchdogEnable:1;
	uint8_t reserved:7;

	uint8_t Reserved1[7];

}VCU_WDG_TypeDef;

typedef struct
{
	uint8_t Heartbeat:1;
	uint8_t reserved:7;

	uint8_t Wdgrefresh:1;
	uint8_t reserved1:7;

	uint8_t Reserved2[6];
}VCU_WDG_REFRESH_TypeDef;




typedef struct
{
	uint32_t can_id;
	uint8_t *p;
}CAN_TX_TypeDef;

typedef struct
{
	uint8_t canTx_Pc_en;
	uint8_t canTx_busy;
	uint8_t can0_count;
	uint8_t can1_count;
	
	uint32_t t100ms;
	uint32_t t200ms;
	
	uint32_t t1000ms;
	uint32_t t2000ms;
uint32_t bmscandowntime;
	uint32_t vcucandowntime;
	uint32_t t5000ms;
	uint32_t t10ms;
}CAN_TXSTATE_TypeDef;


typedef struct
{

	uint16_t reserved;

	uint16_t ms:4;
	uint16_t msg_addr:4;
	uint16_t ver:4;
	uint16_t reserved1:4;
}CAN_ID_TypeDef;

//buss fault process

#define BEHAVIOR_TIMEOUT	0

#define TX_TIMEOUT	100
#define TX_RECOVERY	150	

#define T_BUSOFF_QUICK	100
#define T_BUSOFF_SLOW	1000


typedef struct
{
	uint8_t tx_disable;
	uint8_t busoff_state;
	uint8_t miss_ack_state;
	uint8_t reserved;
	
	uint32_t busoff_start_timer;
	uint32_t miss_ack_start_timer;
	
	uint16_t busoff_counter;
	uint16_t reserved1;
}CAN_RECOVERY_TypeDef;

enum
{
	MISS_ACK_NONE,
	MISS_ACK_TIMEOUT,
	MISS_ACK_RECOVERY
};

enum
{
	BUSOFF_NONE,
	BUSOFF_QUICK,
	BUSOFF_SLOW
};



#define McuFaultEvent     g_CanMcuEvent.BIT.B0
#define McuRunInforEvent     g_CanMcuEvent.BIT.B1
#define McuPwrOutEvent     g_CanMcuEvent.BIT.B2
#define McuSysInfor1Event     g_CanMcuEvent.BIT.B3
#define McuSysInfor2Event     g_CanMcuEvent.BIT.B4
#define McuCCSEvent     g_CanMcuEvent.BIT.B5
#define BmsRtState1Event	g_CanMcuEvent.BIT.B6

#define BmsRtChangEvent      g_CanBmsEvent.BIT.B0
#define BmsRtState2Event     g_CanBmsEvent.BIT.B1
#define BmsRtState3Event     g_CanBmsEvent.BIT.B2
#define BmsCellVolt1Event    g_CanBmsEvent.BIT.B3
#define BmsCellVolt2Event    g_CanBmsEvent.BIT.B4
#define BmsCellVolt3Event    g_CanBmsEvent.BIT.B5
#define BmsCellVolt4Event    g_CanBmsEvent.BIT.B6
#define BmsCellVolt5Event    g_CanBmsEvent.BIT.B7

#define BmsCellVolt6Event    g_CanBms1Event.BIT.B0
#define BmsRtTempEvent    g_CanBms1Event.BIT.B1
#define BmsSysInforEvent    g_CanBms1Event.BIT.B2




void CanRamInit(void);
void CanProc(void);
void CanBmsParse(uint32_t id,uint8_t *data,uint8_t len);
void CanMcuParse(uint32_t id,uint8_t *data,uint8_t len);
void CanTransmit(uint32_t id,uint8_t *data,uint8_t len);
void CanBmsParse_bms(uint32_t id,uint8_t *data,uint8_t len); //bms
uint32_t get_bat_rcap_mWh(void);
void set_CcsEnergy_mWh(uint32_t value);
uint8_t stop_changestaate(void);
#ifdef CAN_TRASMITER_SUPPORT
void Can0RxProc(can_receive_message_struct* rx_message);
void Can1RxProc(can_receive_message_struct* rx_message);

void Can0Transmit(can_trasnmit_message_struct *transmit_message);
void Can1Transmit(can_trasnmit_message_struct *transmit_message);
#endif
#endif


