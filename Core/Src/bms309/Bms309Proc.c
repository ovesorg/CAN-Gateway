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

void bms309Init(void)
{
	Initial();
}


void bms309TimerSet(uint8_t on)
{
	bTimerFlg=on;
}
uint8_t testafe=0;
void bms309Proc(void)
{
	uint8_t i;
	
	if(bAFEFlg)
	{
	 	bAFEFlg = 0;
		AFERdFlag();
	}

	if((ucResetFlag==0x12) && bUartSndAckFlg)	//Software reset occurs, and ended UART communication
	{
		if(!bWrFlashFlg)
        {
			//EA = 0;
			ResetAFE();						//Reset IC
			ResetInit();					//Reset MCU
			}
		}
	
	if(bWakeupFlg)						//The charger / load, keys can wake up the system
	{
		bWakeupFlg = 0;
		WakeUpProcess();				//System to be awakened
	}
	
	if(bBleOnOffFlg)					//按键长按5秒开启/关闭蓝牙
	{
		bBleOnOffFlg = 0;
		BleOnorOff();
	}
	
	BleDisplay();

	if(bCADCFlg)
	{
		bCADCFlg = 0;
		CurProcess();					//Calculate the current value and determines the state of charge and discharge
	}

	if(bCalibrationFlg)
	{
		bCalibrationFlg = 0;
		CaliProcess();					//PC calibration
	}

	if((bWrFlashFlg)&&(ucUpDataLimitTime == 0))
	{
		bWrFlashFlg = 0;
		if(FlashProcess())
		{
			if(bUartNeedAckFlg)
			{
			;//	UART_SEND_ACK;
			}
		}
		else
		{
			if(bUartNeedAckFlg)
			{
				;//UART_SEND_NACK;
			}
		}
		bUartNeedAckFlg = 0;
	}
		
		if(bLEDOpen)
		{
			bLEDOpen = 0;
			bLEDFlg = 1;
			ucLEDTimeCnt = 0;
			LEDPowerOn();					//By LED displays battery capacity
		}

		if(bEnEEPRomBK && bE2PProcessFlg)
		{
			bE2PProcessFlg = 0;
			E2PRomBKProcess();
		}

		/*if(bISPFlg && bUartSndAckFlg)
		{
			bISPFlg = 0;
			ISPProcess();
		}*/
	if(bTimerFlg)						//timer is 1s
	{
		bTimerFlg = 0;
	
		AFECheck(); 					//Check AFE Register		
	
		//LEDPowerOff();					//When the LED display 4s, turn off the LED
	
		BatteryInfoManage();			//Get battery voltage and temperature information
	
		//RamCheckProcess();				//Detect the xdata block data
	
		GaugeManage();					//Calculate the battery charge and discharge capacity
	
		VolProcess();					//Detection of battery charge and discharge cut-off voltage
		
		BalProcess();					//Process external voltage balance
	
		ProtectProcess();				//Process battery abnormal state

		//for(i=0;i<8;i++)
		//	GattSetData(LIST_DIA, DIA_CV01+i, g_UserSet.NetInfor.apn);

		GattSetRelativeSOC( Info.RSOC);
		GattSetBattTemp( Info.CurCadc);
		GattSetBattCurrent(Info.Temperature1);
		GattSetBattVolt(Info.Voltage);
		
		//if(bEnEEPRomBK)
		{
		//	E2PRomBKCheck();			//Detecting whether to backup EEPRom
		}
	
		//ShutDownProcess();				//Detecting whether to enter a low-power
	}

		IntoShutDown();
}


