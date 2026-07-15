 
#include  "main.h"
#include  "eeprom.h"
union BitGroup_TypeDef g_CanMcuEvent;
union BitGroup_TypeDef g_CanBmsEvent;
union BitGroup_TypeDef g_CanBms1Event;

MCU_FAULT_TypeDef g_McuFaultInfor;
MCU_RUNINFOR_TypeDef  g_McuRunInfor;
MCU_POWEROUT_TypeDef g_McuPowerOut;
MCU_SYSINFOR1_TypeDef  g_McuSysInfor1;
MCU_SYSINFOR2_TypeDef  g_McuSysInfor2;
MCU_FUNSWCH_TypeDef g_McuFunSwitch;
MCUCCS_TypeDef     g_McuCCSOut;

HM7280_CCS_DEVID_TypeDef  g_Devid;

BMS_RT_STATUS1_TypeDef g_BmsRtStatus1;
BMS_RT_STATUS2_TypeDef g_BmsRtStatus2;
BMS_RT_STATUS3_TypeDef g_BmsRtStatus3;

#define HM7280_IOT_ID_ALARM_WDG            0x1806E55DUL
#define HM7280_IOT_ID_SERIAL_HIGH          0x1806E55EUL
#define HM7280_IOT_ID_SERIAL_LOW           0x1806E55FUL
#define HM7280_CCS_ID_IOT_DEVH			   0x18FF50E7UL
#define HM7280_CCS_ID_IOT_DEVL             0x18FF50E8UL
BMS_CELLVOLT1_TypeDef g_BmsCellVolt1;
BMS_CELLVOLT2_TypeDef g_BmsCellVolt2;
BMS_CELLVOLT3_TypeDef g_BmsCellVolt3;
BMS_CELLVOLT4_TypeDef g_BmsCellVolt4;
BMS_CELLVOLT5_TypeDef g_BmsCellVolt5;
BMS_CELLVOLT6_TypeDef g_BmsCellVolt6;

BMS_RTTEMP_TypeDef g_BmsRtTemp;
BMSCCS_TypeDef g_Bms_Charge;
BMS_SYSINFOR_TypeDef g_BmsSysInfor;

VCU_WDG_TypeDef g_Vcu_0x00632;
VCU_WDG_REFRESH_TypeDef g_Vcu_0x00633;

CAN_TXSTATE_TypeDef g_CanTransmitState;

#ifdef CAN_TRASMITER_SUPPORT
can_receive_message_struct g_can0RxMessage;
can_receive_message_struct g_can1RxMessage;
uint8_t g_Hm7280IotSerialHigh[8];
uint8_t g_Hm7280IotSerialLow[8];
can_trasnmit_message_struct g_can0TxMessage[CAN_TX_BUF_SIZE];
can_trasnmit_message_struct g_can1TxMessage[CAN_TX_BUF_SIZE];

can_trasnmit_message_struct g_can0TxMessage_bms;
uint8_t g_CanAddr=0;
uint32_t g_Logdelay[2]={0};
#endif

CAN_RECOVERY_TypeDef g_CanRecoverState[2];
uint8_t g_Hm7280Serial[14];
uint8_t g_Hm7280SerialLength;
 
uint8_t g_Hm7280SerialDirty = TRUE;

void CanRamInit(void)
{
	g_CanMcuEvent.BYTE=0;
	g_CanBmsEvent.BYTE=0;
	g_CanBms1Event.BYTE=0;

	memset((uint8_t*)&g_McuFaultInfor,0x00,8);
	memset((uint8_t*)&g_McuRunInfor,0x00,8);
	memset((uint8_t*)&g_McuPowerOut,0x00,8);
	memset((uint8_t*)&g_McuSysInfor1,0x00,8);
	memset((uint8_t*)&g_McuSysInfor2,0x00,8);
	memset((uint8_t*)&g_McuFunSwitch,0x00,8);

	memset((uint8_t*)&g_BmsRtStatus1,0x00,8);
	memset((uint8_t*)&g_BmsRtStatus2,0x00,8);
	memset((uint8_t*)&g_BmsRtStatus3,0x00,8);

	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8);

	memset((uint8_t*)&g_BmsRtTemp,0x00,8);
	
	memset((uint8_t*)&g_BmsSysInfor,0x00,8);

	memset((uint8_t*)&g_CanTransmitState,0x00,sizeof(g_CanTransmitState));
	memset((uint8_t*)&g_CanRecoverState[0],0x00,sizeof(CAN_RECOVERY_TypeDef));
	memset((uint8_t*)&g_CanRecoverState[1],0x00,sizeof(CAN_RECOVERY_TypeDef));
	

	//g_UserSet.canid_cnt = 7;
	g_CanAddr = g_UserSet.canid_cnt+1;
	
	LogPrintf(" g_CanAddr   >>>>>>>>>>  %d\r\n",g_CanAddr);
}

#define CAN_ERR_REG(a) CAN_ERR(a)
#define CAN_TSTAT_REG(a) CAN_TSTAT(a)
#define CAN_CTRL_REG(a) CAN_TSTAT(a)

//BUS OFF BEHAVIOR 

#define MCR_INRQ	 ((uint32_t)0x00000001) /* Initialization request */

void CanRecoveryProc(uint8_t canindex,uint32_t candev)
{
	
	uint32_t time_out=(g_CanRecoverState[canindex].busoff_state==BUSOFF_SLOW)?T_BUSOFF_SLOW:T_BUSOFF_QUICK;

    if(canindex>=2)
		return;

	if(g_CanRecoverState[canindex].busoff_state!=BUSOFF_NONE)
	{
		if(HAL_GetTick()-g_CanRecoverState[canindex].busoff_start_timer>=time_out)
		{	
			if(!(CAN_ERR(candev)&CAN_ERR_BOERR))
			{	
				LogPrintf("-can %d BUSOFF_CLOSE %d %d %d\r\n",canindex,HAL_GetTick(),g_CanRecoverState[canindex].busoff_start_timer,time_out);
				g_CanRecoverState[canindex].tx_disable=FALSE;
				g_CanRecoverState[canindex].busoff_state=BUSOFF_NONE;
				//CAN_Amp3_TransmitPowerState();
				
			}
		}
	}
	
	if((CAN_ERR(candev)&CAN_ERR_BOERR)&&(HAL_GetTick()-g_CanRecoverState[canindex].busoff_start_timer>=time_out))
	{

		CAN_TSTAT(candev) |=0x00808080;

		CAN_CTL(candev) |= MCR_INRQ;
		CAN_CTL(candev) &=~ MCR_INRQ;

		if(g_CanRecoverState[canindex].busoff_counter<1000)
			g_CanRecoverState[canindex].busoff_counter++;

		g_CanRecoverState[canindex].tx_disable=TRUE;
		

		if(g_CanRecoverState[canindex].busoff_counter>=5)//slow  recovery
		{	
			g_CanRecoverState[canindex].busoff_start_timer=HAL_GetTick();//T_BUSOFF_SLOW;
			g_CanRecoverState[canindex].busoff_state=BUSOFF_SLOW;
			LogPrintf("-can %d-BUSOFF_SLOW %d %d\r\n",canindex,HAL_GetTick(),g_CanRecoverState[canindex].busoff_counter);
		}
		else
		{	
			g_CanRecoverState[canindex].busoff_start_timer=HAL_GetTick();//T_BUSOFF_QUICK;
			g_CanRecoverState[canindex].busoff_state=BUSOFF_QUICK;
			LogPrintf("-can %d-BUSOFF_QUICK %d %d\r\n",canindex,HAL_GetTick(),g_CanRecoverState[canindex].busoff_counter);
		}
	}


	if((((CAN_ERR(candev)&0x00000070)==0x00000000)||((CAN_ERR(CAN0)&0x00000070)==0x00000030))
		&&((CAN_ERR(candev)&CAN_ERR_BOERR)==RESET)&&g_CanRecoverState[canindex].busoff_counter)
		{
			g_CanRecoverState[canindex].busoff_counter=0;
			 LogPrintf("-can %d-BUSOFF_CLEAR_NONE %d\r\n",canindex,HAL_GetTick());
		}

	//MISS ACK BEHAVIOR
	if((CAN_ERR(candev)&0x00000070)==0x00000030&&g_CanRecoverState[canindex].busoff_state==BUSOFF_NONE)
	{

		switch(g_CanRecoverState[canindex].miss_ack_state)
		{
			case MISS_ACK_NONE:
				g_CanRecoverState[canindex].miss_ack_start_timer=HAL_GetTick();//TX_TIMEOUT;
				g_CanRecoverState[canindex].miss_ack_state=MISS_ACK_TIMEOUT;
			    LogPrintf("-can %d -MISS_ACK_NONE %d\r\n",canindex,HAL_GetTick());
				break;
			case MISS_ACK_TIMEOUT:
				if(HAL_GetTick()-g_CanRecoverState[canindex].miss_ack_start_timer>=TX_TIMEOUT)
				{	//stop send
					g_CanRecoverState[canindex].miss_ack_start_timer=HAL_GetTick();//TX_RECOVERY;
					g_CanRecoverState[canindex].miss_ack_state=MISS_ACK_RECOVERY;
					//stop transmit
					CAN_TSTAT(candev) |=0x00808080;
					g_CanRecoverState[canindex].tx_disable=TRUE;

					LogPrintf("-can %d -MISS_ACK_TIMEOUT %d\r\n",canindex,HAL_GetTick());
				}
				break;
			case MISS_ACK_RECOVERY:
				if(HAL_GetTick()-g_CanRecoverState[canindex].miss_ack_start_timer>=TX_RECOVERY)
				{	//start send
					g_CanRecoverState[canindex].miss_ack_start_timer=HAL_GetTick();//TX_TIMEOUT;
					g_CanRecoverState[canindex].miss_ack_state=MISS_ACK_TIMEOUT;
					LogPrintf("-can %d -MISS_ACK_RECOVERY %d\r\n",canindex,HAL_GetTick());
					//CAN_Amp3_TransmitPowerState();
					g_CanRecoverState[canindex].tx_disable=FALSE;
				}
				break;
		}
	}
	else
	{
		g_CanRecoverState[canindex].miss_ack_state=MISS_ACK_NONE;
		g_CanRecoverState[canindex].miss_ack_start_timer=HAL_GetTick();

		if(g_CanRecoverState[canindex].busoff_state==BUSOFF_NONE)
			g_CanRecoverState[canindex].tx_disable=FALSE;

		if(HAL_GetTick()-g_Logdelay[canindex]>10)
		{	
//				LogPrintf("-can %d-none %d\r\n",canindex,HAL_GetTick());  yh
			g_Logdelay[canindex]=HAL_GetTick();
		}
	}

}

static void CanHm7280BuildSerialPayload(void)
{
	uint8_t ppid[MEM_SIZE_PPID + 1];
	uint8_t i;

	memset(ppid, 0x00, sizeof(ppid));
	memset(g_Hm7280Serial, 0xFF, sizeof(g_Hm7280Serial));
	memset(g_Hm7280IotSerialHigh, 0xFF, sizeof(g_Hm7280IotSerialHigh));
	memset(g_Hm7280IotSerialLow, 0x0, sizeof(g_Hm7280IotSerialLow));

	GattGetPpid(ppid);
	g_Hm7280SerialLength = 0;

	for (i = 0; i < sizeof(g_Hm7280Serial); i++)
	{
		if (ppid[i] == 0)
			break;

		g_Hm7280Serial[i] = ppid[i];
		g_Hm7280SerialLength++;
	}

	memcpy(g_Hm7280IotSerialHigh, g_Hm7280Serial, 6);
	memcpy(g_Hm7280IotSerialLow, &g_Hm7280Serial[6], 8);
	
	printf("read.......%s-----%s%s..\r\n", g_Hm7280Serial,g_Hm7280IotSerialHigh,g_Hm7280IotSerialLow);
}

#ifdef CAN_TRASMITER_SUPPORT
void CanFifoProc(can_trasnmit_message_struct* tx_message,uint8_t len)
{
	uint8_t i=0;

	for(i=0;i<len-1;i++)
	{
		memcpy((uint8_t*)&tx_message[i],(uint8_t*)&tx_message[i+1],sizeof(can_trasnmit_message_struct));
	}
}
void Can0RxProc(can_receive_message_struct* rx_message) //IOT
{
	CAN_ID_TypeDef  id_infor;
	uint32_t valid_id=rx_message->rx_efid&0xFFFFF000;

	
	if(rx_message->rx_efid == 0X1806E640)
	{
		CanMcuParse(rx_message->rx_efid,rx_message->rx_data,rx_message->rx_dlen);	
	}
	else
	{
		#ifdef CAN0_CAN1_INVERT
		if((0x001806E000==valid_id)&&(CAN_FF_EXTENDED == rx_message->rx_ff))
		#else
		if(g_CanTransmitState.can0_count<CAN_TX_BUF_SIZE)
		#endif
		{	
			
			#ifndef CAN0_CAN1_INVERT
			memcpy((uint8_t*)&id_infor,(uint8_t*)&rx_message->rx_efid,sizeof(CAN_ID_TypeDef));
			id_infor.msg_addr=g_CanAddr;
			id_infor.ms|=0x01;

			memcpy((uint8_t*)&g_can0TxMessage[g_CanTransmitState.can0_count].tx_efid,(uint8_t*)&id_infor,sizeof(CAN_ID_TypeDef));
			#endif
			g_can0TxMessage[g_CanTransmitState.can0_count].tx_sfid=rx_message->rx_sfid;
			#ifdef CAN0_CAN1_INVERT
			g_can0TxMessage[g_CanTransmitState.can0_count].tx_efid|=rx_message->rx_efid;
			#endif
			g_can0TxMessage[g_CanTransmitState.can0_count].tx_ff=rx_message->rx_ff;
			g_can0TxMessage[g_CanTransmitState.can0_count].tx_ft=rx_message->rx_ft;
			g_can0TxMessage[g_CanTransmitState.can0_count].tx_dlen=rx_message->rx_dlen;
			memcpy(g_can0TxMessage[g_CanTransmitState.can0_count].tx_data,rx_message->rx_data,rx_message->rx_dlen);

			if(g_CanTransmitState.can0_count<CAN_TX_BUF_SIZE)
				g_CanTransmitState.can0_count++;
			
			HAL_GPIO_WritePin(CAN1LED_GPIO_Port,CAN1LED_Pin,RESET);
		}
	}	
}

void Can1RxProc(can_receive_message_struct* rx_message) //BMS
{
	uint32_t valid_id=rx_message->rx_efid&0xFFFFF000;
	CAN_ID_TypeDef  id_infor;
	#ifndef CAN0_CAN1_INVERT
	if((0x001806E000==valid_id)&&(CAN_FF_EXTENDED == rx_message->rx_ff))
	#endif
	{	
//		if(rx_message->rx_efid== 0x1806E5F4)
//		{
//			CanBmsParse(rx_message->rx_efid,rx_message->rx_data,rx_message->rx_dlen);
//			
//		}
//		else
		{	
			if(g_CanTransmitState.can1_count<CAN_TX_BUF_SIZE)
			{	
				#ifdef CAN0_CAN1_INVERT
				memcpy((uint8_t*)&id_infor,(uint8_t*)&rx_message->rx_efid,sizeof(CAN_ID_TypeDef));
			
				id_infor.msg_addr=g_CanAddr;
				id_infor.ms|=0x01;

				memcpy((uint8_t*)&g_can1TxMessage[g_CanTransmitState.can1_count].tx_efid,(uint8_t*)&id_infor,sizeof(CAN_ID_TypeDef));
				#endif
				
				g_can1TxMessage[g_CanTransmitState.can1_count].tx_sfid=rx_message->rx_sfid;
				#ifndef CAN0_CAN1_INVERT
				g_can1TxMessage[g_CanTransmitState.can1_count].tx_efid=rx_message->rx_efid;
				#endif
				g_can1TxMessage[g_CanTransmitState.can1_count].tx_ff=rx_message->rx_ff;
				g_can1TxMessage[g_CanTransmitState.can1_count].tx_ft=rx_message->rx_ft;
				g_can1TxMessage[g_CanTransmitState.can1_count].tx_dlen=rx_message->rx_dlen;
				memcpy(g_can1TxMessage[g_CanTransmitState.can1_count].tx_data,rx_message->rx_data,rx_message->rx_dlen);

				if(g_CanTransmitState.can1_count<CAN_TX_BUF_SIZE)
					g_CanTransmitState.can1_count++;
				
				
				if((rx_message->rx_efid== 0x1806E5F4)||(rx_message->rx_efid== 0x1806E612 )||(rx_message->rx_efid == 0x18FF50E6))
				{
					CanBmsParse(rx_message->rx_efid,rx_message->rx_data,rx_message->rx_dlen);
				}
				HAL_GPIO_WritePin(CAN2LED_GPIO_Port,CAN2LED_Pin,RESET);
				
				
			}
		}
	}	
}

#endif
void CanMcuParse(uint32_t id,uint8_t *data,uint8_t len) //vcu
{

	switch(id)
	{
		case 0x01806E502:
			memcpy((uint8_t*)&g_McuSysInfor1,data,len);
			McuSysInfor1Event=TRUE;
			break;
		case 0x01806E503:
			memcpy((uint8_t*)&g_McuSysInfor2,data,len);
			McuSysInfor2Event=TRUE;
			break;
		case 0x01806E600:
			memcpy((uint8_t*)&g_McuFaultInfor,data,len);
			McuFaultEvent=TRUE;
			break;
		case 0x01806E601:
			memcpy((uint8_t*)&g_McuRunInfor,data,len);
			McuRunInforEvent=TRUE;
			break;
		case 0x01806E602:
			memcpy((uint8_t*)&g_McuPowerOut,data,len);
			McuPwrOutEvent=TRUE;
			break;
		case 0X1806E640:
			memcpy((uint8_t*)&g_McuCCSOut,data,len);
			McuCCSEvent=TRUE;
			break;
		
	}
}

void CanBmsParse(uint32_t id,uint8_t *data,uint8_t len) //bms
{
	switch(id)
	{
		case 0x01806E516:
			memcpy((uint8_t*)&g_BmsSysInfor,data,len);
			BmsSysInforEvent=TRUE;
			break;
		case 0x18FF50E6:   //实时温度与输入电压
			memcpy((uint8_t*)&g_BmsRtStatus1,data,len);
			BmsRtState1Event=TRUE;
			break;
		case 0x01806E611:
			memcpy((uint8_t*)&g_BmsRtStatus2,data,len);
			BmsRtState2Event=TRUE;
			break;
		case 0x01806E612:
			memcpy((uint8_t*)&g_BmsRtStatus3,data,len);
			BmsRtState3Event=TRUE;
			break;
		case 0x01806E613:
			memcpy((uint8_t*)&g_BmsCellVolt1,data,len);
			BmsCellVolt1Event=TRUE;
			break;
		case 0x01806E614:
			memcpy((uint8_t*)&g_BmsCellVolt2,data,len);
			BmsCellVolt2Event=TRUE;
			break;
		case 0x01806E615:
			memcpy((uint8_t*)&g_BmsCellVolt3,data,len);
			BmsCellVolt3Event=TRUE;
			break;
		case 0x01806E616:
			memcpy((uint8_t*)&g_BmsCellVolt4,data,len);
			BmsCellVolt4Event=TRUE;
			break;
		case 0x01806E617:
			memcpy((uint8_t*)&g_BmsCellVolt5,data,len);
			BmsCellVolt5Event=TRUE;
			break;
		case 0x01806E618:
			memcpy((uint8_t*)&g_BmsCellVolt6,data,len);
			BmsCellVolt6Event=TRUE;
			break;	
		case 0x01806E620:
			memcpy((uint8_t*)&g_BmsRtTemp,data,len);
			BmsRtTempEvent=TRUE;
			break;
		case 0x1806E5F4:
			memcpy((uint8_t*)&g_Bms_Charge,data,len);
			BmsRtChangEvent=TRUE;
			break;
		
		case HM7280_CCS_ID_IOT_DEVH:
			memcpy((uint8_t *)&g_Devid.DevidH, data, len);
		//CanHm7280UpdateTelemetry();
		break;
		
		case HM7280_CCS_ID_IOT_DEVL:
			memcpy((uint8_t *)&g_Devid.DevidL, data, len);
			//CanHm7280UpdateTelemetry();
			break;
	}
}

void CanTransmit(uint32_t id,uint8_t *data,uint8_t len)
{
	can_trasnmit_message_struct transmit_message;
	
	can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    transmit_message.tx_sfid = 0x00;
    transmit_message.tx_efid = id;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_EXTENDED;
    transmit_message.tx_dlen = 8;
	memcpy(transmit_message.tx_data,data,len);
	can_message_transmit(CAN0, &transmit_message);
	can_message_transmit(CAN1, &transmit_message);
}

#ifdef CAN_TRASMITER_SUPPORT
void Can0Transmit(can_trasnmit_message_struct *transmit_message)
{
	if(g_CanRecoverState[0].tx_disable||transmit_message->tx_efid==0x00)
	{	
		LogPrintf("CAN 0 tx disable %x\r\n",transmit_message->tx_efid);
		return;
	}
//	LogPrintf("CAN 0 tx disable %x\r\n",transmit_message->tx_efid);
//	my_printf_hex(transmit_message->tx_data,8);
	can_message_transmit(CAN0, transmit_message);
}



void Can1Transmit(can_trasnmit_message_struct *transmit_message)
{
	if(g_CanRecoverState[1].tx_disable||transmit_message->tx_efid==0x00)
	{	
		LogPrintf("CAN 1 tx disable %x\r\n",transmit_message->tx_efid);
		return;
	}
	
	LogPrintf("CAN 1 tx disable %x\r\n",transmit_message->tx_efid);
	my_printf_hex(transmit_message->tx_data,8);
	can_message_transmit(CAN1, transmit_message);
}


#endif



uint8_t watchdog_flag = 0,pag_watchcount=0; 
uint32_t  bat_rcap = 0;
uint8_t pag_watchwdg = 0;  // 0xaa

static uint32_t  CcsEnergyLimitReached = 0;	
static uint32_t g_CcsEnergyLimittime = 10000,energy_mWh = 0;  //设置的充电时间 单位S 

void clear_pag_watchwdg(void)
{
    pag_watchwdg = 0;
}


void set_CcsEnergyLimittime(uint32_t value)
{
	g_CcsEnergyLimittime = value;
	CcsEnergyLimitReached = 0;
//	save_time= value;
}


void set_CcsEnergy_mWh(uint32_t value)
{
	energy_mWh = value;
	CcsEnergyLimitReached = 0;
}

uint32_t get_bat_rcap_mWh(void)
{
	return bat_rcap;
}




void CanProc(void)
{
	#ifdef E_MOB48V_PROJECT
	static uint16_t temp16=0,tempbms_cur=0,tempbms_vol=0,tempvcu_cur=0,tempvcu_vol=0,count=0;
	static uint16_t AlarmWatchdogState=0,AlarmWatchdogState_blk=0,pag_count =0;
	uint32_t temp32;
    int16_t tempInt16,g_AC_ccsinput = 0,ccsvcu_cur = 0;
	uint8_t serial_high_cache[8];
	uint8_t serial_low_cache[8];
	uint8_t ppid[15];
	static uint64_t g_CcsEnergyLimittime_count = 0;

	
//	uint8_t *p_u8;
//	uint32_t power;

	#ifdef CAN_TRASMITER_SUPPORT
	
	CanRecoveryProc(0,CAN0);
	CanRecoveryProc(1,CAN1);
	 
	g_CanAddr= g_UserSet.canid_cnt;
	
	if(g_CanRecoverState[0].tx_disable==0 && g_CanRecoverState[1].tx_disable==0)
		TimerSet(TIMER_SLEEP,SLEEP_PRIOD);
	
    if(HAL_GetTick()-g_CanTransmitState.t10ms>=10)
    {
    	g_CanTransmitState.t10ms=HAL_GetTick();

		if(g_CanTransmitState.can0_count)
	    {
			Can0Transmit(&g_can0TxMessage[0]);
			CanFifoProc(g_can0TxMessage,g_CanTransmitState.can0_count);
			g_CanTransmitState.can0_count--;
	    }
		//HAL_Delay(2);
		
		if(g_CanTransmitState.can1_count)
		{
			Can1Transmit(&g_can1TxMessage[0]);
			//HAL_Delay(10);
			//Can1Transmit(&g_can1TxMessage[0]);
			CanFifoProc(g_can1TxMessage,g_CanTransmitState.can1_count);
			g_CanTransmitState.can1_count--;
	    }
		//HAL_Delay(2);
		
		count++;
		if(count >=200)
		{
			count = 0;
			if(tempvcu_cur >0)
			{
				if(tempvcu_cur > tempbms_cur)
				{
					g_can0TxMessage_bms.tx_sfid = 0x00;
					g_can0TxMessage_bms.tx_efid = 0x1806e640;
					g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
					g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
					g_can0TxMessage_bms.tx_dlen = 8; 
					
					g_can0TxMessage_bms.tx_data[0] = tempbms_vol>>8;
					g_can0TxMessage_bms.tx_data[1] = tempbms_vol;
					
					tempbms_cur /=4;
					g_can0TxMessage_bms.tx_data[2] = tempbms_cur>>8;
					g_can0TxMessage_bms.tx_data[3] = tempbms_cur;
					
					if(tempbms_cur> 0) g_can0TxMessage_bms.tx_data[4] = 1;
				  else g_can0TxMessage_bms.tx_data[4] = 0;
				 
					can_message_transmit(CAN0, &g_can0TxMessage_bms);
				}
				else
				{
					
					g_can0TxMessage_bms.tx_sfid = 0x00;
					g_can0TxMessage_bms.tx_efid = 0x1806e640;
					g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
					g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
					g_can0TxMessage_bms.tx_dlen = 8;				
					g_can0TxMessage_bms.tx_data[0] = tempbms_vol>>8;
					g_can0TxMessage_bms.tx_data[1] = tempbms_vol;
					
					tempbms_cur /=4;
					g_can0TxMessage_bms.tx_data[2] = tempvcu_cur>>8;
					g_can0TxMessage_bms.tx_data[3] = tempvcu_cur;
					
					if(tempbms_cur> 0) g_can0TxMessage_bms.tx_data[4] = 1;
				  else g_can0TxMessage_bms.tx_data[4] = 0;
					can_message_transmit(CAN0, &g_can0TxMessage_bms);
				 
				}
			}
			else
			{
				
				if(g_AC_ccsinput > 2100) 
				{
					ccsvcu_cur += 100;
					if(ccsvcu_cur > (tempbms_cur*0.95))
					ccsvcu_cur = tempbms_cur*0.95;
				}
				else if(g_AC_ccsinput > 2000) 
				{
					//if((g_Battcharge_cur*X2) > dc_charge_cur)
					//dc_charge_cur += 50;
					ccsvcu_cur = tempbms_cur*0.5;
				}
				else if(g_AC_ccsinput > 1900) 
				{
					//if(((g_Battcharge_cur*X2) > dc_charge_cur)&&(dc_charge_cur > 20))
					//dc_charge_cur -= 20;
					ccsvcu_cur = tempbms_cur*0.2;
				}
				else ccsvcu_cur  = 0;
				
				
				tempbms_cur =tempbms_cur / 4;
				
				if(CcsEnergyLimitReached ==1)
				{
					tempbms_cur = 0;
					ccsvcu_cur = 200;
				}
				
				
				g_can0TxMessage_bms.tx_sfid = 0x00;
				g_can0TxMessage_bms.tx_efid = 0x1806e640;
				g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
				g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
				g_can0TxMessage_bms.tx_dlen = 8;
				g_can0TxMessage_bms.tx_data[0] = tempbms_vol>>8;
				g_can0TxMessage_bms.tx_data[1] = tempbms_vol;
				
				g_can0TxMessage_bms.tx_data[2] = tempbms_cur>>8;
				g_can0TxMessage_bms.tx_data[3] = tempbms_cur;
				
				if(tempbms_cur> 0) g_can0TxMessage_bms.tx_data[4] = 1;
				else g_can0TxMessage_bms.tx_data[4] = 0;
					
				can_message_transmit(CAN0, &g_can0TxMessage_bms);
				
			//	g_can0TxMessage_bms.tx_efid = 0x1806E5F4;
			////can_message_transmit(CAN1, &g_can0TxMessage_bms);//to vcu dispaly
				LogPrintf("-2222 %d  %d-g_AC_ccsinput %d  ccsvcu_cur %d\r\n",tempbms_cur,tempbms_vol,g_AC_ccsinput,ccsvcu_cur);
				if(energy_mWh > 0)
					LogPrintf("-2222 bat_rcap %d  energy_mWh %d- g_UserSet.lowbat %d \r\n",bat_rcap , energy_mWh, g_UserSet.lowbat);
					
			}
			
			//HAL_Delay(2);
		}
    }
	
	HAL_GPIO_WritePin(CAN1LED_GPIO_Port,CAN1LED_Pin,SET);
	HAL_GPIO_WritePin(CAN2LED_GPIO_Port,CAN2LED_Pin,SET);
	
	
//		if(pag_watchwdg == 0) 
//		{
//			if((PaygGetFreeState())) //free
//			{
//				AlarmWatchdogState &= ~0x02; //开门狗 使能 关闭  1 打开
//			}
//			else
//			{
//				AlarmWatchdogState |= 0x02;
//			}
//		} 
//		
//		if(AlarmWatchdogState != AlarmWatchdogState_blk)
//		{	
//			g_can0TxMessage_bms.tx_sfid = 0x00;
//			g_can0TxMessage_bms.tx_efid = 0x18FF50E5;
//			g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
//			g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
//			g_can0TxMessage_bms.tx_dlen = 8;
//			g_can0TxMessage_bms.tx_data[0] = AlarmWatchdogState;
//			//CanTransmit(HM7280_IOT_ID_ALARM_WDG,(uint8_t*)&g_Hm7280IotAlarmWdg,8);// 警告以及看门狗状态
//			can_message_transmit(CAN0, &g_can0TxMessage_bms);
//			AlarmWatchdogState_blk =  AlarmWatchdogState;
//		}
	
	
	if(pag_watchwdg == 0) // 下发本机PPID
	{
		g_can0TxMessage_bms.tx_sfid = 0x00;
		g_can0TxMessage_bms.tx_efid = 0x1806E55EUL;
		g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
		g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
		g_can0TxMessage_bms.tx_dlen = 8;
		
		memcpy(g_can0TxMessage_bms.tx_data,g_Hm7280IotSerialHigh, 8);
		can_message_transmit(CAN0, &g_can0TxMessage_bms);
		
		g_can0TxMessage_bms.tx_efid = 0x1806E55FUL;
		memcpy(g_can0TxMessage_bms.tx_data,g_Hm7280IotSerialLow, 8);
		can_message_transmit(CAN0, &g_can0TxMessage_bms);
		//CanTransmit(HM7280_IOT_ID_SERIAL_HIGH,g_Hm7280IotSerialHigh,8);
		//CanTransmit(HM7280_IOT_ID_SERIAL_LOW,g_Hm7280IotSerialLow,8);
		pag_count++;
		if(pag_count >=5) 
		{
			pag_watchwdg = 0xaa;
			pag_count = 0;
		}	
	}
		
	
	if(HAL_GetTick()-g_CanTransmitState.t5000ms>=5000) // 分析CAN数据
    {
    	g_CanTransmitState.t5000ms=HAL_GetTick();
		
		if(g_Hm7280SerialDirty == FALSE)
		{
			g_can0TxMessage_bms.tx_sfid = 0x00;
			g_can0TxMessage_bms.tx_efid = 0x1806E641UL;
			g_can0TxMessage_bms.tx_ft = CAN_FT_DATA;
			g_can0TxMessage_bms.tx_ff = CAN_FF_EXTENDED;
			g_can0TxMessage_bms.tx_dlen = 8;
			g_can0TxMessage_bms.tx_data[0] = AlarmWatchdogState;
			
			if((PaygGetPayRemainDays() >0)||(PaygGetFreeState()))
			{
				g_can0TxMessage_bms.tx_data[1] = 0x01;
			}
			else
			{
				g_can0TxMessage_bms.tx_data[1] = 0x00;
			}
			 
			if(pag_watchwdg == 0) 
			{
				if((PaygGetFreeState())) //free
				{
					g_can0TxMessage_bms.tx_data[2] = 0x00; //开门狗 使能 关闭  1 打开
				}
				else
				{
					g_can0TxMessage_bms.tx_data[2] = 0x01;
				}
				
			} 
			g_can0TxMessage_bms.tx_data[0] = 0x01;
			can_message_transmit(CAN0, &g_can0TxMessage_bms); // 心跳以及喂狗
		}
		
		
		 memcpy(serial_high_cache, g_Devid.DevidH, sizeof(serial_high_cache));
		 memcpy(serial_low_cache,  g_Devid.DevidL, sizeof(serial_low_cache));
		
		CanHm7280BuildSerialPayload(); // 获取本机的PPID

		if(memcmp(serial_high_cache, g_Hm7280IotSerialHigh, sizeof(serial_high_cache)) != 0 //
			|| memcmp(serial_low_cache, g_Hm7280IotSerialLow, sizeof(serial_low_cache)) != 0)
		{		
			g_Hm7280SerialDirty = TRUE;
			printf("ID SAME NOT.......%s-----%s%s..\r\n", g_Hm7280Serial,serial_high_cache,serial_low_cache);
		}
		else
		{
			g_Hm7280SerialDirty = FALSE;
			printf("ID SAME \r\n"  );
		}
	}
	
	
	if(BmsRtChangEvent)
	{
		BmsRtChangEvent=FALSE;
		tempbms_cur = (g_Bms_Charge.chargeCurtLimitH<<8)|g_Bms_Charge.chargeCurtLimitL;
		tempbms_vol = (g_Bms_Charge.chargeVolLimitH<<8)|g_Bms_Charge.chargeVolLimitL;	
		
		// LogPrintf("-1111can cur %x-can0 %x\r\n",tempbms_cur,tempbms_vol);
	}
	
	if(McuCCSEvent)  // vcu XIA FA 
	{
		McuCCSEvent=FALSE;
		tempvcu_cur = (g_McuCCSOut.chargeCurtLimitH<<8)|g_McuCCSOut.chargeCurtLimitL;
		tempvcu_vol = (g_McuCCSOut.chargeVolLimitH<<8) |g_McuCCSOut.chargeVolLimitL;
	}
	else
	{
		
	
	}
	
	
	
	if(BmsRtState1Event)
	{
		BmsRtState1Event=FALSE;
		g_AC_ccsinput = (g_BmsRtStatus1.acinputh<<8) |g_BmsRtStatus1.acinputl ;
		
		
	}

    g_CcsEnergyLimittime_count = g_UserSet.time*1000*60 ;
	if (g_CcsEnergyLimittime > 0 &&  ((HAL_GetTick() - g_CcsEnergyLimittime) > (g_CcsEnergyLimittime_count)))  // 设置时间计算 分钟为单位- g_UserSet.time_blk
	{
		CcsEnergyLimitReached = 1;
		g_CcsEnergyLimittime = 0;
		printf("ID g_CcsEnergyLimittime TIME OVER  %d\r\n" ,g_UserSet.time );
	}

	bat_rcap = (g_BmsRtStatus3.RemainBatCapH<<8)|(g_BmsRtStatus3.RemainBatCapL);
	
	
	if((energy_mWh>0)&&(bat_rcap - energy_mWh>= g_UserSet.lowbat*0.9))
	{
		energy_mWh = 0;
		CcsEnergyLimitReached = 1;
	}
	
	return ;
	
	
	
	
	
	
	
	
	
	
	#else
    if(HAL_GetTick()-g_CanTransmitState.t1000ms>=100)
    {
    	g_CanTransmitState.t1000ms=HAL_GetTick();

		g_Vcu_0x00633.Heartbeat=1;
		g_Vcu_0x00633.Wdgrefresh=1;
		CanTransmit(0x1806E633,(uint8_t*)&g_Vcu_0x00633,8);
    	}
	#endif



	//SetDashBoardData(LIST_speed,10);

	if(McuFaultEvent)
	{
		McuFaultEvent=FALSE;
		tempInt16=g_McuFaultInfor.ContrlTemp-40;
		GattSetData(LIST_DTA,DTA_CTMP,(uint8_t*)&tempInt16);
		
		tempInt16=g_McuFaultInfor.MotorTemp-40;
		GattSetData(LIST_DTA,DTA_MTPM,(uint8_t*)&tempInt16);

		SetDashBoardData(LIST_gearlevel,g_McuFaultInfor.GearState);
		SetDashBoardData(LIST_reverse,g_McuFaultInfor.ReverseState);

		SetDashBoardData(LIST_check,g_McuFaultInfor.BrakeState);
		SetDashBoardData(LIST_electrical,g_McuFaultInfor.MotorBlock_fault|g_McuFaultInfor.MotorHall_fault
			|g_McuFaultInfor.MotorOverTemp_fault|g_McuFaultInfor.MotorPhaseLost_fault);
		SetDashBoardData(LIST_handle,g_McuFaultInfor.Throttle_fault);
		SetDashBoardData(LIST_Ecu,0);
   /* if(g_McuFaultInfor.BrakeValue == 0)
		{
			SetDashBoardData(LIST_check,0);
		}
		else*/
		{
			//SetDashBoardData(LIST_check,g_McuFaultInfor.BrakeState);
		}
		/*if (g_McuFaultInfor.EBSState == 1 && g_McuFaultInfor.HillHolderState == 0)
		{
			SetDashBoardData(LIST_gearlevel, 1);
		}
		if (g_McuFaultInfor.EBSState == 0 && g_McuFaultInfor.HillHolderState == 1)
		{
			SetDashBoardData(LIST_gearlevel, 2);
		}
		if (g_McuFaultInfor.EBSState == 1 && g_McuFaultInfor.HillHolderState == 1)
		{
			SetDashBoardData(LIST_gearlevel, 3);
		}*/

		//SetDashBoardData(LIST_gearlevel, g_McuFaultInfor.GearState);

		if(g_McuFaultInfor.PauseDisableStae)
			SetDashBoardData(LIST_parking,TRUE);
		else
			SetDashBoardData(LIST_parking,FALSE);
		
	}
	
	if(McuRunInforEvent)
	{
		temp16=(g_McuRunInfor.MotorSpeedH<<8)|g_McuRunInfor.MotorSpeedL;
		GattSetData(LIST_DTA,DTA_MTRD,(uint8_t*)&temp16);
		SetDashBoardData(LIST_speed, (uint16_t)temp16*0.0129); // *0.01294
		
		temp16=(g_McuRunInfor.TyreSpeedH<<8)|g_McuRunInfor.TyreSpeedL;
		
		GattSetData(LIST_DTA,DTA_TSPD,(uint8_t*)&temp16);
		

		temp16=(g_McuRunInfor.RtVoltageH<<8)|g_McuRunInfor.RtVoltageL;
		temp16=temp16/10;
		GattSetData(LIST_DTA,DTA_RVLT,(uint8_t*)&temp16);

		tempInt16=(int16_t)((g_McuRunInfor.RtCurrentH<<8)|g_McuRunInfor.RtCurrentL);
		//tempInt16=tempInt16/10;
		//tempInt16-=2000;
		tempInt16*=100;
		GattSetData(LIST_DTA,DTA_RCUR,(uint8_t*)&tempInt16);

		McuRunInforEvent=FALSE;
	}
	if(McuPwrOutEvent)
	{
		McuRunInforEvent=FALSE;
	}
	if(McuSysInfor1Event)
	{
		temp16=(g_McuSysInfor1.RateMaxInputCurtH<<8)|g_McuSysInfor1.RateMaxInputCurtL;
		temp16=temp16/10;
		GattSetData(LIST_DTA,DTA_RMAX,(uint8_t*)&temp16);
		McuSysInfor1Event=FALSE;
	}
	if(McuSysInfor2Event)
	{
		McuSysInfor2Event=FALSE;

		temp16=(g_McuSysInfor2.MaxSpeedSetH<<8)|g_McuSysInfor2.MaxSpeedSetL;
		GattSetData(LIST_DTA,DTA_CMXS,(uint8_t*)&temp16);

		temp16=(g_McuSysInfor2.MaxInputCurtSetH<<8)|g_McuSysInfor2.MaxInputCurtSetL;
		GattSetData(LIST_DTA,DTA_CMXC,(uint8_t*)&temp16);
	}
	
	

//	if(BmsRtState1Event)
//	{
//		temp32=(g_BmsRtStatus1.RealtimeVoltageH<<8)+g_BmsRtStatus1.RealtimeVoltageL;
//		temp32*=100;

//		GattSetData(LIST_DTA,DTA_PCKV,(uint8_t*)&temp32);

//		SetDashBoardData(LIST_volts,(uint8_t)(temp32/1000));
//		
//		tempInt16=(int16_t)((g_BmsRtStatus1.RealtimeCurrentH<<8)+g_BmsRtStatus1.RealtimeCurrentL);

//		tempInt16-=2000;
//		tempInt16*=100;
//		
//		GattSetData(LIST_DTA,DTA_PCKC,(uint8_t*)&tempInt16);
//		//GattSetData(LIST_DTA,DTA_RCUR,(uint8_t*)&tempInt16);
//		

//		if(tempInt16>0)
//		{	power=temp32/100*tempInt16/100/100;
//			temp16=power;
//			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
//			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
//			temp16=0;
//			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
//		}
//		else if(tempInt16<0)
//		{	power=temp32/100*-tempInt16/100/100;
//			temp16=0;
//			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
//			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
//			temp16=power;
//			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
//		}
//		else
//		{
//			temp16=0;
//			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
//			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
//			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
//		}

//		temp16=g_BmsRtStatus1.ChargingMosOn;
//		GattSetData(LIST_DTA,DTA_CMOS,(uint8_t*)&temp16);
//		temp16=g_BmsRtStatus1.DisChargeMosOn;
//		GattSetData(LIST_DTA,DTA_DMOS,(uint8_t*)&temp16);

//		p_u8=(uint8_t*)&g_BmsRtStatus1;
//		temp16=p_u8[0]|(p_u8[1]<<8);
//		temp16&=0x3ffc;
//		GattSetData(LIST_DTA,DTA_PPST,(uint8_t*)&temp16);

//		temp16=p_u8[0]&0x03;
//		GattSetData(LIST_DTA,DTA_PMCS,(uint8_t*)&temp16);
//		BmsRtState1Event=FALSE;
//	}
	if(BmsRtState2Event)
	{
		temp16=g_BmsRtStatus2.Soc;
		GattSetData(LIST_DTA,DTA_RSOC,(uint8_t*)&temp16);

		SetDashBoardData(LIST_soc,(uint8_t)temp16);
		BmsRtState2Event=FALSE;
	}
	if(BmsRtState3Event)
	{
		temp16=(g_BmsRtStatus3.CycleNumbH<<8)+g_BmsRtStatus3.CycleNumbL;
		GattSetData(LIST_DTA,DTA_ACYC,(uint8_t*)&temp16);

		temp32=temp16;//acyc
		

		temp16=(g_BmsRtStatus3.FullBatCapH<<8)+g_BmsRtStatus3.FullBatCapL;
		temp16*=10;
		GattSetData(LIST_DTA,DTA_FCCP,(uint8_t*)&temp16);

		temp32=(temp32+1)*temp16; //cyc 
		

		temp16=(g_BmsRtStatus3.RemainBatCapH<<8)+g_BmsRtStatus3.RemainBatCapL;
		temp16*=10;
		GattSetData(LIST_DTA,DTA_RCAP,(uint8_t*)&temp16);

		temp32-=temp16;

		temp16=temp32/1000;

		GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&temp16);
		BmsRtState3Event=FALSE;
	}
	if(BmsCellVolt1Event)
	{
		temp16=(g_BmsCellVolt1.CellVoltage1H<<8)+g_BmsCellVolt1.CellVoltage1L;
		GattSetData(LIST_DIA,DIA_CV01,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1.CellVoltage2H<<8)+g_BmsCellVolt1.CellVoltage2L;
		GattSetData(LIST_DIA,DIA_CV02,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1.CellVoltage3H<<8)+g_BmsCellVolt1.CellVoltage3L;
		GattSetData(LIST_DIA,DIA_CV03,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1.CellVoltage4H<<8)+g_BmsCellVolt1.CellVoltage4L;
		GattSetData(LIST_DIA,DIA_CV04,(uint8_t*)&temp16);
		
		BmsCellVolt1Event=FALSE;
	}
	if(BmsCellVolt2Event)
	{
		temp16=(g_BmsCellVolt2.CellVoltage5H<<8)+g_BmsCellVolt2.CellVoltage5L;
		GattSetData(LIST_DIA,DIA_CV05,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt2.CellVoltage6H<<8)+g_BmsCellVolt2.CellVoltage6L;
		GattSetData(LIST_DIA,DIA_CV06,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt2.CellVoltage7H<<8)+g_BmsCellVolt2.CellVoltage7L;
		GattSetData(LIST_DIA,DIA_CV07,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt2.CellVoltage8H<<8)+g_BmsCellVolt2.CellVoltage8L;
		GattSetData(LIST_DIA,DIA_CV08,(uint8_t*)&temp16);
		McuSysInfor1Event=FALSE;
	}
	if(BmsCellVolt3Event)
	{
		temp16=(g_BmsCellVolt3.CellVoltage9H<<8)+g_BmsCellVolt3.CellVoltage9L;
		GattSetData(LIST_DIA,DIA_CV09,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt3.CellVoltage10H<<8)+g_BmsCellVolt3.CellVoltage10L;
		GattSetData(LIST_DIA,DIA_CV10,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt3.CellVoltage11H<<8)+g_BmsCellVolt3.CellVoltage11L;
		GattSetData(LIST_DIA,DIA_CV11,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt3.CellVoltage12H<<8)+g_BmsCellVolt3.CellVoltage12L;
		GattSetData(LIST_DIA,DIA_CV12,(uint8_t*)&temp16);
		BmsCellVolt3Event=FALSE;
	}
	if(BmsCellVolt4Event)
	{
		temp16=(g_BmsCellVolt4.CellVoltage13H<<8)+g_BmsCellVolt4.CellVoltage13L;
		GattSetData(LIST_DIA,DIA_CV13,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4.CellVoltage14H<<8)+g_BmsCellVolt4.CellVoltage14L;
		GattSetData(LIST_DIA,DIA_CV14,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4.CellVoltage15H<<8)+g_BmsCellVolt4.CellVoltage15L;
		GattSetData(LIST_DIA,DIA_CV15,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4.CellVoltage16H<<8)+g_BmsCellVolt4.CellVoltage16L;
		GattSetData(LIST_DIA,DIA_CV16,(uint8_t*)&temp16);
		BmsCellVolt4Event=FALSE;
	}
	if(BmsCellVolt5Event)
	{
		temp16=(g_BmsCellVolt5.CellVoltage17H<<8)+g_BmsCellVolt5.CellVoltage17L;
		GattSetData(LIST_DIA,DIA_CV17,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5.CellVoltage18H<<8)+g_BmsCellVolt5.CellVoltage18L;
		GattSetData(LIST_DIA,DIA_CV18,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5.CellVoltage19H<<8)+g_BmsCellVolt5.CellVoltage19L;
		GattSetData(LIST_DIA,DIA_CV19,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5.CellVoltage20H<<8)+g_BmsCellVolt5.CellVoltage20L;
		GattSetData(LIST_DIA,DIA_CV20,(uint8_t*)&temp16);
		BmsCellVolt5Event=FALSE;
	}

	if(BmsCellVolt6Event)
	{
		temp16=(g_BmsCellVolt6.CellVoltage21H<<8)+g_BmsCellVolt6.CellVoltage21L;
		GattSetData(LIST_DIA,DIA_CV21,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt6.CellVoltage22H<<8)+g_BmsCellVolt6.CellVoltage22L;
		GattSetData(LIST_DIA,DIA_CV22,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt6.CellVoltage23H<<8)+g_BmsCellVolt6.CellVoltage23L;
		GattSetData(LIST_DIA,DIA_CV23,(uint8_t*)&temp16);
		//temp16=(g_BmsCellVolt6.CellVoltage24H<<8)+g_BmsCellVolt6.CellVoltage24L;
		//GattSetData(LIST_DIA,DIA_CV23,(uint8_t*)&temp16);
		BmsCellVolt6Event=FALSE;
	}
	if(BmsRtTempEvent)
	{
		int rt_temp=0;
		BmsRtTempEvent=FALSE;

		rt_temp=g_BmsRtTemp.BmsTemperature1-40;
		GattSetData(LIST_DIA,DIA_TEMP1,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp.BmsTemperature2-40;
		GattSetData(LIST_DIA,DIA_TEMP2,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp.BmsTemperature3-40;
		GattSetData(LIST_DIA,DIA_TEMP3,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp.BmsTemperature4-40;
		GattSetData(LIST_DIA,DIA_TEMP4,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp.BmsTemperature7-40;
		GattSetData(LIST_DIA,DIA_TEMP5,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp.BmsTemperature8-40;
		GattSetData(LIST_DIA,DIA_TEMP6,(uint8_t*)&rt_temp);
	}
	if(BmsSysInforEvent)
	{
		BmsSysInforEvent=FALSE;
	} 
	#endif
}




