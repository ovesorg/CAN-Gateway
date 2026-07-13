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
#include "McuFlash.h"
#include "AfeMtp.h"
#include "ExtE2PRomRTC.h"
#include "Calibrate.h"

extern S16 CalcuTemp(S16 getdata);

/*******************************************************************************
Function: CaliPackVol()
Description:  Calibration of the total voltage, Update "E2uiVPackGain"
Input:
Output: 
Others:
*******************************************************************************/
bit CaliPackVol(void)
{
	U8 i;
	uint32_t VPackTemp = 0;
    BOOL result;
	
	result = MTPRead(MTP_CELL1, ucCellNum*2, (U8 xdata *)AFE.Cell);
    if(result)
    {
    	for(i=0; i<ucCellNum; i++)
    	{
    		VPackTemp += AFE.Cell[i];
    	}
    
    	E2uiVPackGain = ((uint32_t)CALIPACKVOL*VPackTemp)/ulExtVPack;
    }	

    return result;
}



/*******************************************************************************
Function: CaliTemp()
Description:  Calibration temperature, update temperature offset
Input: Addr, uiExtTemp1, TempeDiff
Output: 
Others:
*******************************************************************************/
bit CaliTemp(U8 Addr, S16 ExtTemp, S16 xdata *TempeDiff)
{
    BOOL result;
	S16 xdata gettemp;
	S16 tempe;

	result = MTPRead(Addr, 2, (U8 xdata *)&gettemp);
    if(result)
    {
    	tempe = ExtTemp-CalcuTemp(gettemp);
    	
    	if(((tempe-*TempeDiff)<150) && ((tempe-*TempeDiff)>-150))
    	{
    	 	*TempeDiff = tempe;
    	}
    }
	
    return result;	
}

/*******************************************************************************
Function: CaliCur()
Description:  Calibration current, update current gain"E2siCadcGain"
Input:
Output: 
Others:
*******************************************************************************/
bit CaliCur(void)
{
    BOOL result;

	result = MTPRead(MTP_ADC2, 2, (U8 xdata *)&AFE.Cadc);
    if(result)
    {
     	E2siCadcGain = (S32)CALICUR*(AFE.Cadc-E2siCadcZero)/slExtCur;
    }

    return result;
}

/*******************************************************************************
Function: CaliCurOffset()
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
bit CaliCurOffset(void)
{
	return(MTPRead(MTP_ADC2, 2, (U8 xdata *)&E2siCadcZero));
}


/*******************************************************************************
Function: CaliRTC()
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void CaliRTCTime(void)
{
	RTCTime.Year = ucExtRTC[5];
	RTCTime.Month = ucExtRTC[4];
	RTCTime.Date = ucExtRTC[3];
	RTCTime.Hour = ucExtRTC[2];
	RTCTime.Minute = ucExtRTC[1];
	RTCTime.Second = ucExtRTC[0];
	RTCModifyTime();
	bE2PProcessFlg = 1;
	bE2PBKRtc = 1;
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void CaliProcess(void)
{
    BOOL result=1;

	if((ucExtcaliSwitch1 & 0x01) != 0)					//Calibration of the total voltage
	{
		if(!CaliPackVol())
        {
            result = 0;
        }
        else
        {
    		ucExtcaliFlag |= 0x03;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x04) != 0)					//Calibration CADC CUR					
	{
		if(!CaliCur())
        {
            result = 0;
        }
        else
        {
    		ucExtcaliFlag |= 0x04;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x08) != 0)					//Calibration CADC Current offset
	{
		if(!CaliCurOffset())
        {
            result = 0;   
        }
        else
        {
    		ucExtcaliFlag |= 0x08;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x10) != 0)					//Calibration TEMP1
	{
		if(!CaliTemp(MTP_TEMP1, uiExtTemp1, &E2siTempe1Offset))
        {
            result = 0;
        }
        else
        {
    		ucExtcaliFlag |= 0x10;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x20) != 0)					//Calibration TEMP2
	{
		if(!CaliTemp(MTP_TEMP2, uiExtTemp2, &E2siTempe2Offset))
        {
            result = 0;
        }
        else
        {
	    	ucExtcaliFlag |= 0x20;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x40) != 0)					//Calibration TEMP3
	{
		if(!CaliTemp(MTP_TEMP3, uiExtTemp3, &E2siTempe3Offset))
        {
            result = 0;
        }
        else
        {
    		ucExtcaliFlag |= 0x40;
        }
	}
	
	if((ucExtcaliSwitch1 & 0x80) != 0)					//Calibration RTC
	{
		CaliRTCTime();
    	ucExtcaliFlag |= 0x80;
	}
	
	if(bEnEEPRomBK)
	{
		if(ucExtcaliFlag == 0xff)
		{
			goto CaliAllOk;
		}
	}
	else if((ucExtcaliFlag & 0x7f) == 0x7f)
	{
CaliAllOk:
		ucExtcaliFlag = 0;
		bCAL = 1;
		E2ucCalibrated = CALI_FLAG;
	}
	
	ucExtcaliSwitch1 = 0;
	bWrFlashFlg = 1;					//Updated calibration parameters, and written to the flash
  //  UpEepromWithDelay();
    
    if(result)
    {
    	;//UART_SEND_ACK;
    }
    else
    {
    	;//UART_SEND_NACK;
    }
}

