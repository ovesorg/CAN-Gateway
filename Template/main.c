/*!
    \file    main.c
    \brief   led spark with systick, USART print and key example

    \version 2014-12-26, V1.0.0, firmware for GD32F10x
    \version 2017-06-20, V2.0.0, firmware for GD32F10x
    \version 2018-07-31, V2.1.0, firmware for GD32F10x
    \version 2020-09-30, V2.2.0, firmware for GD32F10x
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f10x_eval.h"



UART_HandleTypeDef huart1;//BLE
UART_HandleTypeDef huart2;//4G
UART_HandleTypeDef huart3;//485	DI
UART_HandleTypeDef huart4;//GPS
UART_HandleTypeDef huart5;//UART




FlagStatus receive_flag;
uint8_t transmit_number = 0x0;
can_receive_message_struct receive_message;
can_receive_message_struct receive1_message;

uint32_t led_flash =0 ;

/*!
    \brief      toggle the led every 500ms
    \param[in]  none
    \param[out] none
    \retval     none
*/



/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

void GpioInit(void);

void Uart1Init(void);
void Uart2Init(void);
void Uart3Init(void);
void Uart4Init(void);
void Uart5Init(void);

void CanInit(void);
void ExtiInit(void);


void UartConfig(uint32_t uart_no,uint32_t bandrate);

uint8_t testi2c[256];


int main(void)
{
	//uint32_t i;


	//uint8_t testbuf[128];
    /* configure systick */
	//SystemInit();
	
	__enable_irq();
	GpioInit();

	/* configure I2C */
//   i2c_config();

//	Uart1Init(); //BLE
	Uart2Init(); // BLE
	//Uart3Init(); //485  DI
	Uart4Init(); //UART
	//Uart5Init(); //UART
    //Sif_Init();
	 /* initialize EEPROM  */
   i2c_eeprom_init();

	#ifdef WDG_ENABLE
	/* confiure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz */
    fwdgt_config(5*2* 5000, FWDGT_PSC_DIV256);
    /* after 1.6 seconds to generate a reset */
    fwdgt_enable();
	#endif

	systick_config();
	EEpInit();

    TimerInit();
    CanInit();
    
#ifdef CAN_TRASMITER_SUPPORT
	CanRamInit();
	LogPrintf("system start\r\n");

//	while(1)
//	{	
//		#ifdef WDG_ENABLE
//		fwdgt_counter_reload();
//		#endif
//		CanProc();
//		if(TimerSleepState())
//		{
//			LogPrintf("system reset\r\n");
//			__disable_fault_irq(); 
//			NVIC_SystemReset();
//		}
//	}
 #endif 

//	#ifdef GPS_SUPPORT
    GpsInit();
//	#endif

	#ifdef BLE_ENABLE
	BleUartInit();
	#endif

	 
//	 GsmComInit();
	 GattInit();
	 PaygInit();
	 KeyBoardInit();

//	  AdcInit();
	 LogPrintf("main system reset");
	
	 CanRamInit();

	 HAL_Delay(200);
    /* configure CKOUT0 to output system clock */
    rcu_ckout0_config(RCU_CKOUT0SRC_CKSYS);

  
    while(1)
	{
				/* USER CODE END WHILE */
	  	#ifdef WDG_ENABLE		//HAL_IWDG_Refresh(&hiwdg);
		fwdgt_counter_reload();
		#endif

		CanProc();
		BleCmdProc();

		PaygProcess(); 
		
		PKeybordProc();
		  
		EEpProcess();
		// LogPrintf("main system reset  %d\r\n",led_flash);
 
		led_flash++;
		if(led_flash<20000)
		{
			HAL_GPIO_WritePin(SYSLED_GPIO_Port,SYSLED_Pin,SET);
//			
		}
		else if(led_flash<40000)
		{
			HAL_GPIO_WritePin(SYSLED_GPIO_Port,SYSLED_Pin,RESET);
//		if(led_flash == 20001)
		// LogPrintf("main system reset>>>>  %d\r\n",led_flash);
		}
		else if(led_flash>40000) led_flash= 0;
    }
}


void GpioInit(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	
	gpio_init(DS_SCLK_GPIO_Port, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, DS_SCLK_Pin);
	gpio_init(DS_IO_GPIO_Port, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, DS_IO_Pin);
	gpio_init(DS_CE_GPIO_Port, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, DS_CE_Pin);

	gpio_init(GSM_EN_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GSM_EN_Pin);
	gpio_init(GSM_RST_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GSM_RST_Pin);
	gpio_init(GSM_PWR_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GSM_PWR_Pin);

	gpio_init(CAN_INH_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN_INH_Pin);
	gpio_init(CAN_EN_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN_EN_Pin);

	gpio_init(GPS_PWR_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPS_PWR_Pin);
	
	gpio_init(BAT_PWR_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, BAT_PWR_Pin);


//	gpio_init(LCD_CS_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_CS_Pin);
//	gpio_init(LCD_CLK_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_CLK_Pin);
//	gpio_init(LCD_DAT_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_DAT_Pin);

	gpio_init(BL_CTRL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, BL_CTRL_Pin);
 
	gpio_init(SYSLED_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, SYSLED_Pin);
	gpio_init(CAN1LED_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN1LED_Pin);
	gpio_init(CAN2LED_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN2LED_Pin);
	
	
	gpio_init(CAN_STB_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN_STB_Pin);
	

	#ifdef LCD128X64_SUPPORT
	gpio_init(LCD_RST_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_RST_Pin);
	gpio_init(LCD_PWR_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_PWR_Pin);
	gpio_init(LCD_RS_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LCD_RS_Pin);

	HAL_GPIO_WritePin(LCD_PWR_GPIO_Port,LCD_PWR_Pin,SET);
	#endif

	HAL_GPIO_WritePin(CAN_STB_GPIO_Port,CAN_STB_Pin,RESET);

	HAL_GPIO_WritePin(CAN_INH_GPIO_Port,CAN_INH_Pin,SET);
	HAL_GPIO_WritePin(CAN_EN_GPIO_Port,CAN_EN_Pin,SET);

	HAL_GPIO_WritePin(GSM_RST_GPIO_Port,GSM_RST_Pin,RESET);
	HAL_GPIO_WritePin(GSM_PWR_GPIO_Port,GSM_PWR_Pin,RESET);

	gpio_init(I2C_SCL_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, I2C_SCL_PIN);
    gpio_init(I2C_SDA_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, I2C_SDA_PIN);

	//HAL_GPIO_WritePin(GPS_PWR_GPIO_Port, GPS_PWR_Pin, GPIO_PIN_SET);

	gpio_init(PWR_KEY_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, PWR_KEY_Pin);
	gpio_init(KEY_ENTER_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, KEY_ENTER_Pin);

	gpio_init(SW_A_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, SW_A_Pin);
	gpio_init(SW_B_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, SW_B_Pin);

    gpio_init(RELAY_EN_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, RELAY_EN_Pin);

	gpio_init(PWR_CTRL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, PWR_CTRL_Pin);
	//ble pwr
    HAL_GPIO_WritePin(BAT_PWR_GPIO_Port,BAT_PWR_Pin,SET);

	
    #ifdef CAN_TRASMITER_SUPPORT
	gpio_init(CAN_ADDR0_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, CAN_ADDR0_Pin);
	gpio_init(CAN_ADDR1_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, CAN_ADDR1_Pin);
	gpio_init(CAN_ADDR2_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, CAN_ADDR2_Pin);
	gpio_init(CAN_ADDR3_GPIO_Port, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, CAN_ADDR3_Pin);
	
	gpio_init(CAN1_STB_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN1_STB_Pin);
	gpio_init(CAN2_STB_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CAN2_STB_Pin);

	HAL_GPIO_WritePin(CAN1_STB_GPIO_Port,CAN1_STB_Pin,RESET);
	HAL_GPIO_WritePin(CAN2_STB_GPIO_Port,CAN2_STB_Pin,RESET);
	#endif
	ExtiInit();

	
}


void ExtiInit(void)
{
    /* enable the AF clock */
    rcu_periph_clock_enable(RCU_AF);
    /* enable and set key EXTI interrupt to the specified priority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    nvic_irq_enable(EXTI0_IRQn, 2U, 2U);

    /* connect key EXTI line to key GPIO pin */
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_0);

    /* configure key EXTI line */
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);
}



void Uart1Init(void)//BLE
{
	/* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
    /* configure COM0 */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    UartConfig(USART0,38400U);

}

void Uart2Init(void)//GSM
{
	/* USART interrupt configuration */
    nvic_irq_enable(USART1_IRQn, 0, 0);
    /* configure COM0 */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    UartConfig(USART1,38400U);

}

void Uart3Init(void)  //DI   RS485
{
	/* USART interrupt configuration */
    nvic_irq_enable(USART2_IRQn, 0, 0);
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    UartConfig(USART2,9600U);

}

void Uart4Init(void) //GPS
{
	/* USART interrupt configuration */
    nvic_irq_enable(UART3_IRQn, 0, 0);
    /* configure COM0 */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_UART3);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    UartConfig(UART3,9600u);

}

void Uart5Init(void) //EXT UART
{
	/* USART interrupt configuration */
    nvic_irq_enable(UART4_IRQn, 0, 0);
    /* configure COM0 */
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_UART4);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    UartConfig(UART4,115200U);

}





void UartConfig(uint32_t uart_no,uint32_t bandrate)
{
	/* USART configure */
	usart_deinit(uart_no);
	usart_baudrate_set(uart_no, bandrate);
	usart_word_length_set(uart_no, USART_WL_8BIT);
	usart_stop_bit_set(uart_no, USART_STB_1BIT);
	usart_parity_config(uart_no, USART_PM_NONE);
	usart_hardware_flow_rts_config(uart_no, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(uart_no, USART_CTS_DISABLE);
	usart_receive_config(uart_no, USART_RECEIVE_ENABLE);
	usart_transmit_config(uart_no, USART_TRANSMIT_ENABLE);
	usart_enable(uart_no);

	/* enable USART TBE interrupt */  
	usart_interrupt_enable(uart_no, USART_INT_FLAG_RBNE);
	usart_interrupt_enable(UART3, USART_INT_FLAG_IDLE);

}


void Uart1Send(uint8_t *buffer,uint16_t size)
{
	uint16_t i;
	
	for(i=0;i<size;i++)
	{
		 usart_data_transmit(USART1, buffer[i]);
   		 while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
		}
}

void Uart2Send(uint8_t *buffer,uint16_t size)
{
	uint16_t i;
	
	for(i=0;i<size;i++)
	{
		 usart_data_transmit(USART1, buffer[i]);
   		 while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
		}
}

void Uart3Send(uint8_t *buffer,uint16_t size)
{
	uint16_t i;
	
	for(i=0;i<size;i++)
	{
		 usart_data_transmit(USART2, buffer[i]);
   		 while(RESET == usart_flag_get(USART2, USART_FLAG_TBE));
		}
}

void Uart4Send(uint8_t *buffer,uint16_t size)
{
	uint16_t i;
	
	for(i=0;i<size;i++)
	{
		 usart_data_transmit(UART3, buffer[i]);
   		 while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
		}
}


void Uart5Send(uint8_t *buffer,uint16_t size)
{
	uint16_t i;
	
	for(i=0;i<size;i++)
	{
		 usart_data_transmit(UART3, buffer[i]);
   		 while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
	}
}


/*void LogPrintf(uint8_t *buffer,uint16_t size)
{
	extern USER_SET_TypeDef g_UserSet;
		
	if(g_UserSet.log)
	{	Uart5Send(buffer,size);
		}
}*/


void CanInit(void)
{
		can_parameter_struct			can_parameter;
		can_filter_parameter_struct 	can_filter;

		rcu_periph_clock_enable(RCU_AF);
		
		can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
		can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
		
		/* initialize CAN register */
		can_deinit(CAN0);
		#ifdef GD32F10X_CL
		can_deinit(CAN1);
		#endif

		rcu_periph_clock_enable(RCU_CAN0);
		
		#ifdef GD32F10X_CL
		rcu_periph_clock_enable(RCU_CAN1);
		#endif

		gpio_init(GPIOA,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_11);
        gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_12);

		//gpio_pin_remap_config(GPIO_CAN0_FULL_REMAP,FALSE);

		gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_12);
        gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
		//gpio_pin_remap_config(GPIO_CAN1_REMAP,FALSE);
		
		/* initialize CAN */
		can_parameter.time_triggered = DISABLE;
		can_parameter.auto_bus_off_recovery = DISABLE;
		can_parameter.auto_wake_up = DISABLE;
		can_parameter.auto_retrans = DISABLE;
		can_parameter.rec_fifo_overwrite = DISABLE;
		can_parameter.trans_fifo_order = DISABLE;
		can_parameter.working_mode = CAN_NORMAL_MODE;
		can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
		can_parameter.time_segment_1 = CAN_BT_BS1_14TQ;
		can_parameter.time_segment_2 = CAN_BT_BS2_3TQ;
		/* baudrate 1Mbps */
		can_parameter.prescaler = 12;  //250K
		can_init(CANX, &can_parameter);
		
		#ifdef GD32F10X_CL
		can_init(CAN1, &can_parameter);
		#endif
	
		/* initialize filter */
#ifdef  CAN0_USED
		/* CAN0 filter number */
		can_filter.filter_number = 0;
#else
		/* CAN1 filter number */
		can_filter.filter_number = 15;
#endif
		/* initialize filter */    
		can_filter.filter_mode = CAN_FILTERMODE_MASK;
		can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		can_filter.filter_list_high = 0x0000;
		can_filter.filter_list_low = 0x0000;
		can_filter.filter_mask_high = 0x0000;
		can_filter.filter_mask_low = 0x0000;  
		can_filter.filter_fifo_number = CAN_FIFO0;
		can_filter.filter_enable = ENABLE;
		can_filter_init(&can_filter);
		
		#ifdef GD32F10X_CL
		can_filter.filter_number = 15;
		can_filter_init(&can_filter);
		#endif


    /* enable CAN receive FIFO1 not empty interrupt */
    can_interrupt_enable(CANX, CAN_INT_RFNE0);
	
	#ifdef GD32F10X_CL
	can_interrupt_enable(CAN1, CAN_INT_RFNE0);
	#endif
    
    /* initialize transmit message */
   /* can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    transmit_message.tx_sfid = 0x321;
    transmit_message.tx_efid = 0x01;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_STANDARD;
    transmit_message.tx_dlen = 1;
    printf("please press the Tamper key to transmit data!\r\n");*/
    
    /* initialize receive message */
    can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &receive_message);

	
	nvic_irq_enable(CAN0_RX0_IRQn,0,0);
	
	#ifdef GD32F10X_CL
	nvic_irq_enable(CAN1_RX0_IRQn,1,1);
	#endif

}
const uint8_t ASCII[] ={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
uint8_t debugbuff[64];
uint8_t *UserLib_Hex2AscEx(uint8_t * dest, const uint8_t * src, uint16_t srcLength, const char * prefix, const char * decollator)
{
	uint8_t  temp;
	uint16_t prefixLen;
	uint16_t decollatorLen;
	
	if ((0 == srcLength) || (NULL == dest) || (NULL == src))
	{
		return dest;
	}
	
	if (NULL != prefix)
		prefixLen = strlen(prefix);
	if (NULL != decollator)
		decollatorLen = strlen(decollator);
	
	
	do{
		temp = 	*src++;
		if (NULL != prefix)
		{
			memcpy(dest, prefix, prefixLen);
			dest += prefixLen;
		}
	
		*dest++ = ASCII[temp >> 4];			//high 4 bit
		*dest++ = ASCII[temp & 0x0F];		//low 4 bit
		
		if ((NULL != decollator) && (srcLength > 1))
		{
			memcpy(dest, decollator, decollatorLen);
			dest += decollatorLen;
		}
		
	}while(--srcLength != 0);

	return dest;	
}
//extern USER_SET_TypeDef g_UserSet;

void my_printf_hex(uint8_t *src , uint16_t srclen)
{
    //if(g_UserSet.log==2) 
	{ 
        memset(debugbuff,0,sizeof(debugbuff));
        UserLib_Hex2AscEx((uint8_t *)debugbuff, src, srclen, NULL, " ");
        strcat((char *)debugbuff, "\r\n");
        printf("%s\r\n",debugbuff);
	}
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
	extern USER_SET_TypeDef g_UserSet;
	
	if(g_UserSet.log)
	{
	    usart_data_transmit(UART3, (uint8_t)ch);
	    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
	}
    return ch;
}
