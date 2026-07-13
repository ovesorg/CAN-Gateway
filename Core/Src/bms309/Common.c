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


/*******************************************************************************
Function: Delay1ms()
Description: system clock = 24MHz
Input: NULL
Output: NULL
Others: 特别注意该函数的延时准确性问题，在不同优化等级时，反汇编不同造成延时不一致，尚未实测

*******************************************************************************/
void Delay1ms(U8 delaycnt)
{
	/*U8 i;
	U16 j;
	
	for(i=0; i<delaycnt; i++)				//system clock = 24MHz
	{
		for(j=0; j<1670; j++)
		{
		}
	}*/
	HAL_Delay(delaycnt);
}


/*******************************************************************************
Function: MemorySet()
Description:
Input:	pt--memory指针
		setval---需要赋值的数据
		length---需要赋值的memory长度(Byres)
Output: 
Others:
*******************************************************************************/
void MemorySet(U8 xdata *pt, U8 setval, U8 length)
{
	U8 i;
	for(i=0; i<length; i++)
	{
		*pt = setval;
		pt++;
	}
}


/*******************************************************************************
Function: MemoryCopy()
Description:
Input:	source--源Memory指针
		target---目的Memory指针
		length---需要拷贝的数据长度(Byres)
Output: 
Others:
*******************************************************************************/
void MemoryCopy(U8 xdata *source, U8 xdata *target, U8 length)
{
	U8 i;
	for(i=0; i<length; i++)
	{
		*target = *source;
		target++;
		source++;
	}
}


/*******************************************************************************
Function: IntoIdle()
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void IntoIdle(void)
{
	/*SUSLO = 0x55;
	PCON |= 0x01;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();*/
}

/*******************************************************************************
Function: IntoSleep()
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void IntoSleep(void)
{
	/*SUSLO = 0x55;
	PCON |= 0x02;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();*/
}


/*******************************************************************************
Function: ResetInit()
Description:MCU程序复位，从0地址开始执行
Input:	 	
Output: 
Others:
*******************************************************************************/
void ResetInit(void)
{	
 /*	IEN0 = 0x00;						//Disable Interrupt
	IEN1 = 0x00;	
	TCON = 0x00;
	EXF0 = 0x00;
	IENC = 0x00;

	SBRTH = 0x00;						//Disable UART0
	SBRTL = 0x00;
	SCON = 0x00;
	
	((void(code*)(void))0x0000)();		*/
}


/*******************************************************************************
Function: RamCheckProcess()
Description: 检查RamCheck标志是否正常，不正常则重新初始化XRAM的DataFlash数据

Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void RamCheckProcess(void)
{
    if((E2ucRamCheckFlg1  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg2  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg3  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg4  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg5  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg6  != RAM_CHECK_DATA)
    || (E2ucRamCheckFlg7  != RAM_CHECK_DATA)
    || (E2uiCheckFlag != 0x5AA5))
    {
        InitSysPara();
    }
}
