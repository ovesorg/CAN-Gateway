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
#include "Led.h"
#include "AfeMtp.h"
#include "Common.h"
#include "Calibrate.h"
//#include "Uart.h"

extern void InitClk(void);
extern void AFERdFlag(void);
extern void	BleShutDown();

/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void WakeUpProcess(void)
{
	U8 xdata temp= 0x00;

	if(bSleepFlg || bIdleFlg)
	{
		InitClk();

		ucIdleTimeCnt = 0x00;
		ucUartTimeCnt = 0x00;
		bPCSleepFlg = 0;
		bSleepFlg = 0;
		bIdleFlg = 0; 
		ucTimer0Cnt = 0;
		bTimerFlg = 0;
		//ucUartTimeoutCnt = 0;
	
		//IEN0 = 0x03;								//disable uart wakeup
		//UART_IRQ_ENABLE;							//enable uart int

		bWakeupFlg = 0;

		MTPWrite(MTP_BFLAG2, 1, &temp);			//clear 309 WAKE_FLG
		MTPWrite(MTP_BFLAG1, 1, &temp);			//clear 309 Protect flag
		ucMTPConfVal |= 0x0c;
		ucMTPConfVal = (ucMTPConfVal&0xFC)|0x0C;
		MTPWrite(MTP_CONF, 1, &ucMTPConfVal);				//Open function block,include 309 WDT,CADC															
	}
}

/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void SystemIntoSleep(void)
{	/*
	ucMTPConfVal = (ucMTPConfVal&~0x0c) | 0x02;
 	MTPWrite(MTP_CONF,  1,  &ucMTPConfVal);	//309 into Sleep  auto close WDT,CADC,

	BleShutDown();							//关闭蓝牙

	LEDOff();								//shut down LED

	UART_IRQ_DISABLE;
	bAFEFlg = 0;							//309通讯进Sleep时可能来Alarm，忽略此Alarm的置位

	EA = 0;
	CLKCON = CLKCON | 0x80;					//32K_spdup=1
	CLKCON &= ~0x04;					//Set sysclk 32kHz
	_nop_();
	CLKCON &= ~0x08;					//Close  sysclk 24MHz
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	EA = 1;
	
	BANK1_SEL;
	T3CON &= ~0x04;		//关闭定时器3
	BANK0_SEL;

 	bSleepFlg = 1;

	IntoSleep();							//MCU into Sleep
	
	BANK1_SEL;
	T3CON |= 0x04;		//启动定时器3
	BANK0_SEL;	*/
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void SystemIntoIdle(void)    
{	
	/*ucMTPConfVal = (ucMTPConfVal&~0x0c) | 0x01;
	MTPWrite(MTP_CONF, 1, &ucMTPConfVal);	//309 into IDLE, aoto close 309 WDT,CADC
	
	LEDOff();								//shut down LED

	UART_IRQ_DISABLE;

	bAFEFlg = 0;							//309通讯进IDLE时可能来Alarm，忽略此Alarm的置位
	bCADCFlg = 0;
	bIdleTimerFlg = 0;

	EA = 0;
	CLKCON &= ~0x04;					//Set sysclk 32kHz
	_nop_();
	CLKCON &= ~0x08;					//Close  sysclk 24MHz
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	EA = 1;
	
	bIdleFlg = 1;

	IntoIdle();								//MCU into IDLE*/
}

/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void IntoShutDown(void)
{
	/*S16 xdata AveVCurTemp=0;
	S16 xdata AveVCur=0;
	S16 xdata VCurBuf[4];
	U8  xdata VadcTimeCnt=0;
    BOOL tempflg = 0;

	if((!bSleepFlg)&&(!bIdleFlg))
	{
		EA = 0;
		if(!(bWakeupFlg | bCADCFlg | bCalibrationFlg | bWrFlashFlg | bLEDOpen | bISPFlg | bTimerFlg)
			 && (ucResetFlag!=0x12))
		{
			EA = 1;
			IntoIdle();
		}
		EA = 1;	 	
	}
	else
	{
		while(1)
		{
			RSTSTAT = RSTSTAT;
						
			if(bAFEFlg)
			{
				bAFEFlg = 0;
				AFERdFlag();
			}

		 	if(bWakeupFlg)
			{
			 	break;
			}
			else if(bProtectFlg)
			{
			 	bProtectFlg = 0;
				bWakeupFlg = 1;
				break;
			}

			if((bSleepFlg)&&(!bWakeupFlg))
			{
				tempflg = 0;
                SystemIntoSleep();			 	
			}
			
			if(bIdleTimerFlg)						//MCU&309进入IDLE后过5s，MCU打开309的CADC
			{
				if(!tempflg)
				{
					tempflg = 1;
					InitClk();
					ucMTPConfVal = (ucMTPConfVal&0xF8)|0x08;
					MTPWrite(MTP_CONF, 1, &ucMTPConfVal);			//Open CADC
				}

				if(bCADCFlg)
				{ 
					bCADCFlg = 0;
					MTPRead(MTP_ADC2, 2, (U8 xdata *)&VCurBuf[VadcTimeCnt]);

					if(++VadcTimeCnt >= 4)
					{
						VadcTimeCnt = 0;
						bIdleTimerFlg = 0;

						AveVCurTemp = ((S32)VCurBuf[0]+VCurBuf[1]+VCurBuf[2]+VCurBuf[3]) >> 2;
						AveVCur = (S32)CALICUR*(AveVCurTemp-E2siCadcZero)/E2siCadcGain;
														
						if((AveVCur<(-E2siDfilterCur)) || (AveVCur>E2siDfilterCur))
						{
							bWakeupFlg = 1;
							break;															 	
						}
						else
						{
							tempflg = 0;
                            SystemIntoIdle();							 	
						}																					 	
					}											 	
				}
				else
				{
					 IntoIdle();
				} 										
			}
			else if(bIdleFlg)
			{
			 	IntoIdle();
			}
		}
	}*/
}

/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void ShutDownProcess(void)
{
	/*if(!bLEDOpen && !bLEDFlg)					//LED display off and then wait to enter a low-power
	{
		if(++ucUartTimeCnt >= E2ucCommOffDelay)
		{
			ucUartTimeCnt = E2ucCommOffDelay;		
		}
		if(!bCHGING && !bDSGING)
		{
			if(uiBatStatus == 0x0000)
			{
				if(++ucIdleTimeCnt >= E2ucIdleDelay)
				{
					ucIdleTimeCnt = E2ucIdleDelay;
				}
			}
			else
			{
				ucIdleTimeCnt = 0;
			}

			if(bPCSleepFlg && bUartSndAckFlg)
			{
				SystemIntoSleep();
			}
			else if(ucUartTimeCnt >= E2ucCommOffDelay)
			{
				if(bLV)
				{
					SystemIntoSleep();
				}
				else if((ucIdleTimeCnt >= E2ucIdleDelay) && !bBLEOPEN)
				{
					SystemIntoIdle();
				}
			}
		}
		else
		{
			bPCSleepFlg = 0;
			ucIdleTimeCnt = 0;
		}
	} */
}
























