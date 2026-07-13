#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__


#define KEYB_VERSION  33

#define UART5_RX_BUF_SIZE  512

 

#define UART3_RX_BUF_SIZE 512

 extern __IO uint8_t RxUart5Counter_flag ;
extern uint16_t RxUart5Counter;
 extern __IO uint8_t g_Uart5Buf[UART5_RX_BUF_SIZE];
void KeyBoardInit(void);
void PKeybordProc(void) ;
void Serial_Cmd(unsigned char cmd);
void Process_cmd_all(unsigned char cmd);
void Printf_Usart_num(unsigned char *str, int num);
void debug_printf(uint8_t*tstr,uint8_t*str,uint16_t value);
void Send_RechargeOK(void);
uint16_t hi_crc16(uint8_t *buffer, uint16_t length);


#endif

