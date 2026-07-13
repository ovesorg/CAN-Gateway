/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	Jianyi.Zhang
Version: 	V0.1
Date: 		2018/05/06
History:
	V0.0		2014/05/30		 Preliminary
	V0.1		2018/05/06		 修改平衡的判定和配置流程&平衡进退条件
********************************************************************************/
#include "main.h"
#include "Memory.h"
#include "AfeMtp.h"

/*******************************************************************************
Function: BalProcess()
Description: 判定平衡进入/退出条件，配置AFE平衡寄存器 
Input:	 	
Output: 
Others:
*******************************************************************************/
void BalProcess(void)
{
	U8 i;
	U16 xdata balancebk = 0;

    if (((uiCellVmax-uiCellVmin) >= E2uiBalanceVolDiff)	    //电压电流满足平衡进入条件
		&& (Info.CurCadc >= E2siBalanceCur))
	{
		for (i=0; i<ucCellNum; i++)
		{
            if ((Info.VCell[i] >= E2uiBalanceVol)
				&& ((Info.VCell[i]-uiCellVmin) >= E2uiBalanceVolDiff))   //电芯电压大于平衡电压，且压差大于阈值
            {
        		if(++ucBalanceTimeCnt[i] >= E2ucBalanceDelay)			//该电芯满足平衡条件且超过延时阈值
        		{
        	    	ucBalanceTimeCnt[i] = E2ucBalanceDelay;
        	    	balancebk |= (1<<i);
        		}
            }
			else						 			//电芯不满足平衡条件时立即停止该电芯的平衡
			{
       			ucBalanceTimeCnt[i] = 0;
       			balancebk &= ~(1<<i);
			}
		}	
	}

    if (bDSGING										//满足平衡退出条件时立即退出平衡
		|| bUTC
	 	|| bOTC 
		|| bUTD 
		|| bOTD 
		|| bPF 
		|| ((uiCellVmax-uiCellVmin) < E2uiBalanceVolDiff)
		|| (Info.CurCadc < E2siBalanceCur))
	{
		for (i=0; i<ucCellNum; i++)
		{
			ucBalanceTimeCnt[i] = 0;
		}
		balancebk = 0;
	}
	
	if ((uiBalanceChannel != balancebk)				//平衡状态改变时配置平衡，或平衡过程中定时写平衡寄存器
		|| ((uiBalanceChannel != 0) && (++ucBalUpdateTimeCnt >= E2ucBalanceDelay)))
	{
		ucBalUpdateTimeCnt = 0;
		uiBalanceChannel = balancebk;
		MTPWrite(0x41, 0x02, (U8 xdata *)&balancebk);
	}
}














