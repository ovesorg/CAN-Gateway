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
#include "AfeMtp.h"
#include "Common.h"


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void ProtectProcess(void)
{
	U8 xdata tempbuf[3];

	if(MTPRead(MTP_BSTATUS1, 3, tempbuf))							//read AFE reg 43H~45H
    {
    	uiBatStatus =  ((U16)tempbuf[1]<<8) | tempbuf[0];
    	uiPackStatus = (uiPackStatus&0xfff0) | (tempbuf[2]&0x0F);
		//UART_IRQ_DISABLE;
    	Info.BatStatus = uiBatStatus;
    	Info.PackStatus = uiPackStatus;
	//	UART_IRQ_ENABLE;
    }
}



/*******************************************************************************
Function: VolProcess()
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void VolProcess(void)
{	
	if(bCHGING)
	{
		if(!bFC)
		{
			if((uiCellVmax >= E2uiChgEndVol) && (Info.CurCadc <= E2siChgEndCur))
			{
		 		if(++ucChgEndTimeCnt >= E2ucChgEndDelay)
				{
					ucChgEndRTimeCnt = 0;
			 		bFC = 1;
					if(bCHGEnd)									//Full charge off the charge&precharge MOS
					{
						bCHGClosedFlg = 1;
						ucMTPConfVal &= ~0x50;
						MTPWrite(MTP_CONF, 1, &ucMTPConfVal);			//Close CHG & PCHG MOS
					}
				}
			}
			else
			{
				ucChgEndTimeCnt = 0;
			}
		}
	}
	else if(bDSGING)
	{
		ucChgEndTimeCnt = 0;
		if(bFC)
		{
			bFC = 0;
			if(bCHGEnd && bCHGClosedFlg)
			{
				bCHGClosedFlg = 0;
				ucMTPConfVal |= 0x50;				//Open PCHG & CHG MOS
				MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
			}
		}
	}
	else if(uiCellVmax<E2uiChgEndVol)
	{
		if(++ucChgEndRTimeCnt>=E2ucChgEndDelay)
		{
			ucChgEndTimeCnt = 0;
			if(bFC)
			{
				bFC = 0;
				if(bCHGEnd && bCHGClosedFlg)
				{
					bCHGClosedFlg = 0;
					ucMTPConfVal |= 0x50;				//Open PCHG & CHG MOS
					MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
				}
			}
		}
	}
	else
	{
		ucChgEndTimeCnt = 0;
		ucChgEndRTimeCnt = 0;
	}

	if(bDSGING)
	{
		if(!bFD)
		{
		 	if(uiCellVmin <= E2uiDsgEndVol)
			{
			 	if(++ucDsgEndTimeCnt >= E2ucDsgEndDelay)
				{
					ucDsgEndRTimeCnt = 0;
				 	bFD = 1;
					bE2PProcessFlg = 1;				//backup battery info when discharge end
					bE2PBKDsgEnd = 1;
					if(bDSGEnd)									//Discharge end
					{
						bDSGClosedFlg = 1;
						ucMTPConfVal &= ~0x20;			//Close DSG MOS
						MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
					}
				}
			}
			else
			{
				ucDsgEndTimeCnt = 0;
			}
		}
	}
	else if(bCHGING)
	{
	 	ucDsgEndTimeCnt = 0;
		bFD = 0;
		if(bDSGEnd && bDSGClosedFlg)
		{
			bDSGClosedFlg = 0;
			ucMTPConfVal |= 0x20;					//Open DSG MOS
			MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
		}
	}
	else if(uiCellVmin>E2uiDsgEndVol)
	{
		if(++ucDsgEndRTimeCnt>=E2ucDsgEndDelay)
		{
		 	ucDsgEndTimeCnt = 0;
			bFD = 0;
			if(bDSGEnd && bDSGClosedFlg)
			{
				bDSGClosedFlg = 0;
				ucMTPConfVal |= 0x20;					//Open DSG MOS
				MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
			}
		}
	}
	else
	{
		ucDsgEndTimeCnt = 0;
		ucDsgEndRTimeCnt = 0;
	}
}



/*******************************************************************************
Function: AFECheck()
Description:  Programming mode is only for temporary use, follow Demo Board will solve the problem
Input:	 	
Output: 
Others:
*******************************************************************************/
void AFECheck(void)
{
	U8 xdata tempconf;

	if(MTPRead(MTP_CONF, 3, &tempconf))
	{
		if(tempconf != ucMTPConfVal)
		{
		 	MTPWrite(MTP_CONF, 1, &ucMTPConfVal);	
		}
	}
}

void AFERdFlag(void)
{
	//U8 xdata Bflag=0;
	U8 xdata Bflag[2]={0,0};
	
	if(MTPRead(MTP_BFLAG2, 1, (U8 xdata*)&Bflag[1]))			//Read AFE REG--BFLAG2
    {
    	if((Bflag[1]&0x20) != 0)
    	{
    		bCADCFlg = 1;
    	}
    
    	if((Bflag[1]&0x40) != 0)
    	{
    		bWakeupFlg = 1;
    	}
    }

	if(MTPRead(MTP_BFLAG1, 1, (U8 xdata*)&Bflag[0]))			//Read AFE REG--BFLAG1
	{
		if((Bflag[0]&0xFF) != 0)
		{
			bProtectFlg = 1;		
		}
		else
		{
		 	bProtectFlg = 0;
		}
	}
}
