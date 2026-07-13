/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	andyliu
Version: 	V0.0
Date: 		2014/05/30
History:
	V0.0		2014/08/28		 Preliminary
********************************************************************************/
#include "main.h"
#include "memory.h"
#include "ExtE2PRomRTC.h"
#include "TwiIO.h"
//#include "TwiModule.h"
//#include "Uart.h"



/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void E2PRomWrite(U16 WrAddr, U8 Length, U8 xdata *WrBuf)
{
    U8 xdata wirtecount;

	//IENC &= ~0x20;				//禁止AlARM中断    
    wirtecount = 5;
	while((!TwiWrite(E2PROM_ID, WrAddr, Length, WrBuf))&&(wirtecount))
	{
        Delay1ms(1);
        wirtecount--;
	}
	//IENC &= ~0x20;				//允许AlARM中断 
}

/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
bit E2PRomRead(U16 RdAddr, U8 Length, U8 xdata *RdBuf)
{
    BOOL result = 0;

    U8 xdata readcount;
    
    readcount = 5;
	while((!result)&&(readcount))
	{
        result = TwiRead(E2PROM_ID, RdAddr, Length, RdBuf);
        Delay1ms(1);
        readcount--;
	}
    
    return result;
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void InitE2PRom(void)
{
	U8 xdata RdBuf[8];

//	if(!bEnEEPRomBK)
//    {
//        bE2ON = 0;
//    }
//    else if(!bE2ON)
//    {
//        bE2ON = 1;

		E2PRomRead(E2PROM_BOOT_ADDR, 8, RdBuf);
		
		//注意：RdBuf[0]与Rdbuf[1]相加后必须强制转换为U8，
		//否则系统会为了防溢出扩展成U16，导致checksum比较错误
		//RdBuf[4]与Rdbuf[5]同理	
		if(((U8)(RdBuf[0]+RdBuf[1])) == RdBuf[3])
		{
			uiE2PDataAddr = ((U16)RdBuf[0]<<8) + RdBuf[1];
		}
		else if(((U8)(RdBuf[4]+RdBuf[5])) == RdBuf[7])
		{
			uiE2PDataAddr = ((U16)RdBuf[4]<<8) + RdBuf[5];
		}
		else
		{
			uiE2PDataAddr = 0;
		}

		if(uiE2PDataAddr >= E2PROM_BOOT_ADDR)
		{
            uiE2PDataAddr = 0;			
		}
//	}
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void E2PRomBKRTC(void)
{
 	U8 xdata WrBuf[8];
	U8 i, checksum=0;
	
	RTCReadTime();
//	MemoryCopy((U8 xdata *)&RTC, WrBuf, 7);
//	MemoryCopy((U8 xdata *)&(RTC.Second), WrBuf, 3);
// 	MemoryCopy((U8 xdata *)&(RTC.Date), WrBuf + 3, 3);
	for(i=0; i<6; i++)
	{
		checksum += WrBuf[i];
	}
	WrBuf[6] = checksum;
	WrBuf[7] = 0x5A;
	
	E2PRomWrite(E2PROM_RTC_ADDR, 8, WrBuf);
}


void E2PRomBKBoot(void)
{
 	U8 xdata WrBuf[8];

	WrBuf[0] = (uiE2PDataAddr>>8);
	WrBuf[1] = uiE2PDataAddr;
	WrBuf[3] = WrBuf[0]+WrBuf[1];				//Calculate checksum

	WrBuf[4] = WrBuf[0];
	WrBuf[5] = WrBuf[1];
	WrBuf[7] = WrBuf[3];
	
	E2PRomWrite(E2PROM_BOOT_ADDR, 8, WrBuf);
} 


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void E2PRomBKData(U8 BKType)
{
	U8 xdata WrBuf[32];

    if(BKType == 0x10)                  //Charge shart
    {
      //  MemoryCopy((U8 xdata *)&ucRTCBuf[0], WrBuf, 3);
	//	MemoryCopy((U8 xdata *)&ucRTCBuf[4], WrBuf + 3, 3);
    }
    else
    {
    	RTCReadTime();
	//	MemoryCopy((U8 xdata *)&(RTCTime.Second), WrBuf, 3);
 	//	MemoryCopy((U8 xdata *)&(RTCTime.Date), WrBuf + 3, 3);
    }
	
	WrBuf[6] = (Info.PackStatus>>8);
	WrBuf[7] = Info.PackStatus;
	WrBuf[8] = (Info.BatStatus>>8);
	WrBuf[9] = Info.BatStatus;
	WrBuf[10] = (Info.E2ulFCC>>24);
	WrBuf[11] = (Info.E2ulFCC>>16);
	WrBuf[12] = (Info.E2ulFCC>>8);
	WrBuf[13] = Info.E2ulFCC;
	WrBuf[14] = (Info.RC1>>24);
	WrBuf[15] = (Info.RC1>>16);
	WrBuf[16] = (Info.RC1>>8);
	WrBuf[17] = Info.RC1;
	WrBuf[18] = (Info.Voltage>>24);
	WrBuf[19] = (Info.Voltage>>16);
	WrBuf[20] = (Info.Voltage>>8);
	WrBuf[21] = Info.Voltage;
	WrBuf[22] = (Info.CurCadc>>24);
	WrBuf[23] = (Info.CurCadc>>16);
	WrBuf[24] = (Info.CurCadc>>8);
	WrBuf[25] = Info.CurCadc;
	WrBuf[26] = (Info.Temperature1>>8);
	WrBuf[27] = Info.Temperature1;
	WrBuf[28] = (Info.E2uiCycleCount>>8);
	WrBuf[29] = Info.E2uiCycleCount;
	WrBuf[30] = BKType;
	WrBuf[31] = 0x5A;

	E2PRomWrite(uiE2PDataAddr, 32, WrBuf);
	
	uiE2PDataAddr += 32;
	if(uiE2PDataAddr >= E2PROM_BOOT_ADDR)
	{
		uiE2PDataAddr = 0;
	}

	E2PRomBKBoot();
}


void E2PRomErase(void)
{
	U16 i;
	U8 xdata WrBuf[8];
	
	//MemorySet(WrBuf, 0, 8);	

	for(i=0; i<(E2PROM_SIZE/32-1); i++)
	{
	//	RSTSTAT = RSTSTAT;
		E2PRomWrite(i*32+31, 1, WrBuf);
	}

	E2PRomWrite(E2PROM_BOOT_ADDR, 8, WrBuf);
	uiE2PDataAddr = 0;	
}


void UartRdE2PRom(void)
{	
	U16 RdAddr;
    
//    RdAddr = ((U16)ucUartBuf[UART_CMD_NO]-CMD_RD_EEPROM)*128+((U16)ucSubClassID-0x80)*4096;
    
//	E2PRomRead(RdAddr, ucUartBuf[UART_LENGTH], &ucUartBuf[UART_DATA]);
    
   // ucUartBuf[UART_DATA+ucUartBuf[UART_LENGTH]] = CRC8cal(&ucUartBuf,ucUartBuf[UART_LENGTH]+UART_DATA);
	
//	UART_SEND_DATA;			//Start Send Data; Set UART REG
}



void UartRdRTC(void)
{
	RTCReadTime();
	//MemoryCopy((U8 xdata *)&(RTC.Second), &ucUartBuf[UART_DATA], 3);
 	//MemoryCopy((U8 xdata *)&(RTC.Date), &ucUartBuf[UART_DATA + 3], 3);
	
//	ucUartBuf[UART_DATA+ucUartBuf[UART_LENGTH]] = CRC8cal(&ucUartBuf,ucUartBuf[UART_LENGTH]+UART_DATA);
	
	//UART_SEND_DATA;			//Start Send Data; Set UART REG
}


/*******************************************************************************
Function: 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
void E2PRomBKProcess(void)
{
	U8 BKType;

//    InitE2PRom();
//    
//    InitRTC();

	if(bE2PErase)
	{
		bE2PErase = 0;
		E2PRomErase();
	}
	
	if(bE2PRdData)
	{
		bE2PRdData = 0;
		UartRdE2PRom();
	}
	
	if(bRTCRdTime)
	{
		bRTCRdTime = 0;
		UartRdRTC();
	}
	
	if(bE2PBKRtc)
	{
		bE2PBKRtc = 0;
		E2PRomBKRTC();
	}
	
	if(bE2PBKChgStart)
	{
		bE2PBKChgStart = 0;
		BKType = 0x10;
		E2PRomBKData(BKType);
	}
	
	if(bE2PBKChgStop)
	{
		bE2PBKChgStop = 0;
		BKType = 0x01;
		E2PRomBKData(BKType);
	}
	
	if(bE2PBKDsgEnd)
	{
		bE2PBKDsgEnd = 0;
		BKType = 0x02;
		E2PRomBKData(BKType);
	}
}



void E2PRomBKCheck(void)
{
//    InitE2PRom();
//    
//    InitRTC();

	if(++ucRTCBKTime1 >= E2ucRTCBKDelay)
	{
		ucRTCBKTime1 = 0;
		RTCReadTime();
	}
	
	if(++uiRTCBKTime2 >= 3600)					//1h
	{
		uiRTCBKTime2 = 0;
		bE2PProcessFlg = 1;
		bE2PBKRtc = 1;
	}

	if(!bCHGING)											//筿篈竚夹粁竚Τ筿
	{
		bDsgToChgFlg = 1;
		if(bChgToDsgFlg)
		{
			bChgToDsgFlg = 0;
			bE2PProcessFlg = 1;
			bE2PBKChgStop = 1;						//筿挡盢糶EEPROM
		}
	}

	if((bDsgToChgFlg && Info.CurCadc>=E2siChgBKCur))
	{
		if(!uiCHGValidTime)
        {
        	RTCReadTime();
        	//MemoryCopy((U8 xdata *)&RTCTime, ucRTCBuf, 7);                       
        }

        if(++uiCHGValidTime > ((U16)E2ucChgBKDelay*60))
		{
			uiCHGValidTime = 0;
			bChgToDsgFlg = 1;
			bDsgToChgFlg = 0;
			bE2PProcessFlg = 1;
			bE2PBKChgStart = 1;				    	//筿秨﹍盢糶EEPROM
		}
	}
	else
	{
		uiCHGValidTime = 0;
	}
}
