/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	andyliu
Version: 	V0.0
Date: 		2014/05/30
History:
	V0.0		2014/05/30		 Preliminary
********************************************************************************/
#include "main.h"
#include "memory.h"
#include "Led.h"


/*******************************************************************************
Function: LEDPowerOn(), LEDPowerOff()
Description:  By LED displays battery capacity, long display time 4s
	bLEDNum1,0: The total number of LED
	Info.RSOC:   The percentage of remaining capacity
Input:
Output: 
Others:
*******************************************************************************/
void LEDPowerOn(void)
{	
	if(!bLEDNum1 && bLEDNum0)					//3'LED
	{
		if(Info.RSOC > 60)
		{
			LEDOn3();
		}
		else if(Info.RSOC > 20)
		{
			LEDOn2();
		}
		else if(Info.RSOC > 0)
		{
			LEDOn1();
		}
		else
		{
			LEDOff();
		}
	}
	else if(bLEDNum1 && !bLEDNum0)			//4'LED
	{
		if(Info.RSOC > 75)
		{
			LEDOn4();
		}
		else if(Info.RSOC > 50)
		{
			LEDOn3();
		}
		else if(Info.RSOC > 25)
		{
			LEDOn2();
		}
		else if(Info.RSOC > 0)
		{
			LEDOn1();
		}
		else
		{
			LEDOff();
		}
	}
	else if(bLEDNum1 && bLEDNum0)			//5'LED
	{
		if(Info.RSOC > 80)
		{
			LEDOn5();
		}
		else if(Info.RSOC > 60)
		{
			LEDOn4();
		}
		else if(Info.RSOC > 40)
		{
			LEDOn3();
		}
		else if(Info.RSOC > 20)
		{
			LEDOn2();
		}
		else if(Info.RSOC > 0)
		{
			LEDOn1();
		}
		else
		{
			LEDOff();
		}
	}
}

void LEDPowerOff(void)
{
	if(bLEDFlg)						//LED display time counting
	{
		if(++ucLEDTimeCnt >= 4)
		{
			ucLEDTimeCnt = 0;
			bLEDFlg = 0;
			LEDOff();
		}
	}
}

