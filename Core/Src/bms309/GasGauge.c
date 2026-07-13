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
#include "GasGauge.h"


/*******************************************************************************
Function:GaugeManage() 
Description: Manage the capacity of the pack  
Input:  NULL	
Output: NULL
Others:
*******************************************************************************/
void GaugeManage(void)
{
	U16 FccDelta;

	if(bCHGING | bSC | bOCC | bOCD1 | bOCD2)						//If the charge or discharge overcurrent
	{
		bVDQ = 0;
	}
	else
	{
		if(!bVDQ && (Info.RC1>(Info.E2ulFCC-E2uiNearFCC)))
		{
			if(!bTempNum1 && bTempNum0)					//Support one temperature detection
			{
				if(Info.Temperature1>E2siLearnLowTemp)
				{
					bVDQ = 1;
					ulFCCCount = 0;					
				}
			}
			else if(bTempNum1 && !bTempNum0)			//Support two temperature detection
			{
				if((Info.Temperature1>E2siLearnLowTemp)
					&& (Info.Temperature2>E2siLearnLowTemp))
				{
					bVDQ = 1;
					ulFCCCount = 0;					
				}
			}
			else if(bTempNum1 && bTempNum0)				//Support three temperature detection
			{
				if((Info.Temperature1>E2siLearnLowTemp)
					&& (Info.Temperature2>E2siLearnLowTemp)
					&& (Info.Temperature3>E2siLearnLowTemp))
				{
					bVDQ = 1;
					ulFCCCount = 0;					
				}				
			}
			else									   //No temperature detection
			{
				bVDQ = 1;
				ulFCCCount = 0;
			}
		}
	}
	
	if(bCHGING)														//charging
	{
		ulRCCharge += Info.CurCadc;
		while(ulRCCharge >= VALUE_mAH)
		{
			ulRCCharge -= VALUE_mAH;
			if(Info.RC1 < Info.E2ulFCC)
			{
				//UART_IRQ_DISABLE;
				Info.RC1++;										//have charged 1mAh
				//UART_IRQ_ENABLE;
			}
			else
			{
				//UART_IRQ_DISABLE;
				Info.RC1 = Info.E2ulFCC;
				//UART_IRQ_ENABLE;
				ulRCCharge = 0;
			}
		}
	}
	else if(bDSGING)													//discharging
	{
		ulRCDischarge += -Info.CurCadc;
		if(ulRCDischarge >= VALUE_mAH)
		{
			while(ulRCDischarge >= VALUE_mAH)
			{
				ulRCDischarge -= VALUE_mAH;
				ulDsgCycleCount++;
				if(Info.RC1 > 0)
				{
					//UART_IRQ_DISABLE;
					Info.RC1--;
					//UART_IRQ_ENABLE;
				}
				if(bVDQ)
				{
					ulFCCCount++;
				}
			}
			
			if(ulDsgCycleCount >= E2ulCycleThreshold)
			{
				ulDsgCycleCount -= E2ulCycleThreshold;
				E2uiCycleCount++;
 			    Info.E2uiCycleCount = E2uiCycleCount;
				bWrFlashFlg = 1;				//write E2uiCycleCount to flash flag
                //UpEepromWithDelay();
			}
		}
	}
	
	if(Info.RC1 < Info.E2ulFCC)						//Calculate RSOC
	{
		//UART_IRQ_DISABLE;
		Info.RSOC = (uint32_t)Info.RC1*100/Info.E2ulFCC;
		//UART_IRQ_ENABLE;
	}
	else
	{
		//UART_IRQ_DISABLE;
		Info.RSOC = 100;
		//UART_IRQ_ENABLE;
	}

	if(bFD)	//Discharge end
	{
		if(bVDQ)														        //E2ulFCC updata
		{
			FccDelta = (Info.E2ulFCC/10)*FCC_UPDATE_PERCENT;
			
			if(ulFCCCount > Info.E2ulFCC+FccDelta)
			{
				ulFCCCount = Info.E2ulFCC + FccDelta;
			}
			else if(ulFCCCount+FccDelta < Info.E2ulFCC)
			{
				ulFCCCount = Info.E2ulFCC - FccDelta;
			}
			//UART_IRQ_DISABLE;
			Info.E2ulFCC = ulFCCCount;											//write E2ulFCC to flash
			//UART_IRQ_ENABLE;
			E2ulFCC = ulFCCCount;
			bVDQ = 0;
			bWrFlashFlg = 1;
        //    UpEepromWithDelay();
		}
		//UART_IRQ_DISABLE;
		if(Info.RSOC > 10)
		{
			Info.RSOC = 10;
		}
		Info.RC1 = Info.E2ulFCC*Info.RSOC/100;
		//UART_IRQ_ENABLE;
	}
	else if(bFC)	//Charge end
	{
		//UART_IRQ_DISABLE;
		Info.RC1 = Info.E2ulFCC;
		Info.RSOC = 100;
		//UART_IRQ_ENABLE;
	}
	
	//UART_IRQ_DISABLE;
	if(bLV)																			//OverVoltage Protect
	{
		Info.RC1 = 0;																
		Info.RSOC = 0;
		if(!bLVBkFlg)
		{
			bLVBkFlg = 1;
			bE2PProcessFlg = 1;				//backup battery info when bLV
			bE2PBKDsgEnd = 1;
		}
	}
	else if(bHV)																	//UnderVoltage Protect
	{
		Info.RC1 = Info.E2ulFCC;
		Info.RSOC = 100;
	}
	else
	{
		bLVBkFlg = 0;
	}
	//UART_IRQ_ENABLE;
}




/*******************************************************************************
Function:InitGasGauge() 
Description: Calculate the remaining capacity according to pack voltage  
Input:  NULL	
Output: NULL
Others:
*******************************************************************************/
void InitGasGauge(void)
{
	U8 i;
	
	Info.RSOC = 100;
	for(i=0; i<10; i++)
	{
		if(Info.Voltage < E2uiVOC[i]*ucCellNum)
		{
			if(i == 0)
			{
				if(Info.Voltage < E2uiDsgEndVol*ucCellNum)
				{
					Info.RSOC = 0;	
				}
				else
				{
				 	Info.RSOC = (uint32_t)(Info.Voltage-E2uiDsgEndVol*ucCellNum)*10/((E2uiVOC[0]-E2uiDsgEndVol)*ucCellNum);	
				}
				break;
			}
			else
			{
				Info.RSOC = 10*i + (uint32_t)(Info.Voltage-E2uiVOC[i-1]*ucCellNum)*10/((E2uiVOC[i]-E2uiVOC[i-1])*ucCellNum);
				break;
			}
		}
	}

	Info.RC1 = E2ulFCC*Info.RSOC/100;
}


