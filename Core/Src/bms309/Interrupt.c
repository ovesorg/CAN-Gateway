/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	andyliu
Version: 	V0.0
Date: 		2014/05/30
History:
	V0.0		2014/05/30		 Preliminary
********************************************************************************/
#include "Main.h"
#include "Memory.h"
#include "AfeMtp.h"

/*******************************************************************************
Function: InterruptINT4() 
Description:响应309 Alarm信号  
Input:	 	
Output: 
Others:
*******************************************************************************/
/*void InterruptINT4(void) interrupt 10
{

	_push_(INSCON);
    BANK0_SEL;
	if(IF45)		
	{
		IF45 = 0;
		if(bSleepFlg||bIdleFlg)	 //通过主控309的Alarm来进行唤醒
		{
			InitClk();
		}
		bAFEFlg = 1;

	}
	_pop_(INSCON);
}*/


/*******************************************************************************
Function: InterruptINT3() 
Description: 响应KEY_LED按键唤醒、亮LED 
Input:	 	
Output: 
Others:
*******************************************************************************/
/*void InterruptINT3(void) interrupt 11
{
	_push_(INSCON);
    BANK0_SEL;

	if(bSleepFlg||bIdleFlg)
	{
		InitClk();	 	
	}
 	if(!IO_KEY)
	{
		//原1612时钟16.6M，_nop_();为四个
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		//现6441时钟24M，增加两个_nop_();
		_nop_();	
		_nop_();
		if(!IO_KEY)
		{
			bWakeupFlg = 1;

			if(!bLEDOpen)
			{
				bLEDOpen = 1;
			}

		}
	}

	_pop_(INSCON);
}*/


/*******************************************************************************
Function: InterruptTimer3()
Description:  定时器3中断响应，周期20ms
Input:	 	
Output: 
Others:
*******************************************************************************/
/*void InterruptTimer3(void)	interrupt	5
{
	_push_(INSCON);
    BANK0_SEL;
	if(++ucTimer0Cnt >= 50)			//Timer0 = 1S
	{
	 	ucTimer0Cnt = 0;
		bTimerFlg = 1;
		bHalfHzFlg = ~bHalfHzFlg;
		b2HzFlg	= 0;
	}
	else if(ucTimer0Cnt == 25)		//Timer0 = 0.5s
	{
		b2HzFlg = 1;
	}
    
    if(ucUpDataLimitTime > 0)
    {
        ucUpDataLimitTime--;
    }
	
	if(++ucUartTimeoutCnt >= 10)		//If not Uart communication within 200ms, then clear 
	{
		REN = 1;	
		ucUartBufPT = 0;
	}

	if(bIdleFlg)
	{
		if(++ucTimer0Cnt1 >= 250)
		{
			ucTimer0Cnt1 = 0;
			bIdleTimerFlg = 1;		 	
		}
	}

	if(!IO_KEY)						   //按键扫描
    {
        if(!bLongKeyFlg)
        {
            if(++ucKeyDownCnt >= 250)  //按键长按5S
            {
                ucKeyDownCnt = 0;
                bLongKeyFlg = 1;

				bBleOnOffFlg = 1;
            }
        }
        else
        {
            ucKeyDownCnt = 0;
        }
    }
    else
    {
        bLongKeyFlg = 0;               //确保在下次长按时，可以重新识别长按键
        ucKeyDownCnt = 0;
    }

	_pop_(INSCON);
}*/




