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
//#include "Uart.h"
#include "Calibrate.h"
#include "AfeMtp.h"


U16	code	NTC103AT[161]={32950, 30970, 29124, 27404, 25799, 24770, 23335, 21995, 20743, 19572,
							18850, 17796, 16810, 15885, 15019, 14410, 13633, 12904, 12219, 11576,
							11130, 10550, 10005,  9492,  9009,  8643,  8208,  7798,  7412,  7048,
							 6777,  6447,  6136,  5842,  5564,  5341,  5090,  4852,  4627,  4414,
							 4247,  4053,  3870,  3696,  3532,  3390,  3241,  3099,  2964,  2836,
							 2728,  2611,  2501,  2395,  2295,  2205,  2114,  2027,  1944,  1866,
							 1796,  1724,  1655,  1590,  1527,  1469,  1412,  1357,  1305,  1256,
							 1209,	1163,  1120,  1078,  1038,  1000,   963,   928,   894,   862,
							  831,   801,   772,   745,   719,   694,   670,   646,   624,   603,
							  583,   562,   543,   525,   508,   491,   474,   459,   444,   430,
							  416,   402,   389,   377,   365,   354,   342,   331,   321,   311,
							  302,   292,   283,   275,   267,   259,	251,   243,   236,   229,
							  223,   216,   210,   204,   198,   192,   186,   181,   176,   171,
							  167,   162,   157,   153,   149,   145,   141,   137,   133,   130,
							  127,   123,   120,   117,   114,   111,   108,   105,   102,   100,
							   97,    95,    92,    90,    88,    86,    83,    81,    79,    78,
							   76};


/*******************************************************************************
Function: S16 CalcuTemp(S16 getdata)
Description:  Based on collected voltage ratio to calculate the corresponding temperature(-50~110)
Input:	 	
Output: 
Others:
*******************************************************************************/
#define REF_RES_VAL	(680 + 5*ucMTPBuffer[25])				////resistance = (680 + 5*TR[6~0])*10,  The unit is 10 ohms
U16 CalcuTemp(U16 getdata)
{
	U8	i;
	U16	tempcalcu, temperature;
		
	tempcalcu = (uint32_t)getdata*REF_RES_VAL/(32768-getdata);

	if(tempcalcu >= NTC103AT[0])			//Determine whether the excess temperature resistance range
	{
		temperature = 2731-500;			
	}
	else if(tempcalcu <= NTC103AT[160])
	{
		temperature = 2731+1100;
	}
	else
	{
		i = ucTempeMiddle;
		if(tempcalcu > NTC103AT[i])
		{
			for(i=ucTempeMiddle-1; i>=0; i--)
			{
				if(tempcalcu <= NTC103AT[i])		//NTC103AT[i+1]<resis<NTC103AT[i]
				{
					break;
				}
			}
		}
		else
		{
			for(i=ucTempeMiddle+1; i<160; i++)
			{
				if(tempcalcu > NTC103AT[i])		//NTC103AT[i-1]<resis<NTC103AT[i]
				{
					break;
				}
			}
			i--;
		}
		ucTempeMiddle = i;
		
		temperature = (U16)(ucTempeMiddle-50)*10+(NTC103AT[i]-tempcalcu)*10/(NTC103AT[i]-NTC103AT[i+1])+2731;
	}
	return temperature;
}



/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void CalcuTemperature(void)
{
	U16 tempedata;
	
	if(!bTempNum1 && bTempNum0)					//Support one temperature detection
	{
		tempedata = CalcuTemp(AFE.Temp1)+E2siTempe1Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature1 = tempedata;
        Info.Temperature2 = 2731;
        Info.Temperature3 = 2731;
		//UART_IRQ_ENABLE;
	}
	else if(bTempNum1 && !bTempNum0)			//Support two temperature detection
	{
		tempedata = CalcuTemp(AFE.Temp1)+E2siTempe1Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature1 = tempedata;
		//UART_IRQ_ENABLE;
		
		tempedata = CalcuTemp(AFE.Temp2)+E2siTempe2Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature2 = tempedata;
        Info.Temperature3 = 2731;
		//UART_IRQ_ENABLE;
	}
	else if(bTempNum1 && bTempNum0)				//Support three temperature detection
	{
		tempedata = CalcuTemp(AFE.Temp1)+E2siTempe1Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature1 = tempedata;
		//UART_IRQ_ENABLE;
		
		tempedata = CalcuTemp(AFE.Temp2)+E2siTempe2Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature2 = tempedata;
		//UART_IRQ_ENABLE;
		
		tempedata = CalcuTemp(AFE.Temp3)+E2siTempe3Offset;
		//UART_IRQ_DISABLE;
		Info.Temperature3 = tempedata;
		//UART_IRQ_ENABLE;
	}
}



/*******************************************************************************
Function: 
Description:  
1. Calculate a single battery voltage
2. Calculate the total voltage
Input:	 	
Output: 
Others:
*******************************************************************************/
void CalcuVoltage(void)
{
	U8 i;
	uint32_t temppackvol=0, tempcellvol;
	
	for(i=0; i<ucCellNum; i++)
	{
		tempcellvol = (uint32_t)((uint32_t)AFE.Cell[i]*CALIVOL)/ (uint32_t)E2uiVPackGain;			//Calculate a single battery voltage
		temppackvol += tempcellvol;						//Calculate the total voltage
		//UART_IRQ_DISABLE;
		Info.VCell[i] = tempcellvol;
		//UART_IRQ_ENABLE;
	}
	
	//UART_IRQ_DISABLE;
	Info.Voltage = temppackvol;
	//UART_IRQ_ENABLE;
}


/*******************************************************************************
Function: 
Description:  Get the maximum and minimum voltage
Input:	 	
Output: 
Others:
*******************************************************************************/
void CalcuVolMaxMin(void)
{
	U8 i;

	uiCellVmax = Info.VCell[0];
	uiCellVmin = Info.VCell[0];
	
	for(i=1; i<ucCellNum; i++)
	{
		if(Info.VCell[i] > uiCellVmax)
		{
			uiCellVmax = Info.VCell[i];
		}
		if(Info.VCell[i] < uiCellVmin)
		{
			uiCellVmin = Info.VCell[i];
		}
	}
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
bit GetAFEData(void)
{
	BOOL result;
	
	result = MTPRead(MTP_TEMP1, sizeof(AFE), (U8 xdata *)&AFE);
	return result;
}


/*******************************************************************************
Function: 
Description: First Calculate Temp1/Temp2/Temp3/VCell1-VCell16/Current 
Input:
Output:
Others:
*******************************************************************************/
void BatteryInfoManage(void)
{
	if(GetAFEData())
	{
		CalcuTemperature();
		
		CalcuVoltage();
		
		CalcuVolMaxMin();
	}
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void CurProcess(void)
{
	S16 xdata avecur;

    avecur =  siCurBuf[ucCadcTimeCnt];
	if(!MTPRead(MTP_ADC2, 2, (U8 xdata *)&siCurBuf[ucCadcTimeCnt]))
    {
        siCurBuf[ucCadcTimeCnt] = avecur;
    }
	
	if(++ucCadcTimeCnt >= 4)
	{
	 	ucCadcTimeCnt = 0;
		avecur = ((S32)siCurBuf[0]+siCurBuf[1]+siCurBuf[2]+siCurBuf[3]) >> 2;			//Calculate the average current four consecutive times
	
		//UART_IRQ_DISABLE;
		Info.CurCadc = (S32)CALICUR*(avecur-E2siCadcZero)/E2siCadcGain;
		//UART_IRQ_ENABLE;
	} 

	bDSGING = 0;
	bCHGING = 0;
	if(Info.CurCadc < (-E2siDfilterCur))
	{
		bDSGING = 1;
	}
	else if(Info.CurCadc > E2siDfilterCur)
	{
		bCHGING = 1;	 	
	}
	else
	{
		//UART_IRQ_DISABLE;
		Info.CurCadc = 0;
		//UART_IRQ_ENABLE;
	}
}