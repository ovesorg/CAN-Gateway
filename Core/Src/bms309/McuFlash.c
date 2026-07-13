/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	Jianyi.Zhang
Version: 	V0.1
Date: 		2019/06/11
History:
	        V0.0		2016/05/18		 Preliminary
			V0.1		2019/06/11
********************************************************************************/
#include "main.h"
#include "memory.h"
#include "mcuflash.h"


/*******************************************************************************
Function:Flash_wirte_one_byte() 
Description: 向类EEPROM特定单元写入1字节 
Input:	 	
Output: 
Others:
*******************************************************************************/
void Flash_wirte_one_byte(U16 xdata ADRS,U8 xdata WDAT)
{
  /*  RSTSTAT = RSTSTAT;
    EA = 0;
    FLASHCON = 1;

    XPAGE = ADRS / 256;
    IB_OFFSET = ADRS % 256;

    IB_DATA = WDAT;

    IB_CON1 = 0x6E;	
    IB_CON2 = 0x05;
    IB_CON3 = 0x0A;
    IB_CON4 = 0x09;
    IB_CON5 = 0x06;
    _nop_();		
    _nop_();
    _nop_(); 
    _nop_();
	_nop_();

    FLASHCON = 0;

    EA = 1;*/
}

/*******************************************************************************
Function:BlackE2CheckUp(U16 Saddr, U16 num)
Description:
Input:
Output:
Others:
*******************************************************************************/
BOOL BlackE2CheckUp(U16 Saddr)
{
   /* U16 i;
    RSTSTAT = RSTSTAT;
	    
    FLASHCON = 0x01;
    
    for(i=0; i<512; i++)
    {
        if(CBYTE[Saddr+i] != 0x00)
        {
            return 0;
        }
    }*/
    
    return 1;
}


/*******************************************************************************
Function:E2DataCheckUp() 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
BOOL E2DataCheckUp(U16 Saddr)
{
   /* U8 xdata *data ptr = 0;
    U16 i;
    RSTSTAT = RSTSTAT; 
	   
    FLASHCON = 0x01;
    
    for(i=0; i<512; i++)
    {
        if(CBYTE[Saddr+i] != *(ptr+i))
        {
            return 0;
        }
    }*/
    return 1;
}


/*******************************************************************************
Function:WriteE2Sector(U16 Saddr, U16 num) 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
BOOL WriteE2Sector(U16 Saddr)
{
   U16 i;
    U8 xdata *data ptr = 0;
    BOOL result = 0;
    
    /* RSTSTAT = RSTSTAT;

    for(i=0; i<512; i++)
    {
        IB_OFFSET = Saddr+i;
        XPAGE = (Saddr+i)>>8;
        IB_DATA = *(ptr+i);
        
        if(ucUpDataLimitTime == 0)
        {
            IB_CON1 = 0x6E;
            if(!EA)
            {
                IB_CON2 = 0x05;
                if(FLASHCON)
                {
                    IB_CON3 = 0x0A;
                    if(ucFlashWrValid == 0x55)
                    {
                        IB_CON4 = 0x09;
                        if(XPAGE == ((Saddr + i) >> 8))
                        {
                            IB_CON5 = 0x06;
                            _nop_();
                            _nop_();
                            _nop_();
                            _nop_();
                            _nop_();
                            
                            result = 1;
                        }
                        else break;
                    }
                    else break;
                }
                else break;
            }
            else break;
        }
        else break;
    }*/
    return result;
}
/*******************************************************************************
Function:WriteMcuFlash(U16 Saddr) 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
BOOL WriteMcuFlash(U16 Saddr)
{
    BOOL result = 0;
   /* RSTSTAT = RSTSTAT;
	EA = 0;
	FLASHCON = 0x01;
	XPAGE = Saddr >> 8;
    
    if(ucUpDataLimitTime == 0)
    {
        IB_CON1 = 0xE6;
        if(!EA)
        {
            IB_CON2 = 0x05;
            if(FLASHCON)
            {
                IB_CON3 = 0x0A;
                if(ucFlashWrValid == 0x55)
                {
                    IB_CON4 = 0x09;
                    if(XPAGE == (Saddr >> 8))
                    {
                        IB_CON5 = 0x06;
                        _nop_();
                        _nop_();
                        _nop_();
                        _nop_();
                        _nop_();

                        if(BlackE2CheckUp(Saddr))                   
                        {
                            E2uiCheckFlag = 0x5AA5;
                            
                            if(WriteE2Sector(Saddr))                    
                            {
                                if(E2DataCheckUp(Saddr))              
                                {
                                    result = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    IB_CON1 = 0;
    IB_CON2 = 0;
    IB_CON3 = 0;
    IB_CON4 = 0;
    IB_CON5 = 0;
    
    FLASHCON = 0x00;
    XPAGE = 0x00;
    EA = 1;*/

    
    return result;
}


/*******************************************************************************
Function:FlashProcess() 
Description:  
Input:	 	
Output: 
Others:
*******************************************************************************/
bit FlashProcess(void)
{
	BOOL result = 1;
	BOOL E2S0WRFailFlg=0, E2S1WRFailFlg=0;
    
    if((Info.Voltage >= 10000) && (E2uiCheckFlag == 0x5AA5))   //10V && XRAM data标志正常
    {
        if(!WriteMcuFlash(DATAFLASH_ADDR))
        {
            if(!WriteMcuFlash(DATAFLASH_ADDR))				   //尝试写备份A区失败
            {
				result = 0;
                E2S0WRFailFlg = 1;
				E2uiCheckFlag = 0xFFFF;
				Flash_wirte_one_byte(DATAFLASH_ADDR+510,0xFF);
				Flash_wirte_one_byte(DATAFLASH_ADDR+511,0xFF);
            }
			else if(!WriteMcuFlash(DATAFLASH_ADDR2))
			{
				if(!WriteMcuFlash(DATAFLASH_ADDR2))
				{
					result = 0;
					E2S1WRFailFlg = 1;
					E2uiCheckFlag = 0xFFFF;
					Flash_wirte_one_byte(DATAFLASH_ADDR2+510,0xFF);
					Flash_wirte_one_byte(DATAFLASH_ADDR2+511,0xFF);
				}
			}	
        }
		else
		{
			if(!WriteMcuFlash(DATAFLASH_ADDR2))
        	{
        	    if(!WriteMcuFlash(DATAFLASH_ADDR2))
        	    {
					result = 0;
					E2S1WRFailFlg = 1;
					E2uiCheckFlag = 0xFFFF;
					Flash_wirte_one_byte(DATAFLASH_ADDR2+510,0xFF);
					Flash_wirte_one_byte(DATAFLASH_ADDR2+511,0xFF);
        	    }	
        	}
		}
    }
    else
    {
		result = 0;
        E2S0WRFailFlg = 1;
        E2S1WRFailFlg = 1;
    }

	ucFlashWrValid = 0x00;
	
	return result;
}


/*******************************************************************************
Function: ReadMcuFlash(U16 SourceAddr, U16 DestAddr, U8 Length)
Description:  
Input: 	
Output: 
Others:
*******************************************************************************/
BOOL ReadMcuFlash(U16 SourceAddr, U8 xdata *DestAddr, U16 Length)
{
	U16 i;
    BOOL result = 0;
	/*RSTSTAT = RSTSTAT;
	EA = 0;
	FLASHCON = 0x01;
    
    if(CWORD[(SourceAddr+510)/2] == 0x5AA5)
    {
        for(i=0; i<Length; i++)
        {

            *DestAddr = CBYTE[SourceAddr+i];
            DestAddr++;
        }
        result = 1;
    }

	FLASHCON = 0x00;*/
    return result;
}



