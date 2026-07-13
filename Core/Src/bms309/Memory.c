/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	andyliu
Version: 	V0.0
Date: 		2014/05/30
History:
	V0.0		2014/05/30		 Preliminary
********************************************************************************/
#include "main.h"
#include "Memory.h"
#include "McuFlash.h"

U8 code JumpISP[5]	={0x5A,0xA5,0x02,0xF4,0x00};        //ISP关键字定义

//*****************************BIT MEMORY START***************************//


/*U16 bdata uiPackConfig;				
sbit bCHGEnd		=	uiPackConfig^8;
sbit bDSGEnd		=	uiPackConfig^9;
sbit bEnEEPRomBK	=	uiPackConfig^11;
sbit bLEDNum0		=	uiPackConfig^12;
sbit bLEDNum1		=	uiPackConfig^13;
sbit bTempNum0		=	uiPackConfig^14;
sbit bTempNum1		=	uiPackConfig^15;*/


UI_PACK_CONFIG_Typedef g_uiPackConfig;





/*U16 bdata uiPackStatus;				
sbit bFC			=	uiPackStatus^0;
sbit bFD			=	uiPackStatus^1;
sbit bVDQ			=	uiPackStatus^2;
//sbit bOverLoad		=	uiPackStatus^3;
sbit bBLEOPEN		=	uiPackStatus^3;
sbit bCAL			=	uiPackStatus^4;
sbit bDSG_FET		=	uiPackStatus^8;
sbit bCHG_FET		=	uiPackStatus^9;
sbit bPCHG_FET		=	uiPackStatus^10;
sbit bL0V			=	uiPackStatus^11;
sbit bAFE_ERR		=   uiPackStatus^12;
sbit bDSGING		=	uiPackStatus^14;
sbit bCHGING		=	uiPackStatus^15;*/

UI_PACK_STATE_Typedef g_uiPackStatus;



/*U16 bdata uiBatStatus;				
sbit bUTC			=	uiBatStatus^0;
sbit bOTC			=	uiBatStatus^1;
sbit bUTD			=	uiBatStatus^2;
sbit bOTD			=	uiBatStatus^3;
sbit bHV			=	uiBatStatus^8;
sbit bLV			=	uiBatStatus^9;
sbit bOCD1			=	uiBatStatus^10;
sbit bOCD2			=	uiBatStatus^11;
sbit bOCC			=	uiBatStatus^12;
sbit bSC			=	uiBatStatus^13;
sbit bPF			=	uiBatStatus^14;*/

UI_BAT_STATE_Typedef g_uiBatStatus;


BOOL bWakeupFlg;					//唤醒标志，系统从低功耗被唤醒
BOOL bCADCFlg;						//CADC转码完成标志
BOOL bCalibrationFlg=0;				//上位机发送校准命令后置位该标志
BOOL bWrFlashFlg;					//写FLASH标志
BOOL bE2PProcessFlg;				//EEPROM处理标志
BOOL bISPFlg;						//ISP升级标志，进入ISP程序
BOOL bTimerFlg;						//1s定时器标志
BOOL bIdleTimerFlg;					//5s定时器标志
BOOL bHalfHzFlg;					//0.5HZ标志（用于LED显示蓝牙状态）
BOOL b2HzFlg;						//2HZ标志（用于LED显示蓝牙状态）
BOOL bIdleFlg;						//系统进入IDLE标志
BOOL bSleepFlg;						//系统进入SLEEP标志
BOOL bPCSleepFlg;					//PC通知系统进入SLEEP标志
BOOL bLEDOpen;						//LED电量显示标志
BOOL bLEDFlg;						//LED电量显示过程中
BOOL bLongKeyFlg;					//按键长按标志
BOOL bBleOnOffFlg;					//蓝牙启停标志
BOOL bUartSndAckFlg;				//UART已经发送ACK给主机
BOOL bUartNeedAckFlg;				//UART需要发送ACK给主机
BOOL bCHGClosedFlg;					//充电结束关闭充电MOS标志
BOOL bDSGClosedFlg;					//放电结束关闭放电MOS标志
BOOL bProtectFlg;					//保护发生标志，需要从低功耗唤醒(未置位，恒为0)
BOOL bAFEFlg;						//AFE的ALARM发生标志

BOOL bDsgToChgFlg;					//放电转换为充电，需要备份数据
BOOL bChgToDsgFlg;					//充电转换为放电，需要备份数据
BOOL bLVBkFlg;						//LV低电压标志，需要备份信息到外挂EEPROM
BOOL bE2PBKDsgEnd;					//放电结束标志，需要备份信息到外挂EEPROM
BOOL bE2PBKChgStop;					//充电结束标志，需要备份信息到外挂EEPROM
BOOL bE2PBKChgStart;				//充电开始标志，需要备份信息到外挂EEPROM
BOOL bE2PBKRtc;						//RTC定时备份标志，需要备份信息到外挂EEPROM
BOOL bE2PErase;						//擦除外挂EEPROM标志
BOOL bE2PRdData;					//读取外挂EEPROM标志
BOOL bRTCRdTime;					//读取RTC时间标志
//BOOL bE2ON;				            //E2ON(0：外挂E2需初始化。1：外挂E2无需初始化。)
//BOOL bRTCON;						//RTCON(0：外挂RTC需初始化。1：外挂RTC无需初始化。)

//*****************************DATA MEMORY START***************************//
//U8 idata STACK[0x100-STACK_ADDR]  	_at_	STACK_ADDR;		//堆栈

U8  data ucResetFlag;				//PC to send a software reset instruction
U8  data ucTimer0Cnt;				//Timer0 counter, Every 20ms +1
U8	data ucTimer0Cnt1;				
U8  data ucFlashWrValid;			//Write flash protect flag
U8  data ucKeyDownCnt;				//Key Down state counter
U8	xdata ucCellNum;				//For storage cell num
U16 data uiCellVmax;				//The maximum value of all the Cell
U16 data uiCellVmin;				//The minimum value of all the Cell
U8  data ucUartTimeCnt;				//Uart no communication timing, for enter sleep or idle
U8  data ucIdleTimeCnt;				//idle counter
S16 xdata siCurBuf[4];				//for storage CADC value, Is used to calculate the mean Within 1s
U8  data ucCadcTimeCnt;				//for storage CADC value, Is used to calculate the mean Within 1s
U8  data ucChgEndTimeCnt;			//Charging cut-off delay count
U8  data ucChgEndRTimeCnt;
U8  data ucDsgEndTimeCnt;			//Discharging cut-off delay count
U8  data ucDsgEndRTimeCnt;			//Discharging cut-off delay count
U8  data ucBalanceTimeCnt[16];		//Balance time counter(for each cell)
U8  data ucBalUpdateTimeCnt;		//Balance update time counter
U16 data uiBalanceChannel;			//Balance Channel
U8  data ucLEDTimeCnt;				//LED display delay count

U8  xdata ucExtcaliSwitch1;			//calibration flag
U8  xdata ucExtcaliFlag;			//calibration flag
U32 xdata ulExtVPack;				//During calibration, the received total voltage
S32 xdata slExtCur;					//During calibration, the received current
U16 xdata uiExtTemp1;				//During calibration, the received ttemperature1
U16 xdata uiExtTemp2;				//During calibration, the received ttemperature2
U16 xdata uiExtTemp3;				//During calibration, the received ttemperature3
U8  xdata ucTempeMiddle;			//Record the current temperature resistance corresponding address, for the next quick look
U8  xdata ucExtRTC[6];				//During calibration, the received RTC Time
RTC_VAR xdata RTCTime;					//For External RTC

//U8  xdata ucMTPBuffer[26];			//for storage MTP register values
U8  xdata ucMTPConfVal;				//for MTP CONF Register

U32 xdata ulRCCharge;				//Charge capacity statistics
U32 xdata ulRCDischarge;			//Discharge capacity statistics
U32 xdata ulDsgCycleCount;			//Discharge capacity statistics, for update E2uiCycleCount
U32 xdata ulFCCCount;				//The effective discharge capacity statistics, for updating E2ulFCC


U16 xdata uiE2PDataAddr;
U8  xdata ucRTCBKTime1;
U16 xdata uiRTCBKTime2;
U8  xdata ucRTCBuf[7];
U8  idata ucUpDataLimitTime;

U16  xdata uiCHGValidTime;

BOOL     bUartReadFlg;
BOOL     bUartWriteFlg;
U8 xdata ucSubClassID;
/*U8 xdata ucUartBufPT 				_at_ UART_BUFPT_MAP_ADDR;		//Pointing to the current UART Buffer
U8 xdata ucUartSndLength 			_at_ UART_SEND_LENGTH_MAP_ADDR;	//UART Buffer send length
U8 xdata ucUartTimeoutCnt 			_at_ UART_TIMEOUT_CNT_MAP_ADDR;	//UART timerout cnt, If not Uart communication within 2s, then clear ucUartBufPT
U8 xdata ucUartBuf[150] 			_at_ UART_BUF_MAP_ADDR;			//For UART transmit or acceptance buffer
*/

SYSINFOR xdata Info;				//System information, For PC display
AFEDATA xdata AFE;					//Used to store SH367309 ADC Data collection


//*****************************XDATA MEMORY START***************************//
//系统参数，子命令号：0x00
U16 xdata E2uiPackConfigMap		;//	_at_	SYS_PARA_MAP_ADDR;
U16 xdata E2uiVOC[10]			;//	_at_	SYS_PARA_MAP_ADDR+2;
U32 xdata E2ulDesignCap			;//	_at_	SYS_PARA_MAP_ADDR+22;
U32 xdata E2ulFCC				;//	_at_	SYS_PARA_MAP_ADDR+26;
U32 xdata E2ulCycleThreshold	;//	_at_	SYS_PARA_MAP_ADDR+30;
U16 xdata E2uiCycleCount		;//	_at_	SYS_PARA_MAP_ADDR+34;
U16 xdata E2uiNearFCC			;//	_at_	SYS_PARA_MAP_ADDR+36;
S16 xdata E2siLearnLowTemp		;//	_at_	SYS_PARA_MAP_ADDR+38;
S16 xdata E2siDfilterCur		;//	_at_	SYS_PARA_MAP_ADDR+40;
U8  xdata E2ucSleepDelay		;//	_at_	SYS_PARA_MAP_ADDR+42;
U8  xdata E2ucIdleDelay			;//	_at_	SYS_PARA_MAP_ADDR+43;
U8  xdata E2ucCommOffDelay		;//	_at_	SYS_PARA_MAP_ADDR+44;
U8  xdata E2ucChgBKDelay		;//	_at_	SYS_PARA_MAP_ADDR+45;
S16 xdata E2siChgBKCur			;//	_at_	SYS_PARA_MAP_ADDR+46;
U8  xdata E2ucRTCBKDelay		;//	_at_	SYS_PARA_MAP_ADDR+48;
U8  xdata E2ucRamCheckFlg1		;//	_at_	SYS_PARA_MAP_ADDR+49;

//用户自定义A区，子命令号：0x01
U16 xdata E2uiSWVersion			;//	_at_	SYSINFO_MAP_ADDR;
U16 xdata E2uiHWVersion			;//	_at_	SYSINFO_MAP_ADDR+2;
U8  xdata E2ucID				;//	_at_	SYSINFO_MAP_ADDR+4;
U8  xdata E2ucMNFName[12]		;//	_at_	SYSINFO_MAP_ADDR+5;
U32 xdata E2ulMNFDate			;//	_at_	SYSINFO_MAP_ADDR+17;
U16 xdata E2uiSerialNum			;//	_at_	SYSINFO_MAP_ADDR+21;
U8  xdata E2ucDeviceName[12]	;//	_at_	SYSINFO_MAP_ADDR+23;
U8  xdata E2ucDeviceChem[12]	;//	_at_	SYSINFO_MAP_ADDR+35;
U16 xdata E2uiChemID			;//	_at_	SYSINFO_MAP_ADDR+47;
U8  xdata E2ucRamCheckFlg2		;//	_at_	SYSINFO_MAP_ADDR+49;

//充电参数，子命令号：0x02
U16 xdata E2uiChgEndVol			;//	_at_	CHG_PARA_MAP_ADDR;
S16 xdata E2siChgEndCur			;//	_at_	CHG_PARA_MAP_ADDR+2;
U8  xdata E2ucChgEndDelay		;//	_at_	CHG_PARA_MAP_ADDR+4;
U8  xdata E2ucRamCheckFlg3		;//	_at_	CHG_PARA_MAP_ADDR+5;

//平衡参数，子命令号：0x08
U16 xdata E2uiBalanceVol		;//	_at_	BAL_PARA_MAP_ADDR;
U16 xdata E2uiBalanceVolDiff	;//	_at_	BAL_PARA_MAP_ADDR+2;
S16 xdata E2siBalanceCur		;//	_at_	BAL_PARA_MAP_ADDR+4;
U8  xdata E2ucBalanceDelay		;//	_at_	BAL_PARA_MAP_ADDR+6;
U8  xdata E2ucRamCheckFlg4		;//	_at_	BAL_PARA_MAP_ADDR+7;

//放电参数，子命令号：0x03
U16 xdata E2uiDsgEndVol			;//	_at_	DSG_PARA_MAP_ADDR;
U8  xdata E2ucDsgEndDelay		;//	_at_	DSG_PARA_MAP_ADDR+2;
U8  xdata E2ucRamCheckFlg5		;//	_at_	DSG_PARA_MAP_ADDR+3;

//AFE参数，子命令号：0x0A
U8  xdata ucMTPBuffer[26]		;//	_at_	AFE_PARA_MAP_ADDR;
U8  xdata E2ucRamCheckFlg6		;//	_at_	AFE_PARA_MAP_ADDR+26;

//校准参数，子命令号：0x0B
U16 xdata E2uiVPackGain			;//	_at_	CALI_PARA_MAP_ADDR;		
S16 xdata E2siCadcGain			;//	_at_ 	CALI_PARA_MAP_ADDR+2;
S16 xdata E2siCadcZero			;//	_at_	CALI_PARA_MAP_ADDR+4;
S16 xdata E2siTempe1Offset		;//	_at_	CALI_PARA_MAP_ADDR+6;
S16 xdata E2siTempe2Offset		;//	_at_	CALI_PARA_MAP_ADDR+8;
S16 xdata E2siTempe3Offset		;//	_at_	CALI_PARA_MAP_ADDR+10;
U8  xdata E2ucCalibrated		;//	_at_	CALI_PARA_MAP_ADDR+12;
U8  xdata E2ucRamCheckFlg7		;//	_at_	CALI_PARA_MAP_ADDR+13;

//Reserved区占位
//U8 	xdata ucReserved[RESERV_PARA_LEN]	_at_	RESERV_PARA_MAP_ADDR;

//类E2写入OK标志
U16 xdata E2uiCheckFlag			;//	_at_    DATAFLASH_MAP_ADDR+510;


extern unsigned char code dataflash[] ;

void DataMemoryInit(void)
{
	//*****************************XDATA MEMORY START***************************//
	//系统参数，子命令号：0x00
	//U16 xdata E2uiPackConfigMap 	;// _at_	SYS_PARA_MAP_ADDR;
	memcpy((uint8_t*)&E2uiPackConfigMap,&dataflash[SYS_PARA_MAP_ADDR],2);
	//U16 xdata E2uiVOC[10]			;// _at_	SYS_PARA_MAP_ADDR+2;
	memcpy((uint8_t*)&E2uiVOC,&dataflash[SYS_PARA_MAP_ADDR+2],20);
	//U32 xdata E2ulDesignCap 		;// _at_	SYS_PARA_MAP_ADDR+22;
	memcpy((uint8_t*)&E2ulDesignCap,&dataflash[SYS_PARA_MAP_ADDR+22],4);
	//U32 xdata E2ulFCC				;// _at_	SYS_PARA_MAP_ADDR+26;
	memcpy((uint8_t*)&E2ulFCC,&dataflash[SYS_PARA_MAP_ADDR+26],4);
	//U32 xdata E2ulCycleThreshold	;// _at_	SYS_PARA_MAP_ADDR+30;
	memcpy((uint8_t*)&E2ulCycleThreshold,&dataflash[SYS_PARA_MAP_ADDR+30],4);
	//U16 xdata E2uiCycleCount		;// _at_	SYS_PARA_MAP_ADDR+34;
	memcpy((uint8_t*)&E2uiCycleCount,&dataflash[SYS_PARA_MAP_ADDR+34],4);
	//U16 xdata E2uiNearFCC			;// _at_	SYS_PARA_MAP_ADDR+36;
	memcpy((uint8_t*)&E2uiNearFCC,&dataflash[SYS_PARA_MAP_ADDR+36],2);
	//S16 xdata E2siLearnLowTemp		;// _at_	SYS_PARA_MAP_ADDR+38;
	memcpy((uint8_t*)&E2siLearnLowTemp,&dataflash[SYS_PARA_MAP_ADDR+38],2);
	//S16 xdata E2siDfilterCur		;// _at_	SYS_PARA_MAP_ADDR+40;
	memcpy((uint8_t*)&E2siDfilterCur,&dataflash[SYS_PARA_MAP_ADDR+40],2);
	//U8	xdata E2ucSleepDelay		;// _at_	SYS_PARA_MAP_ADDR+42;
	memcpy((uint8_t*)&E2ucSleepDelay,&dataflash[SYS_PARA_MAP_ADDR+42],1);
	//U8	xdata E2ucIdleDelay 		;// _at_	SYS_PARA_MAP_ADDR+43;
	memcpy((uint8_t*)&E2ucIdleDelay,&dataflash[SYS_PARA_MAP_ADDR+43],1);
	//U8	xdata E2ucCommOffDelay		;// _at_	SYS_PARA_MAP_ADDR+44;
	memcpy((uint8_t*)&E2ucCommOffDelay,&dataflash[SYS_PARA_MAP_ADDR+44],1);
	//U8	xdata E2ucChgBKDelay		;// _at_	SYS_PARA_MAP_ADDR+45;
	memcpy((uint8_t*)&E2ucChgBKDelay,&dataflash[SYS_PARA_MAP_ADDR+45],1);
	//S16 xdata E2siChgBKCur			;// _at_	SYS_PARA_MAP_ADDR+46;
	memcpy((uint8_t*)&E2siChgBKCur,&dataflash[SYS_PARA_MAP_ADDR+46],2);
	//U8	xdata E2ucRTCBKDelay		;// _at_	SYS_PARA_MAP_ADDR+48;
	memcpy((uint8_t*)&E2ucRTCBKDelay,&dataflash[SYS_PARA_MAP_ADDR+48],1);
	//U8	xdata E2ucRamCheckFlg1		;// _at_	SYS_PARA_MAP_ADDR+49;
	memcpy((uint8_t*)&E2ucRamCheckFlg1,&dataflash[SYS_PARA_MAP_ADDR+49],1);
	
	//用户自定义A区，子命令号：0x01
	//U16 xdata E2uiSWVersion 		;// _at_	SYSINFO_MAP_ADDR;
	memcpy((uint8_t*)&E2uiSWVersion,&dataflash[SYSINFO_MAP_ADDR],2);
	//U16 xdata E2uiHWVersion 		;// _at_	SYSINFO_MAP_ADDR+2;
	memcpy((uint8_t*)&E2uiHWVersion,&dataflash[SYSINFO_MAP_ADDR+2],2);
	//U8	xdata E2ucID				;// _at_	SYSINFO_MAP_ADDR+4;
	memcpy((uint8_t*)&E2ucID,&dataflash[SYSINFO_MAP_ADDR+4],1);
	//U8	xdata E2ucMNFName[12]		;// _at_	SYSINFO_MAP_ADDR+5;
	memcpy((uint8_t*)&E2ucMNFName[0],&dataflash[SYSINFO_MAP_ADDR+5],12);
	//U32 xdata E2ulMNFDate			;// _at_	SYSINFO_MAP_ADDR+17;
	memcpy((uint8_t*)&E2ulMNFDate,&dataflash[SYSINFO_MAP_ADDR+17],4);
	//U16 xdata E2uiSerialNum 		;// _at_	SYSINFO_MAP_ADDR+21;
	memcpy((uint8_t*)&E2uiSerialNum,&dataflash[SYSINFO_MAP_ADDR+21],2);
	//U8	xdata E2ucDeviceName[12]	;// _at_	SYSINFO_MAP_ADDR+23;
	memcpy((uint8_t*)&E2ucDeviceName[0],&dataflash[SYSINFO_MAP_ADDR+23],12);
	//U8	xdata E2ucDeviceChem[12]	;// _at_	SYSINFO_MAP_ADDR+35;
	memcpy((uint8_t*)&E2ucDeviceChem[0],&dataflash[SYSINFO_MAP_ADDR+23],12);
	//U16 xdata E2uiChemID			;// _at_	SYSINFO_MAP_ADDR+47;
	memcpy((uint8_t*)&E2uiChemID,&dataflash[SYSINFO_MAP_ADDR+47],2);
	//U8	xdata E2ucRamCheckFlg2		;// _at_	SYSINFO_MAP_ADDR+49;
	memcpy((uint8_t*)&E2ucRamCheckFlg2,&dataflash[SYSINFO_MAP_ADDR+49],1);
	
	//充电参数，子命令号：0x02
	//U16 xdata E2uiChgEndVol 		;// _at_	CHG_PARA_MAP_ADDR;
	memcpy((uint8_t*)&E2uiChgEndVol,&dataflash[CHG_PARA_MAP_ADDR],2);
	//S16 xdata E2siChgEndCur 		;// _at_	CHG_PARA_MAP_ADDR+2;
	memcpy((uint8_t*)&E2siChgEndCur,&dataflash[CHG_PARA_MAP_ADDR+2],2);
	//U8	xdata E2ucChgEndDelay		;// _at_	CHG_PARA_MAP_ADDR+4;
	memcpy((uint8_t*)&E2ucChgEndDelay,&dataflash[CHG_PARA_MAP_ADDR+4],1);
	//U8	xdata E2ucRamCheckFlg3		;// _at_	CHG_PARA_MAP_ADDR+5;
	memcpy((uint8_t*)&E2ucRamCheckFlg3,&dataflash[CHG_PARA_MAP_ADDR+5],1);
	
	//平衡参数，子命令号：0x08
	//U16 xdata E2uiBalanceVol		;// _at_	BAL_PARA_MAP_ADDR;
	memcpy((uint8_t*)&E2uiBalanceVol,&dataflash[BAL_PARA_MAP_ADDR],2);
	//U16 xdata E2uiBalanceVolDiff	;// _at_	BAL_PARA_MAP_ADDR+2;
	memcpy((uint8_t*)&E2uiBalanceVolDiff,&dataflash[BAL_PARA_MAP_ADDR+2],2);
	//S16 xdata E2siBalanceCur		;// _at_	BAL_PARA_MAP_ADDR+4;
	memcpy((uint8_t*)&E2siBalanceCur,&dataflash[BAL_PARA_MAP_ADDR+4],2);
	//U8	xdata E2ucBalanceDelay		;// _at_	BAL_PARA_MAP_ADDR+6;
	memcpy((uint8_t*)&E2ucBalanceDelay,&dataflash[BAL_PARA_MAP_ADDR+6],1);
	//U8	xdata E2ucRamCheckFlg4		;// _at_	BAL_PARA_MAP_ADDR+7;
	memcpy((uint8_t*)&E2ucRamCheckFlg4,&dataflash[BAL_PARA_MAP_ADDR+7],1);
	
	//放电参数，子命令号：0x03
	//U16 xdata E2uiDsgEndVol 		;// _at_	DSG_PARA_MAP_ADDR;
	memcpy((uint8_t*)&E2uiDsgEndVol,&dataflash[DSG_PARA_MAP_ADDR],2);
	//U8	xdata E2ucDsgEndDelay		;// _at_	DSG_PARA_MAP_ADDR+2;
	memcpy((uint8_t*)&E2ucDsgEndDelay,&dataflash[DSG_PARA_MAP_ADDR+2],1);
	//U8	xdata E2ucRamCheckFlg5		;// _at_	DSG_PARA_MAP_ADDR+3;
	memcpy((uint8_t*)&E2ucRamCheckFlg5,&dataflash[DSG_PARA_MAP_ADDR+3],1);
	
	//AFE参数，子命令号：0x0A
	//U8	xdata ucMTPBuffer[26]		;// _at_	AFE_PARA_MAP_ADDR;
	memcpy((uint8_t*)&ucMTPBuffer[0],&dataflash[AFE_PARA_MAP_ADDR],26);
	//U8	xdata E2ucRamCheckFlg6		;// _at_	AFE_PARA_MAP_ADDR+26;
	memcpy((uint8_t*)&E2ucRamCheckFlg6,&dataflash[AFE_PARA_MAP_ADDR+26],1);
	
	//校准参数，子命令号：0x0B
	//U16 xdata E2uiVPackGain 		;// _at_	CALI_PARA_MAP_ADDR; 	
	memcpy((uint8_t*)&E2uiVPackGain,&dataflash[CALI_PARA_MAP_ADDR],2);
	//S16 xdata E2siCadcGain			;// _at_	CALI_PARA_MAP_ADDR+2;
	memcpy((uint8_t*)&E2siCadcGain,&dataflash[CALI_PARA_MAP_ADDR+2],2);
	//S16 xdata E2siCadcZero			;// _at_	CALI_PARA_MAP_ADDR+4;
	memcpy((uint8_t*)&E2siCadcZero,&dataflash[CALI_PARA_MAP_ADDR+4],2);
	//S16 xdata E2siTempe1Offset		;// _at_	CALI_PARA_MAP_ADDR+6;
	memcpy((uint8_t*)&E2siTempe1Offset,&dataflash[CALI_PARA_MAP_ADDR+6],2);
	//S16 xdata E2siTempe2Offset		;// _at_	CALI_PARA_MAP_ADDR+8;
	memcpy((uint8_t*)&E2siTempe2Offset,&dataflash[CALI_PARA_MAP_ADDR+8],2);
	//S16 xdata E2siTempe3Offset		;// _at_	CALI_PARA_MAP_ADDR+10;
	memcpy((uint8_t*)&E2siTempe3Offset,&dataflash[CALI_PARA_MAP_ADDR+10],2);
	//U8	xdata E2ucCalibrated		;// _at_	CALI_PARA_MAP_ADDR+12;
	memcpy((uint8_t*)&E2ucCalibrated,&dataflash[CALI_PARA_MAP_ADDR+12],1);
	//U8	xdata E2ucRamCheckFlg7		;// _at_	CALI_PARA_MAP_ADDR+13;
	memcpy((uint8_t*)&E2ucRamCheckFlg7,&dataflash[CALI_PARA_MAP_ADDR+13],1);
	
	//Reserved区占位
	//U8	xdata ucReserved[RESERV_PARA_LEN]	_at_	RESERV_PARA_MAP_ADDR;
	//memcpy((uint8_t*)&ucReserved,&dataflash[RESERV_PARA_MAP_ADDR],1);
	
	//类E2写入OK标志
	//U16 xdata E2uiCheckFlag 		;// _at_	DATAFLASH_MAP_ADDR+510;
	memcpy((uint8_t*)&E2uiCheckFlag,&dataflash[DATAFLASH_MAP_ADDR+510],2);
	
	
   // Uart0_Printf("00000000000000", 14);
	//Uart0_Printf((uint8_t*)&E2uiVPackGain, 2);

}

