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
#include "AfeMtp.h"
#include "TwiIO.h"
//#include "TwiModule.h"
#include "Common.h"
#include "ExtE2PRomRTC.h"
#include "McuFlash.h"
#include "GasGauge.h"
//#include "Uart.h"
//#include "ISP.h"
//#include "Main.h"


/*******************************************************************************
Function:InitSealCalibrate() 
Description:  
Input: 	
Output: 
Others:
*******************************************************************************/
void InitSealCalibrate(void)
{
	if(E2ucCalibrated == CALI_FLAG)				//Initialize the Calibration flag
	{
		bCAL = 1;
	}
	else
	{
		bCAL = 0;
	}
}



/*******************************************************************************
Function:InitVar() 
Description:  
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitVar(void)
{
	U8 i;

	bUartSndAckFlg = 0;
	bUartNeedAckFlg = 0;
	ucUartTimeCnt = 0;
	ucIdleTimeCnt = 0;
	ucCadcTimeCnt = 0;
	ucChgEndTimeCnt = 0;
	ucChgEndRTimeCnt = 0;
	ucDsgEndTimeCnt = 0;
	ucDsgEndRTimeCnt = 0;
	for (i=0; i<16; i++)
	{
		ucBalanceTimeCnt[i] = 0;
	}

	ucBalUpdateTimeCnt = 0;
	uiBalanceChannel = 0;
	ucLEDTimeCnt = 0;
	bLEDOpen = 0;
	bLEDFlg = 0;
	bLongKeyFlg = 0;
	bBleOnOffFlg = 0;
	bHalfHzFlg = 0;
	b2HzFlg = 0;
	bLVBkFlg = 0;
	bCHGClosedFlg = 0;
	bDSGClosedFlg = 0;
	bSleepFlg = 0;
	bIdleFlg = 0;
	bPCSleepFlg = 0;
	ucMTPConfVal = 0x70;						//Enable CHG.DSG,PCHG
	
	MemorySet((U8 xdata *)Info.VCell, 0, sizeof(Info.VCell));	//clr Info.VCell[]
	Info.CurCadc = 0;
	Info.Temperature1 = 2731;				//0
	Info.Temperature2 = 2731;				//0
	Info.Temperature3 = 2731;				//0
}



/*******************************************************************************
Function: InitSysPara()
Description: DataFlash Data write to XRAM  
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitSysPara(void)
{	
	/*if(!ReadMcuFlash(DATAFLASH_ADDR, DATAFLASH_MAP_ADDR, 512))                      	
    {
        if(!ReadMcuFlash(DATAFLASH_ADDR2, DATAFLASH_MAP_ADDR, 512))         
        {
			while(1)
			{
				RSTSTAT = RSTSTAT;
				CLKCON &= ~0x04;					//Set sysclk 32kHz
				_nop_();
				_nop_();
				_nop_();
				_nop_();
				CLKCON &= ~0x08;					//Close  sysclk 24MHz
				IntoSleep();
			}
		}
		else
		{
			UpEepromNoDelay();
            if(!WriteMcuFlash(DATAFLASH_ADDR))                                 
            {
				UpEepromNoDelay();
                WriteMcuFlash(DATAFLASH_ADDR);	
            }
        }
    }
    else
	{
		FLASHCON = 0x01;
		if(CWORD[DATAFLASH_OK_FLG2_ADDR/2] != 0x5AA5)
		{
			UpEepromNoDelay();
			if(!WriteMcuFlash(DATAFLASH_ADDR2))
			{
				UpEepromNoDelay();
				WriteMcuFlash(DATAFLASH_ADDR2);	
			}
		}
		FLASHCON = 0x00;	
	}*/

	DataMemoryInit();
    
	ucFlashWrValid = 0x00;

	uiPackConfig = E2uiPackConfigMap;
	Info.CurCadc = 0;								//After the program is reset, the current default is "0",update after 1'seconds
	Info.E2ulFCC = E2ulFCC;
	Info.E2uiCycleCount = E2uiCycleCount;
	Info.PackStatus = uiPackStatus;
	Info.BatStatus = uiBatStatus;
	Info.PackConfig = uiPackConfig;
}


/*******************************************************************************
Function: InitIRQ() 
Description: Init EUART0,INT4,INT3,TIMER PCA0 Interrupt  
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitIRQ(void)
{
 	/*IEN0 = 0x30;						//Enable UART0,Timer3
	IEN1 = 0x18;						//Enable INT4,INT3
	IENC = 0x20;						//Enable INT45
	
	TCON = 0x05;						//Exint0/1：Falling edge trigger
	EXF0 = 0x54;						//Exint4/3/2：Falling edge trigger
	
	IPL0 = 0x10;
	IPH0 = 0x10;
	IPL1 = 0x00;
	IPH1 = 0x00;						//set uart priority = 3, other interrupt priority  = 0

	BANK1_SEL;
	T3CON |= 0x04;						//启动定时器3
	BANK0_SEL;

	EA = 1;*/
}


/*******************************************************************************
Function: InitTimer()
Description: Init Timer PCA0,Set Timer PCA0 time is 20ms 
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitTimer(void)
{
	/*BANK1_SEL;
	T3CON = 0x02;		//外部32.768kHz/128kHz为时钟源，1分频
	TL3 = 0x70;		 
	TH3 = 0xFD;			//20mS
	BANK0_SEL;*/
}


/*******************************************************************************
Function: InitGPIO()
Description:
    P0.7[TXD],  P0.6[RXD],  P0.5[SCL],  P0.4[SDA],  P0.3[],     P0.2[],     P0.1[KEY],  P0.0[BLEPW], 
    P0.7ST[1],  P0.6ST[1],  P0.5ST[1],  P0.4ST[1],  P0.3ST[0],  P0.2ST[0],  P0.1ST[1],  P0.0ST[1],  P0 = 0xf3;
    P0.7CR[1],  P0.6CR[0],  P0.5CR[0],  P0.4CR[0],  P0.3CR[1],  P0.2CR[1],  P0.1CR[0],  P0.0CR[1],  P0CR = 0x8d;
    P0.7PC[1],  P0.6PC[1],  P0.5PC[1],  P0.4PC[1],  P0.3PC[0],  P0.2PC[0],  P0.1PC[1],  P0.0PC[0],  P0PCR = 0xf2;

    P1.7[RST],  P1.6[],     P1.5[TCK],  P1.4[TDI],  P1.3[TMS],  P1.2[TDO],  P1.1[T1],   P1.0[T2], 
    P1.7ST[0],  P1.6ST[0],  P1.5ST[0],  P1.4ST[0],  P1.3ST[0],  P1.2ST[0],  P1.1ST[0],  P1.0ST[0],  P1 = 0x00;
    P1.7CR[1],  P1.6CR[1],  P1.5CR[1],  P1.4CR[1],  P1.3CR[1],  P1.2CR[1],  P1.1CR[1],  P1.0CR[1],  P1CR = 0xff;
    P1.7PC[0],  P1.6PC[0],  P1.5PC[0],  P1.4PC[0],  P1.3PC[0],  P1.2PC[0],  P1.1PC[0],  P1.0PC[0],  P1PCR = 0x00;

    P2.7[T3],   P2.6[ALARM],P2.5[],		P2.4[LED5], P2.3[LED4], P2.2[LED3], P2.1[LED2], P2.0[LED1], 
    P2.7ST[0],  P2.6ST[1],  P2.5ST[0],  P2.4ST[0],  P2.3ST[0],  P2.2ST[0],  P2.1ST[0],  P2.0ST[0],  P2 = 0x40;
    P2.7CR[1],  P2.6CR[0],  P2.5CR[1],  P2.4CR[1],  P2.3CR[1],  P2.2CR[1],  P2.1CR[1],  P2.0CR[1],  P2CR = 0xbf;
    P2.7PC[0],  P2.6PC[1],  P2.5PC[0],  P2.4PC[0],  P2.3PC[0],  P2.2PC[0],  P2.1PC[0],  P2.0PC[0],  P2PCR = 0x40;

    P3.7[],     P3.6[],     P3.5[],		P3.4[XTAL1],P3.3[XTAL2],P3.2[VPRO],	P3.1[], 	P3.0[], 
    P3.7ST[0],  P3.6ST[0],  P3.5ST[0],  P3.4ST[0],  P3.3ST[0],  P3.2ST[0],  P3.1ST[0],  P3.0ST[0],  P3 = 0x00;
    P3.7CR[1],  P3.6CR[1],  P3.5CR[1],  P3.4CR[1],  P3.3CR[1],  P3.2CR[1],  P3.1CR[1],  P3.0CR[1],  P3CR = 0xff;
    P3.7PC[0],  P3.6PC[0],  P3.5PC[0],  P3.4PC[0],  P3.3PC[0],  P3.2PC[0],  P3.1PC[0],  P3.0PC[0],  P3PCR = 0x00;
Input:	NULL
Output: NULL
Others:
*******************************************************************************/
void InitGPIO(void)
{
	/*P0 = 0xf3;
	P0PCR = 0xf2;
	P0CR = 0x8d;
	
	P1 = 0x00;
	P1PCR = 0x00;
	P1CR = 0xff;
	
	P2 = 0x40;
	P2PCR = 0x40;
	P2CR = 0xbf;
	
	P3 = 0x00;
	P3PCR = 0x00;
	P3CR = 0xff;
	
	P4 = 0x00;
	P4PCR = 0x00;
	P4CR = 0x00;*/
}


/*******************************************************************************
Function:  ClrRam()
Description: Clear idata space:0x08~STACK_ADDR-1
			 Clear XDATA space:0x0000~0x2FFF  
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void ClrRam(void)
{
 	/*U8 idata *ptr1;
	U8 xdata *ptr2;
	U8 xdata i;
	U16 idata j;

	for(i = 0x08; i <= STACK_ADDR-1; i++)	//clear ram address: 08H~STACK_ADDR-1
	{
		ptr1 = (U8 idata *)i;
		*ptr1 = 0;
	}

	for(j=0; j<=0xAFF; j++)					//clear XDATA address:0000H-0AFFH
	{
		ptr2 = (U8 xdata *)j;
		*ptr2 = 0;
	}*/
}


/*******************************************************************************
Function: InitClk()
Description: Set system clock = 16.6M/12 
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitClk(void)
{
 	/*CLKCON	= 0x08;								//SETB HFON
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	CLKCON |= 0x04;								//SETB	FS, SYSCLK=24M*/
}


/*******************************************************************************
Function:PartialInitial()  
Description:  
Input: 	
Output: 
Others:
*******************************************************************************/
void PartialInitial(void)
{
	InitClk();						//初始化系统时钟

	InitGPIO();						//初始化GPIO

	InitTimer();					//初始化Timer

	//InitUART0();					//初始化UART0模块

	InitTwi();						//初始化TWI模块

	InitSysPara();					//初始化系统参数

	InitVar();						//初始化变量
	
	InitAFE();						//初始化AFE

	UpdataAfeConfig();				//更新AFE E2参数
	
	InitSealCalibrate();			//判断是否做过校准全部参数

	if(bEnEEPRomBK)					//如果支持外部RTC和EEPROM，需要初始化
	{
		InitE2PRom();
		InitRTC();
	}

	EnableAFEWdtCadc();				//使能AFE的 CADC & WDT

	InitIRQ();						//初始化中断

	BleShutDown();					//蓝牙默认关闭
}

/*******************************************************************************
Function: AllInitial()
Description:  
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void AllInitial(void)
{
	InitClk();						//初始化系统时钟

	InitGPIO();						//初始化GPIO

	ClrRam();						//清空MCU RAM
	
	InitTimer();					//初始化Timer

	//InitUART0();					//初始化UART0模块

	InitTwi();						//初始化TWI模块

	InitSysPara();					//初始化系统参数

	InitVar();						//初始化变量
	
	InitAFE();						//初始化AFE

    UpdataAfeConfig();				//更新AFE E2参数
	
	BatteryInfoManage();			//电池信息管理：获取电压、电流、温度

	InitGasGauge();					//初始化容量信息，RSOC
	
	InitSealCalibrate();			//判断是否做过校准全部参数

	if(bEnEEPRomBK)					//如果支持外部RTC和EEPROM，需要初始化
	{
		InitE2PRom();
		InitRTC();
	}

	EnableAFEWdtCadc();				//使能AFE的 CADC & WDT

	InitIRQ();						//初始化中断

	BleShutDown();					//蓝牙默认关闭
}

/*******************************************************************************
Function: Initial()
Description:  ALLInitial()--Initial all data; PartialInitial()--Initial part data;
Input: 	NULL
Output: NULL
Others:
*******************************************************************************/
void Initial(void)
{
	/*FLASHCON = 0x00;
    if(CWORD[510/2] != 0x5AA5)	//程序检测第一个512B数据最后两个字节是否为0x5AA5，如果不是则认为程序异常，直接进入ISP
	{
        ISPProcess();
	}

	if(((RSTSTAT&0x38)!=0) || (ucResetFlag==0x12))	//POR+LVR+ResetPin+PC instruction
	{
	 	//RSTSTAT	= 0x00;
		ucResetFlag = 0;
		
		AllInitial();
	}
	else			//WDT+OVL复位*/
	{
	 	//RSTSTAT	= 0x00;
		//PartialInitial();
		AllInitial();
	}
}






