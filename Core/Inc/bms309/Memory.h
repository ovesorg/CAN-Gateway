#ifndef	_MEMORY_H
#define	_MEMORY_H
//#include "c51_type.h"


#define xdata  
#define bdata  
#define data
#define idata
#define code 

#define BOOL uint8_t

#define bit uint8_t


typedef	uint8_t	U8;
typedef	uint16_t	U16;
typedef	uint32_t	U32;
typedef	int8_t	S8;
typedef	int16_t	S16;
typedef	int32_t	S32;




#define STACK_ADDR					0xa0 
#define DATAFLASH_MAP_ADDR	        0x0000
#define DATAFLASH_ADDR				0x0000
#define	DATAFLASH_ADDR2             (DATAFLASH_ADDR + 512)
#define DATAFLASH_OK_FLG1_ADDR      (DATAFLASH_ADDR + 510)
#define DATAFLASH_OK_FLG2_ADDR      (DATAFLASH_ADDR2 + 510)

#define RAM_CHECK_DATA              0x5A


typedef struct
{
	uint8_t reserved;
	
	uint8_t bCHGEnd:1 ;//		=	uiPackConfig^8;
	uint8_t bDSGEnd :1;//		=	uiPackConfig^9;
	uint8_t reserved1:1;
	uint8_t bEnEEPRomBK:1;//	=	uiPackConfig^11;
	uint8_t bLEDNum0:1;//		=	uiPackConfig^12;
	uint8_t bLEDNum1:1;//		=	uiPackConfig^13;
	uint8_t bTempNum0:1;//		=	uiPackConfig^14;
	uint8_t bTempNum1:1;//		=	uiPackConfig^15;
}PACK_CONFIG_Typedef;

typedef union
{
	uint16_t PackConfig;
	PACK_CONFIG_Typedef bitmap;
}UI_PACK_CONFIG_Typedef;

typedef struct
{
	uint8_t bFC:1 ;//			=	uiPackStatus^0;
	uint8_t bFD:1 ;//			=	uiPackStatus^1;
	uint8_t bVDQ:1 ;//			=	uiPackStatus^2;
	uint8_t bOverLoad:1 ;//		=	uiPackStatus^3;
	uint8_t bBLEOPEN:1 ;//		=	uiPackStatus^3;
	uint8_t bCAL:1 ;//			=	uiPackStatus^4;
	uint8_t reserved:3;
	
	uint8_t bDSG_FET:1 ;//		=	uiPackStatus^8;
	uint8_t bCHG_FET:1 ;//		=	uiPackStatus^9;
	uint8_t bPCHG_FET:1 ;//		=	uiPackStatus^10;
	uint8_t bL0V:1 ;//			=	uiPackStatus^11;
	uint8_t bAFE_ERR:1 ;//		=   uiPackStatus^12;
	uint8_t reserved1:1;
	uint8_t bDSGING:1 ;//		=	uiPackStatus^14;
	uint8_t bCHGING:1 ;//		=	uiPackStatus^15;
}PACK_STATE_Typedef;


typedef union
{
	uint16_t PackStatus;
	PACK_STATE_Typedef bitmap;
}UI_PACK_STATE_Typedef;


typedef struct
{
	uint8_t bUTC:1 ;//			=	uiBatStatus^0;
	uint8_t bOTC:1 ;//			=	uiBatStatus^1;
	uint8_t bUTD:1 ;//			=	uiBatStatus^2;
	uint8_t bOTD:1 ;//			=	uiBatStatus^3;
	uint8_t reserved:4;
	
	uint8_t bHV:1 ;//			=	uiBatStatus^8;
	uint8_t bLV:1 ;//			=	uiBatStatus^9;
	uint8_t bOCD1:1 ;//			=	uiBatStatus^10;
	uint8_t bOCD2:1 ;//			=	uiBatStatus^11;
	uint8_t bOCC:1 ;//			=	uiBatStatus^12;
	uint8_t bSC:1 ;//			=	uiBatStatus^13;
	uint8_t bPF:1 ;//			=	uiBatStatus^14;
	uint8_t reserved1:1;

}BAT_STATE_Typedef;

typedef union
{
	uint16_t BatStatus;
	BAT_STATE_Typedef bitmap;
}UI_BAT_STATE_Typedef;


//*****************************BIT MEMORY START***************************//

/*extern U16 bdata uiPackConfig;				
extern U16 bdata uiPackStatus;				
extern U16 bdata uiBatStatus;*/
extern UI_BAT_STATE_Typedef g_uiBatStatus;
extern UI_PACK_CONFIG_Typedef g_uiPackConfig;
extern UI_PACK_STATE_Typedef g_uiPackStatus;

#define uiPackConfig  g_uiPackConfig.PackConfig
#define uiPackStatus  g_uiPackStatus.PackStatus
#define uiBatStatus  g_uiBatStatus.BatStatus


 

#define  bCHGEnd  g_uiPackConfig.bitmap.bCHGEnd
#define  bDSGEnd	g_uiPackConfig.bitmap.bDSGEnd 
#define  bEnEEPRomBK	 g_uiPackConfig.bitmap.bEnEEPRomBK
#define  bLEDNum0	  g_uiPackConfig.bitmap.bLEDNum0
#define  bLEDNum1	  g_uiPackConfig.bitmap.bLEDNum1
#define  bTempNum0	   g_uiPackConfig.bitmap.bTempNum0
#define  bTempNum1	   g_uiPackConfig.bitmap.bTempNum1

#define  bFC	 g_uiPackStatus.bitmap.bFC
#define  bFD	 g_uiPackStatus.bitmap.bFD
#define  bVDQ	  g_uiPackStatus.bitmap.bVDQ
#define  bOverLoad	   g_uiPackStatus.bitmap.bOverLoad
#define  bBLEOPEN	  g_uiPackStatus.bitmap.bBLEOPEN
#define  bCAL	  g_uiPackStatus.bitmap.bCAL
#define  bDSG_FET	  g_uiPackStatus.bitmap.bDSG_FET
#define  bCHG_FET	  g_uiPackStatus.bitmap.bCHG_FET
#define  bPCHG_FET	  g_uiPackStatus.bitmap.bPCHG_FET 
#define  bL0V	  g_uiPackStatus.bitmap.bL0V
#define  bAFE_ERR	g_uiPackStatus.bitmap.bAFE_ERR  
#define  bDSGING	 g_uiPackStatus.bitmap.bDSGING
#define  bCHGING	 g_uiPackStatus.bitmap.bCHGING

#define  bUTC	  g_uiBatStatus.bitmap.bUTC
#define  bOTC	  g_uiBatStatus.bitmap.bOTC
#define  bUTD	  g_uiBatStatus.bitmap.bUTD
#define  bOTD	  g_uiBatStatus.bitmap.bOTD
#define  bHV	 g_uiBatStatus.bitmap.bHV
#define  bLV	 g_uiBatStatus.bitmap.bLV
#define  bOCD1	  g_uiBatStatus.bitmap.bOCD1 
#define  bOCD2	   g_uiBatStatus.bitmap.bOCD2
#define  bOCC	  g_uiBatStatus.bitmap.bOCC
#define  bSC	 g_uiBatStatus.bitmap.bSC
#define  bPF	 g_uiBatStatus.bitmap.bPF


/*extern BOOL bCHGEnd;
extern BOOL bDSGEnd;
extern BOOL bEnEEPRomBK;
extern BOOL bLEDNum0;
extern BOOL bLEDNum1;
extern BOOL bTempNum0;
extern BOOL bTempNum1;
extern BOOL bFC;
extern BOOL bFD;
extern BOOL bVDQ;
//extern BOOL bOverLoad;
extern BOOL bBLEOPEN;
extern BOOL bCAL;
extern BOOL bDSG_FET;
extern BOOL bCHG_FET;
extern BOOL bPCHG_FET;
extern BOOL bL0V;
extern BOOL bAFE_ERR;
extern BOOL bDSGING;
extern BOOL bCHGING;
extern BOOL bUTC;
extern BOOL bOTC;
extern BOOL bUTD;
extern BOOL bOTD;
extern BOOL bHV;
extern BOOL bLV;
extern BOOL bOCD1;
extern BOOL bOCD2;
extern BOOL bOCC;
extern BOOL bSC;
extern BOOL bPF;*/

extern BOOL bWakeupFlg;						//唤醒标志，系统从低功耗被唤醒
extern BOOL bCADCFlg;						//CADC转码完成标志
extern BOOL bCalibrationFlg;				//上位机发送校准命令后置位该标志
extern BOOL bWrFlashFlg;					//写FLASH标志
extern BOOL bE2PProcessFlg;					//EEPROM处理标志
extern BOOL bISPFlg;						//ISP升级标志，进入ISP程序
extern BOOL bTimerFlg;						//1s定时器标志
extern BOOL bIdleTimerFlg;					//5s定时器标志
extern BOOL bHalfHzFlg;						//0.5HZ标志（用于LED显示蓝牙状态）
extern BOOL b2HzFlg;						//2HZ标志（用于LED显示蓝牙状态）
extern BOOL bIdleFlg;						//系统进入IDLE标志
extern BOOL bSleepFlg;						//系统进入SLEEP标志
extern BOOL bPCSleepFlg;					//PC通知系统进入SLEEP标志
extern BOOL bLEDOpen;						//LED电量显示标志
extern BOOL bLEDFlg;						//LED电量显示过程中
extern BOOL bLongKeyFlg;					//按键长按检测标志
extern BOOL bBleOnOffFlg;					//蓝牙启停标志
extern BOOL bUartSndAckFlg;					//UART已经发送ACK给主机
extern BOOL bUartNeedAckFlg;				//UART需要发送ACK给主机
extern BOOL bCHGClosedFlg;					//充电结束关闭充电MOS标志
extern BOOL bDSGClosedFlg;					//放电结束关闭放电MOS标志
extern BOOL bProtectFlg;					//保护发生标志，需要从低功耗唤醒(未置位，恒为0)
extern BOOL bAFEFlg;						//AFE的ALARM发生标志

extern BOOL bDsgToChgFlg;					//放电转换为充电，需要备份数据
extern BOOL bChgToDsgFlg;					//充电转换为放电，需要备份数据
extern BOOL bLVBkFlg;						//LV低电压标志，需要备份信息到外挂EEPROM
extern BOOL bE2PBKDsgEnd;					//放电结束标志，需要备份信息到外挂EEPROM
extern BOOL bE2PBKChgStop;					//充电结束标志，需要备份信息到外挂EEPROM
extern BOOL bE2PBKChgStart;					//充电开始标志，需要备份信息到外挂EEPROM
extern BOOL bE2PBKRtc;						//RTC定时备份标志，需要备份信息到外挂EEPROM
extern BOOL bE2PErase;						//擦除外挂EEPROM标志
extern BOOL bE2PRdData;						//读取外挂EEPROM标志
extern BOOL bRTCRdTime;						//读取RTC时间标志
//extern BOOL bE2ON;				            //E2ON(0：外挂E2需初始化。1：外挂E2无需初始化。)
//extern BOOL bRTCON;							//RTCON(0：外挂RTC需初始化。1：外挂RTC无需初始化。)



extern U16 code  NTC103AT[161];				//热敏电阻NTC103AT阻值表

//*****************************DATA MEMORY START***************************//
extern U8 idata STACK[];		//堆栈

extern U8  data ucResetFlag;				//PC to send a software reset instruction
extern U8  data ucTimer0Cnt;				//Timer0 counter, Every 20ms +1
extern U8  data ucTimer0Cnt1;				
extern U8  data ucFlashWrValid;				//Write flash protect flag
extern U8  data ucKeyDownCnt;				//Key Down state counter
extern U8  xdata ucCellNum;					//For storage cell num
extern U16 data uiCellVmax;					//The maximum value of all the Cell
extern U16 data uiCellVmin;					//The minimum value of all the Cell
extern U8  data ucUartTimeCnt;				//Uart no communication timing, for enter sleep or idle
extern U8  data ucIdleTimeCnt;				//idle counter
extern S16 xdata siCurBuf[4];				//for storage CADC value, Is used to calculate the mean Within 1s
extern U8  data ucCadcTimeCnt;				//for storage CADC value, Is used to calculate the mean Within 1s
extern U8  data ucChgEndTimeCnt;			//Charging cut-off delay count
extern U8  data ucChgEndRTimeCnt;
extern U8  data ucDsgEndTimeCnt;			//Discharging cut-off delay count
extern U8  data ucDsgEndRTimeCnt;			//Discharging cut-off delay count
extern U8  data ucBalanceTimeCnt[16];		//Balance time counter(for each cell)
extern U8  data ucBalUpdateTimeCnt;			//Balance update time counter
extern U16 data uiBalanceChannel;			//Balance Channel
extern U8  data ucLEDTimeCnt;				//LED display delay count

extern U8  xdata ucExtcaliSwitch1;			//calibration flag
extern U8  xdata ucExtcaliFlag;				//calibration flag
extern U32 xdata ulExtVPack;				//During calibration, the received total voltage
extern S32 xdata slExtCur;					//During calibration, the received current
extern U16 xdata uiExtTemp1;				//During calibration, the received ttemperature1
extern U16 xdata uiExtTemp2;				//During calibration, the received ttemperature2
extern U16 xdata uiExtTemp3;				//During calibration, the received ttemperature3
extern U8  xdata ucTempeMiddle;				//Record the current temperature resistance corresponding address, for the next quick look
extern U8  xdata ucExtRTC[6];				//During calibration, the received RTC Time


extern U8  xdata ucMTPConfVal;				//for MTP CONF Register

extern U32 xdata ulRCCharge;				//Charge capacity statistics
extern U32 xdata ulRCDischarge;				//Discharge capacity statistics
extern U32 xdata ulDsgCycleCount;			//Discharge capacity statistics, for update E2uiCycleCount
extern U32 xdata ulFCCCount;				//The effective discharge capacity statistics, for updating E2ulFCC

extern U16 xdata uiE2PDataAddr;
extern U8  xdata ucRTCBKTime1;
extern U16 xdata uiRTCBKTime2;
extern U8  xdata ucRTCBuf[];
extern U8 idata ucUpDataLimitTime;

extern U16 xdata uiCHGValidTime;

extern BOOL bUartReadFlg;
extern BOOL bUartWriteFlg;
extern U8 xdata ucSubClassID;
extern U8 xdata ucUartBufPT;			//Pointing to the current UART Buffer
extern U8 xdata ucUartSndLength;		//UART Buffer send length
extern U8 xdata ucUartTimeoutCnt;		//UART timerout cnt, If not Uart communication within 2s, then clear ucUartBufPT
extern U8 xdata ucUartBuf[];			//For UART transmit or acceptance buffer




//*****************************XDATA MEMORY START***************************//
//系统参数，子命令号：0x00
extern U16 xdata E2uiPackConfigMap;
extern U16 xdata E2uiVOC[];
extern U32 xdata E2ulDesignCap;
extern U32 xdata E2ulFCC;
extern U32 xdata E2ulCycleThreshold;
extern U16 xdata E2uiCycleCount;
extern U16 xdata E2uiNearFCC;
extern S16 xdata E2siLearnLowTemp;
extern S16 xdata E2siDfilterCur;
extern U8  xdata E2ucSleepDelay;
extern U8  xdata E2ucIdleDelay;
extern U8  xdata E2ucCommOffDelay;
extern U8  xdata E2ucChgBKDelay;
extern S16 xdata E2siChgBKCur;
extern U8  xdata E2ucRTCBKDelay;
extern U8  xdata E2ucRamCheckFlg1;

//用户自定义A区，子命令号：0x01
extern U16 xdata E2uiSWVersion;
extern U16 xdata E2uiHWVersion;
extern U8  xdata E2ucID;
extern U8  xdata E2ucMNFName[];
extern U32 xdata E2ulMNFDate;
extern U16 xdata E2uiSerialNum;
extern U8  xdata E2ucDeviceName[];
extern U8  xdata E2ucDeviceChem[];
extern U16 xdata E2uiChemID;
extern U8  xdata E2ucRamCheckFlg2;

//充电参数，子命令号：0x02
extern U16 xdata E2uiChgEndVol;
extern S16 xdata E2siChgEndCur;
extern U8  xdata E2ucChgEndDelay;
extern U8  xdata E2ucRamCheckFlg3;

//平衡参数，子命令号：0x08
extern U16 xdata E2uiBalanceVol;
extern S16 xdata E2siBalanceCur;
extern U16 xdata E2uiBalanceVolDiff;
extern U8  xdata E2ucBalanceDelay;
extern U8  xdata E2ucRamCheckFlg4;

//放电参数，子命令号：0x03
extern U16 xdata E2uiDsgEndVol;
extern U8  xdata E2ucDsgEndDelay;
extern U8  xdata E2ucRamCheckFlg5;

//AFE参数，子命令号：0x0A
extern U8  xdata ucMTPBuffer[26];
extern U8  xdata E2ucRamCheckFlg6;

//校准参数，子命令号：0x0B
extern U16 xdata E2uiVPackGain;
extern S16 xdata E2siCadcGain;
extern S16 xdata E2siCadcZero;
extern S16 xdata E2siTempe1Offset;
extern S16 xdata E2siTempe2Offset;
extern S16 xdata E2siTempe3Offset;
extern U8  xdata E2ucCalibrated;
extern U8  xdata E2ucRamCheckFlg7;

//类E2写入OK标志
extern U16 xdata E2uiCheckFlag;


typedef struct	_SYSINFOR_							
{
	U16		VCell[16];
	U32		Voltage;
	S32		CurCadc;
	U16		Temperature1;
	U16		Temperature2;
	U16		Temperature3;
	U32		E2ulFCC;
  U32		RC1;
	U16		RSOC;
	U16		E2uiCycleCount;
	U16		PackStatus;
	U16		BatStatus;
	U16		PackConfig;
	U16		ManufactureAccess;
}SYSINFOR;

extern SYSINFOR	xdata	Info;

typedef struct	_AFEDATA_
{
	U16 Temp1;
	U16 Temp2;
	U16 Temp3;
	S16 Cur1;
	U16 Cell[16];
	S16 Cadc;
}AFEDATA;

extern AFEDATA xdata AFE;


typedef struct _RTC_VAR_					//BCD Type
{
	U8 Second;
	U8 Minute;
	U8 Hour;
	U8 Week;
	U8 Date;
	U8 Month;
	U8 Year;
}RTC_VAR;
extern RTC_VAR xdata RTCTime;
extern U16  xdata uiE2PDataAddr;
extern U8   xdata ucRTCBKTime1;
extern U16  xdata uiRTCBKTime2;
extern U8   xdata ucRTCBuf[];

#endif



