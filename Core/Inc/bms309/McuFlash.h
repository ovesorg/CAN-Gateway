#ifndef __MCU_FLASH_H
#define __MCU_FLASH_H


#define DATAFLASH_MAP_ADDR	        0x0000
#define DATAFLASH_ADDR				0x0000
#define DATAFLASH_B_ADDR			0x0100
#define DATAFLASH_LEN				0x0100

/**************************************************************************************/
//DataFlash中区块地址定义
/**************************************************************************************/
//系统信息区开始 SubClassID=0x00
#define SYS_PARA_MAP_ADDR		    0x0000
#define SYS_PARA_LEN				50

//用户自定义参数区开始 SubClassID=0x01
#define SYSINFO_MAP_ADDR			SYS_PARA_MAP_ADDR+SYS_PARA_LEN
#define SYSINFO_LEN					50

//充电参数区开始 SubClassID=0x02
#define CHG_PARA_MAP_ADDR			SYSINFO_MAP_ADDR+SYSINFO_LEN	
#define	CHG_PARA_LEN				6

//平衡参数区开始 SubClassID=0x08
#define	BAL_PARA_MAP_ADDR			CHG_PARA_MAP_ADDR+CHG_PARA_LEN
#define	BAL_PARA_LEN				8

//放电参数区开始 SubClassID=0x03
#define	DSG_PARA_MAP_ADDR			BAL_PARA_MAP_ADDR+BAL_PARA_LEN
#define DSG_PARA_LEN				4

//放电PWM参数区开始 SubClassID=0x04
#define DSG_PWM_PARA_MAP_ADDR		DSG_PARA_MAP_ADDR+DSG_PARA_LEN
#define DSG_PWM_PARA_LEN			0

//充电温度保护参数开始 SubClassID=0x05
#define CHG_TEMP_PARA_MAP_ADDR		DSG_PWM_PARA_MAP_ADDR+DSG_PWM_PARA_LEN
#define CHG_TEMP_PARA_LEN			0

//放电温度保护参数开始 SubClassID=0x06
#define DSG_TEMP_PARA_MAP_ADDR		CHG_TEMP_PARA_MAP_ADDR+CHG_TEMP_PARA_LEN
#define	DSG_TEMP_PARA_LEN			0

//AFE参数区开始 SubClassID=0x0A
#define	AFE_PARA_MAP_ADDR			DSG_TEMP_PARA_MAP_ADDR+DSG_TEMP_PARA_LEN
#define	AFE_PARA_LEN				27

//校准参数区开始 SubClassID=0x0B
#define CALI_PARA_MAP_ADDR			AFE_PARA_MAP_ADDR+AFE_PARA_LEN
#define	CALI_PARA_LEN				14

//Reserved区
#define RESERV_PARA_MAP_ADDR		CALI_PARA_MAP_ADDR + CALI_PARA_LEN
#define	RESERV_PARA_LEN				510 - (RESERV_PARA_MAP_ADDR)

#define DataflashCheck_Map_ADDR		0xfe
#define DataflashCheck_LEN			0x0002


//Define system macro
#define	CALI_FLAG				0xaa

//UART相关变量开始
#define UART_BUFPT_MAP_ADDR             0x400
#define UART_BUFPT_LEN                  1 
#define UART_SEND_LENGTH_MAP_ADDR       UART_BUFPT_MAP_ADDR+UART_BUFPT_LEN
#define UART_SEND_LENGTH_LEN            1
#define UART_TIMEOUT_CNT_MAP_ADDR       UART_SEND_LENGTH_MAP_ADDR+UART_SEND_LENGTH_LEN
#define UART_TIMEOUT_CNT_LEN            1
#define UART_BUF_MAP_ADDR               UART_TIMEOUT_CNT_MAP_ADDR+UART_TIMEOUT_CNT_LEN
#define UART_BUF_LEN                    150


extern bit FlashProcess(void);
extern BOOL ReadMcuFlash(U16 SourceAddr, U8 xdata *DestAddr, U16 Length);
extern BOOL WriteMcuFlash(U16 Saddr);

#endif



