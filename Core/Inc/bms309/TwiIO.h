#ifndef	_TWI_IO_H
#define	_TWI_IO_H

#define TWI_IO_OPERATE
#define E2PROM_ID  0xA0

#ifdef TWI_IO_OPERATE

//#define SDA_BIT			0x10
//#define SCL_BIT			0x20
#define SDA				HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)//P0_4
#define SCL				HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)//P0_5

#define TWI_CLK_OUT		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);//P0CR |= SCL_BIT;
#define TWI_CLK_IN		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);//P0CR &= ~SCL_BIT;
#define TWI_CLK_HIGH	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);//TWI_CLK_IN; SCL = 1;
#define TWI_CLK_LOW	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);//TWI_CLK_OUT; SCL = 0;

#define TWI_DAT_OUT		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);//P0CR |= SDA_BIT;
#define TWI_DAT_IN      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);//P0CR &= ~SDA_BIT;
#define TWI_DAT_HIGH	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);//TWI_DAT_IN; SDA = 1;
#define TWI_DAT_LOW	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);//TWI_DAT_OUT; SDA = 0;

#define TWI_RD_DAT		SDA
#define TWI_RD_CLK		SCL

extern void InitTwi(void);
extern bit TwiRead(U8 SlaveID, U16 RdAddr, U8 Length, U8 xdata *RdBuf);
extern bit TwiWrite(U8 SlaveID, U16 WrAddr, U8 Length, U8 xdata *WrBuf);
extern U8 CRC8cal(U8 *p, U8 counter);


#endif

#endif

