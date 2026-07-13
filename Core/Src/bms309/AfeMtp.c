/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	andyliu
Version: 	V0.0
Date: 		2014/05/30
History:
	V0.0		2014/08/28		 Preliminary
********************************************************************************/

#include "Main.h"
#include "memory.h"
#include "AfeMtp.h"
#include "TwiIO.h"
//#include "TwiModule.h"


bit MTPWrite(U8 WrAddr, U8 Length, U8 xdata *WrBuf)
{
	BOOL result;
	U8 i;
	//RSTSTAT = RSTSTAT;

    if(bAFE_ERR)
    {
        result = 0;
    }
    else
    {
    	for(i=0; i<Length; i++)
    	{
    		result = TwiWrite(AFE_ID, WrAddr, 1, WrBuf);
    		if(!result)
    		{
				Delay1ms(1);
                result = TwiWrite(AFE_ID, WrAddr, 1, WrBuf);
                if(!result)
                {
                    break;
                }
    		}
    		WrAddr++;
    		WrBuf++;
			Delay1ms(1);
     	}

    }
    
    return result;  
}


bit MTPWriteROM(U8 WrAddr, U8 Length, U8 xdata *WrBuf)
{
	BOOL result;
	U8 i;

    if(bAFE_ERR)
    {
        result = 0;
    }
    else
    {
    	for(i=0; i<Length; i++)
    	{
			//RSTSTAT = RSTSTAT;
    		result = TwiWrite(AFE_ID, WrAddr, 1, WrBuf);
    		if(!result)
    		{
				Delay1ms(40);
                result = TwiWrite(AFE_ID, WrAddr, 1, WrBuf);
                if(!result)
                {
                    break;
                }
    		}
    		WrAddr++;
    		WrBuf++;
			Delay1ms(40);
     	}

    }
    
    return result;  
}


bit MTPRead(U8 RdAddr, U8 Length, U8 xdata *RdBuf)
{
	BOOL result=1;
	//RSTSTAT = RSTSTAT;

    if(bAFE_ERR)
    {
        result = 0;
    }
    else
    {
		result = TwiRead(AFE_ID, RdAddr, Length, RdBuf);
		if(!result)
		{
			result = TwiRead(AFE_ID, RdAddr, Length, RdBuf);
		}
    }
	
	return result;
}



/*******************************************************************************
Function:ResetAFE() 
Description:  Reset SH367309 IC, Send Data:0xEA, 0xC0, CRC
Input:	 NULL
Output: NULL
Others:
*******************************************************************************/
void ResetAFE(void)
{
	U8 xdata WrBuf[2];

	WrBuf[0] = 0xC0;
	WrBuf[1] = 0xA5;
    
    if(!bAFE_ERR)
    {
	    if(!TwiWrite(AFE_ID, 0xEA, 1, WrBuf))               //0xEA, 0xC0 CRC
        {
            TwiWrite(AFE_ID, 0xEA, 1, WrBuf);
        }
    }
}


/*******************************************************************************
Function:UpdataAfeConfig() 
Description:检查SH367309的配置跟MCU是否一致，不一致就要更新SH367309的配置信息
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void UpdataAfeConfig(void)
{
    U8 xdata bufferbak[26], mtpbufferbak[26];
    U8 idata i;
    U16 idata tempres[8],refres;
	//RSTSTAT = RSTSTAT;
    
    if(MTPRead(0x00, 26, bufferbak))						//读309配置,包括TR
    {
        ucMTPBuffer[MTP_TR] = bufferbak[MTP_TR] & 0x7F;     //先获取TR[6~0]的值
        MemoryCopy(ucMTPBuffer, mtpbufferbak, sizeof(ucMTPBuffer));
        tempres[0] = NTC103AT[((S8)ucMTPBuffer[MTP_OTC])+50];
        tempres[1] = NTC103AT[((S8)ucMTPBuffer[MTP_OTCR])+50];
        tempres[2] = NTC103AT[((S8)ucMTPBuffer[MTP_UTC])+50];
        tempres[3] = NTC103AT[((S8)ucMTPBuffer[MTP_UTCR])+50];
        tempres[4] = NTC103AT[((S8)ucMTPBuffer[MTP_OTD])+50];
        tempres[5] = NTC103AT[((S8)ucMTPBuffer[MTP_OTDR])+50];
        tempres[6] = NTC103AT[((S8)ucMTPBuffer[MTP_UTD])+50];
        tempres[7] = NTC103AT[((S8)ucMTPBuffer[MTP_UTDR])+50];
        refres = 680 + 5*ucMTPBuffer[MTP_TR];
        mtpbufferbak[MTP_OTC]  = ((float)tempres[0] / ((float)refres + tempres[0])) * 512;
        mtpbufferbak[MTP_OTCR] = ((float)tempres[1] / ((float)refres + tempres[1])) * 512;
        mtpbufferbak[MTP_UTC]  = ((float)tempres[2] / ((float)refres + tempres[2]) - 0.5) * 512;
        mtpbufferbak[MTP_UTCR] = ((float)tempres[3] / ((float)refres + tempres[3]) - 0.5) * 512;
        mtpbufferbak[MTP_OTD]  = ((float)tempres[4] / ((float)refres + tempres[4])) * 512;
        mtpbufferbak[MTP_OTDR] = ((float)tempres[5] / ((float)refres + tempres[5])) * 512;
        mtpbufferbak[MTP_UTD]  = ((float)tempres[6] / ((float)refres + tempres[6]) - 0.5) * 512;
        mtpbufferbak[MTP_UTDR] = ((float)tempres[7] / ((float)refres + tempres[7]) - 0.5) * 512;
        
        for(i=0; i<25; i++)                                 //最后一个TR不做对比
        {
//            RSTSTAT = RSTSTAT;
            if(bufferbak[i] != mtpbufferbak[i])
            {
			    ucCellNum = 16;
                BatteryInfoManage();
                if(Info.Voltage < 18000)		//如果AFE供电低于18V，烧写时VPRO电压可能不足8V，不能烧写
                {
                  //  bAFE_ERR = 1;
                    break;
                }

                //VPRO_CON_ON();
                if(!MTPWriteROM(0x00, 25, mtpbufferbak))     //try 2 次
                {
                  //  bAFE_ERR = 1;
                }
                //VPRO_CON_OFF();
                
                if(!bAFE_ERR)
                {
                  //  EA = 0;
                    ResetAFE();                             //Reset IC
					Delay1ms(5);
                    InitAFE();
                }
                
                break;
            }
        }
        
        if(!bAFE_ERR)
        {
			if(MTPRead(0x00, 26, bufferbak))						//Read MTP value
		 	{
		 	    for(i=0; i<25; i++)                                 //最后一个TR不做对比
			    {
			        if(bufferbak[i] != mtpbufferbak[i])
		 	        {
			          //  bAFE_ERR = 1;
						break;
			        }
			    }
		 	}
			else
			{
			   // bAFE_ERR = 1;				
			}
        }

		if(!bAFE_ERR)
		{
			ucMTPBuffer[0] = bufferbak[0];
            ucCellNum = ucMTPBuffer[0] & 0x0F;              //Get System Cell number							        
            if(ucCellNum < 5)
            {
                ucCellNum = 16;
            }
			ucMTPBuffer[MTP_TR] = bufferbak[MTP_TR] & 0x7F;     //获取TR[6~0]的值			
		}
    }
    else
    {
        bAFE_ERR = 1;
    }
	Info.PackStatus = uiPackStatus;
}


/*******************************************************************************
Function:InitAFE() 
Description:  check SH367309 is ready, and initialization MTP Buffer
Input:	NULL 	
Output: NULL
Others:
*******************************************************************************/
void InitAFE(void)
{
    U8 TempCnt=0;
    U8 xdata MTPConfVal_BAK = 0;
    U8 xdata TempVar;

	bAFE_ERR = 0;

	while(1)
	{
		//RSTSTAT = RSTSTAT;
		
        TempVar = 0;
		if(MTPRead(0x71, 1, &TempVar))          //读取BLFG2，查看VADC是否转换完成
 		{
            if((TempVar&0x10) == 0x10)
            {
                break;
            }
 		}

        Delay1ms(20);
        if(++TempCnt >= 50)
        {
            bAFE_ERR = 1;
			Info.PackStatus = uiPackStatus;					//Updata bAFE_ERR flag and display on the upper computer
            break;
        }
	}
    
//    MTPRead(0x00, 1, &ucCellNum);
//	ucCellNum &= 0x0F;								        //Get System Cell number
//	if(ucCellNum < 5)
//	{
//		ucCellNum = 16;
//	}

//	if(!MTPRead(0x00, 26, ucMTPBuffer))						//Read MTP value  for the PC display
//    {
//        MemorySet(ucMTPBuffer, 0, sizeof(ucMTPBuffer));    
//    }
//	ucMTPBuffer[25] &= 0x7F;								//TR[6~0]


}


/*******************************************************************************
Function:EnableAFEWdtCadc()  
Description:使能CHG&DSG&PCHG输出，且使能WDT和CADC模块
Input: 	
Output: 
Others:
*******************************************************************************/
void EnableAFEWdtCadc(void)
{
	ucMTPConfVal |= 0x7c;
	MTPWrite(MTP_CONF, 1, &ucMTPConfVal);
}