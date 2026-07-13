#include "main.h"


//uint8_t g_run_protect=0;

//uint8_t g_jbsbms_tx_buf[16];

//uint8_t g_JbsBmsCmd=JBS_CMD_INFOR;

//uint8_t g_jbdCmd_Factorymode[]={0xDD,0x5A,0x00,0x02,0x56,0x78,0xFF,0x30,0x77};
//uint8_t g_jbdCmd_FETCtrlTime[]={0xDD,0x5A,0x30,0x02,0x02,0x58,0xFF,0x74,0x77};
//uint8_t g_jbdCmd_FETCtrl5STime[]={0xDD,0x5A,0x30,0x02,0x00,0x05,0xFF,0xC9,0x77};
//uint8_t g_jbdCmd_FETCtrl20MinTime[]={0xDD,0x5A,0x30,0x02,0x04,0xB0,0xFF,0x1A,0x77};
//uint8_t g_jbdCmd_FETCtrlFreeTime[]={0xDD,0x5A,0x30,0x02,0x00,0x00,0xFF,0xCE,0x77};
//uint8_t g_jbdCmd_FETCtrlLongTime[]={0xDD,0x5A,0x30,0x02,0xFF,0xEF,0xFD,0xE0,0x77};





//uint8_t g_jbdCmd_ExitFactory[]={0xDD,0x5A,0x01,0x02,0x28,0x28,0xFF,0xAD,0x77};
//uint8_t g_jbdCmd_Exit0000Factory[]={0xDD,0x5A,0x01,0x02,0x00,0x00,0xFF,0xFD,0x77};

//uint8_t g_charge_counter=0;
//uint16_t g_jbd_watchdog_timer=0;
//uint16_t g_jbd_epromWrite_timer=0;

//uint16_t g_jbd_ComErr_Cnt=0;



////DD5A30020005FFC977   5S

//extern BQ40Z50_TypeDef g_bq40z50_state;

//extern __IO  uint8_t g_Uart485Buf[UART485_RX_BUF_SIZE];

//extern uint16_t  RxUart3Counter;
//extern uint16_t  RxUart3ParsePos;


//extern UART_HandleTypeDef huart3;
//extern BQ40Z50_TypeDef g_bq40z50_state;
//extern __IO uint8_t g_BmsGetInfor_Enable;
//extern SYS_STATE_CODE_TypeDef  g_sysStateCode;

//void Jbd485Send(uint8_t *buffer,uint16_t size)
//{
//		Uart3Send(buffer,size);
//}

//void JbsBmsInit(void)
//{
//	memset((uint8_t*)g_Uart485Buf,0x00,UART485_RX_BUF_SIZE);

//	//if(huart1.RxState==HAL_UART_STATE_READY)
//	//	HAL_UART_Receive_IT(&huart1,(uint8_t*)g_Uart485Buf,UART485_RX_BUF_SIZE);

//	
//	g_JbsBmsCmd=JBS_CMD_INFOR;

//	//if(PaygGetFreeState())
//	{
//	//	HAL_UART_Transmit(&huart1,g_jbdCmd_ExitFactory,9,200);	
//		}
//	//else
//	{
//		//HAL_UART_Transmit(&huart1,g_jbdCmd_Factorymode,9,200);	

//		//HAL_Delay(200);
//		
//		if(PaygGetPayState()||PaygGetFreeState())
//		{	//HAL_UART_Transmit(&huart1,g_jbdCmd_Factorymode,9,200);
//			Jbd485Send(g_jbdCmd_Factorymode,9);
//			HAL_Delay(100);

//			if(PaygGetFreeState())
//				Jbd485Send(g_jbdCmd_FETCtrlFreeTime,9);
//				//HAL_UART_Transmit(&huart1,/*g_jbdCmd_FETCtrlTime*/g_jbdCmd_FETCtrlFreeTime,9,200);	
//			else
//				//HAL_UART_Transmit(&huart1,/*g_jbdCmd_FETCtrlTime*/g_jbdCmd_FETCtrlLongTime,9,200);
//				//Jbd485Send(g_jbdCmd_FETCtrlLongTime,9);
//				Jbd485Send(g_jbdCmd_FETCtrl20MinTime,9);
//			HAL_Delay(100);
//			//HAL_UART_Transmit(&huart1,g_jbdCmd_ExitFactory,9,200);
//			Jbd485Send(g_jbdCmd_ExitFactory,9);
//			HAL_Delay(100);

//			g_run_protect=TRUE;

//			LogPrintf("Jbd: payg Normal FET  10min  \r\n");
//			}
//		else
//		{	//HAL_UART_Transmit(&huart1,g_jbdCmd_FETCtrl5STime,9,200);
//			Jbd485Send(g_jbdCmd_Factorymode,9);
//			HAL_Delay(100);
//			Jbd485Send(g_jbdCmd_FETCtrlTime,9);
//			HAL_Delay(100);
//			//HAL_UART_Transmit(&huart1,g_jbdCmd_ExitFactory,9,200);
//			Jbd485Send(g_jbdCmd_ExitFactory,9);
//			HAL_Delay(100);

//			LogPrintf("Jbd:no payg  FET  10min  \r\n");
//			}
//		}

//}


//uint16_t JbsBmsChecksum(uint8_t *cmd,uint8_t size)
//{
//	uint8_t i=0;
//	uint16_t checksum=0;

//	for(i=0;i<size;i++)
//		checksum+=cmd[i];

//	checksum=(~checksum)+1;

//	return checksum ;
//}

//void JbsMosCtrl(uint8_t on)
//{
//	uint16_t chksum=0;

//	memset(g_jbsbms_tx_buf,0X00,16);
//	
//	g_jbsbms_tx_buf[0]=0xdd;
//	g_jbsbms_tx_buf[1]=0x5a;
//    g_jbsbms_tx_buf[2]=0xe1;
//	g_jbsbms_tx_buf[3]=2;
//	g_jbsbms_tx_buf[4]=00;

//	if(on)
//		g_jbsbms_tx_buf[5]=0x00;  //out
//	else	
//		g_jbsbms_tx_buf[5]=0x02;  //out

//	chksum=JbsBmsChecksum(g_jbsbms_tx_buf,2);


//	g_jbsbms_tx_buf[6]=chksum>>8;  //checksun high byte
//	g_jbsbms_tx_buf[7]=chksum;  //checksun low byte


//	g_jbsbms_tx_buf[8]=0x77; 

//	memset((uint8_t*)g_Uart485Buf,0x00,UART485_RX_BUF_SIZE);
//	huart3.RxXferSize=UART485_RX_BUF_SIZE;
//    huart3.RxXferCount=0;
//    huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
//	
//    //HAL_UART_Transmit(&huart1,g_jbsbms_tx_buf,9,200);	
//    Jbd485Send(g_jbsbms_tx_buf,9);
//}


//void JbsBms_GetInfo(uint8_t cmd)
//{
//    uint8_t  error_cnt=0;
//	uint16_t chksum=0;

//	memset(g_jbsbms_tx_buf,0X00,16);


//	g_jbsbms_tx_buf[0]=0xdd;
//	g_jbsbms_tx_buf[1]=0xa5;
//    g_jbsbms_tx_buf[2]=cmd;
//	g_jbsbms_tx_buf[3]=0;

//	chksum=JbsBmsChecksum(&g_jbsbms_tx_buf[2],2);
//	
//	g_jbsbms_tx_buf[4]=chksum>>8;  //checksun high byte
//	g_jbsbms_tx_buf[5]=chksum;  //checksun low byte

//	g_jbsbms_tx_buf[6]=0x77;

//	memset((uint8_t*)g_Uart485Buf,0x00,UART485_RX_BUF_SIZE);
//	
//    huart3.RxXferCount=0;
//	huart3.RxXferSize=UART485_RX_BUF_SIZE;
//    huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 

//	g_jbd_watchdog_timer++;

//	if(g_jbd_watchdog_timer<60u)
//	{	
//		if(cmd>=JBS_CMD_FACTOR)  //小于15分钟，不写BMS watchdog
//		{	
//			g_JbsBmsCmd=JBS_CMD_INFOR;
//			return ;
//			}
//		}

//	 if(cmd==JBS_CMD_LED)
//    {	memset(g_jbsbms_tx_buf,0X00,16);
//    	sprintf((char*)g_jbsbms_tx_buf,"$001,%d#",GattGetRelativeSOC());
//		//HAL_UART_Transmit(&huart1,g_jbsbms_tx_buf,strlen((char*)g_jbsbms_tx_buf),200);	
//		Jbd485Send(g_jbsbms_tx_buf,strlen((char*)g_jbsbms_tx_buf));
//		return ;
//		}

//	if(cmd>=JBS_CMD_FACTOR) //15 min
//	{
//		if(PaygGetPayState()||PaygGetFreeState()||g_run_protect)
//		{	
//			if(cmd==JBS_CMD_FACTOR)
//			{	//HAL_UART_Transmit(&huart1,g_jbdCmd_Factorymode,9,200);
//				Jbd485Send(g_jbdCmd_Factorymode,9);
//				}
//			if(cmd==JBS_CMD_FET)
//			{	
//				if(PaygGetFreeState())
//				{	//HAL_UART_Transmit(&huart1,g_jbdCmd_FETCtrlFreeTime,9,200);	
//					Jbd485Send(g_jbdCmd_FETCtrlFreeTime,9);

//					LogPrintf("Jbd:Free FET  \r\n");
//					}
//				else	
//				{	//HAL_UART_Transmit(&huart1,g_jbdCmd_FETCtrlLongTime,9,200);	
//					//Jbd485Send(g_jbdCmd_FETCtrlLongTime,9);
//					Jbd485Send(g_jbdCmd_FETCtrl20MinTime,9);
//					LogPrintf("Jbd:Watdog reload \r\n");
//					}
//				}
//			
//			if(cmd==JBS_CMD_EXIT)
//			{	
//				g_jbd_epromWrite_timer++;
//				
//				if(g_jbd_epromWrite_timer<50)
//				{	//HAL_UART_Transmit(&huart1,g_jbdCmd_Exit0000Factory,9,200);
//					Jbd485Send(g_jbdCmd_Exit0000Factory,9);

//					LogPrintf("Jbd:Write ram %d \r\n",g_jbd_epromWrite_timer);
//					}
//				else
//				{	//HAL_UART_Transmit(&huart1,g_jbdCmd_ExitFactory,9,200);
//					Jbd485Send(g_jbdCmd_ExitFactory,9);

//					LogPrintf("Jbd:Write flash %d \r\n",g_jbd_epromWrite_timer);
//					}
//				
//				g_jbd_epromWrite_timer=g_jbd_epromWrite_timer%50;
//				
//				g_jbd_watchdog_timer=0;
//				}
//			
//			}
// 
//		return ;
//		}
//	
//	{
//		// HAL_UART_Transmit(&huart1,g_jbsbms_tx_buf,7,200);	
//		Jbd485Send(g_jbsbms_tx_buf,7);
//		}
//	 
//}

//uint8_t g_testbuff[]={0xDD,0x04,0x00,0x1E,0x0F,0x66,0x0F,0x63,0x0F,0x63,0x0F,0x64,0x0F,0x3E,0x0F,0x63,0x0F,0x37,0x0F,0x5B,0x0F,0x65,0x0F,0x3B,0x0F,0x63,0x0F,0x63,0x0F,0x3C,0x0F,0x66,0x0F,0x3D,0xF9,0xF9,0x77};









