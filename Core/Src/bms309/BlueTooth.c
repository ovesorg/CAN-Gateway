/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	Jianyi.Zhang
Version: 	V0.0
Date: 		2019/04/26
History:
	V0.0		2019/04/26		 Preliminary
********************************************************************************/
#include "main.h"
#include "Memory.h"
#include "Led.h"


#define BleLEDOn() 		//IO_LED5=1;
#define BleLEDOff() 	//IO_LED5=0;
/*******************************************************************************
Function: BleOnorOff()
Description: 开启/关闭蓝牙 
Input:	 	
Output: 
Others:
*******************************************************************************/
void BleOnorOff(void)
{
	bBLEOPEN = ~bBLEOPEN;
	//IO_BLEPW = ~bBLEOPEN;
}


/*******************************************************************************
Function: BleShutDown()
Description: 关闭蓝牙 
Input:	 	
Output: 
Others:
*******************************************************************************/
void BleShutDown(void)
{
	bBLEOPEN = 0;
	//IO_BLEPW = 1;
}

/*******************************************************************************
Function: BleDisplay()
Description: LED显示蓝牙是否开启 
Input:	 	
Output: 
Others:
*******************************************************************************/
void BleDisplay(void)
{
	//309 demo板电量显示的LED5复用为蓝牙LED
	if((!bLEDOpen && !bLEDFlg)
		|| !(bLEDNum1 && bLEDNum0)) 	//5段LED显示电量时，LED5优先显示电量
	{
		//蓝牙开启时LED5闪烁1秒灭1秒，闪烁频率2Hz
		if(bBLEOPEN)
		{
			if(bHalfHzFlg)
			{
				if(b2HzFlg)
				{
					BleLEDOn();
				}
				else
				{
					BleLEDOff();
				}
			}
			else
			{
				BleLEDOff();
			}
		}
		else
		{
			BleLEDOff();  //蓝牙关闭时LED5关闭
		}
	}
}

