#ifndef	_SCI_H
#define	_SCI_H

#define AFE_ID				0x34

//Define MTP register addr
#define MTP_OTC             0x11
#define MTP_OTCR            0x12
#define MTP_UTC             0x13
#define MTP_UTCR            0x14
#define MTP_OTD             0x15
#define MTP_OTDR            0x16
#define MTP_UTD             0x17
#define MTP_UTDR            0x18
#define MTP_TR              0x19

#define MTP_CONF			0x40
#define MTP_BALANCEH		0x41
#define MTP_BALANCEL		0x42
#define MTP_BSTATUS1		0x43
#define MTP_BSTATUS2		0x44
#define MTP_BSTATUS3		0x45
#define MTP_TEMP1			0x46
#define MTP_TEMP2			0x48
#define MTP_TEMP3			0x4A
#define MTP_CUR				0x4C
#define MTP_CELL1			0x4E
#define MTP_CELL2			0x50
#define MTP_CELL3			0x52
#define MTP_CELL4			0x54
#define MTP_CELL5			0x56
#define MTP_CELL6			0x58
#define MTP_CELL7			0x5A
#define MTP_CELL8			0x5C
#define MTP_CELL9			0x5E
#define MTP_CELL10			0x60
#define MTP_CELL11			0x62
#define MTP_CELL12			0x64
#define MTP_CELL13			0x66
#define MTP_CELL14			0x68
#define MTP_CELL15			0x6A
#define MTP_CELL16			0x6C
#define MTP_ADC2			0x6E
#define MTP_BFLAG1			0x70
#define MTP_BFLAG2			0x71
#define MTP_RSTSTAT			0x72

extern bit MTPWrite(U8 WrAddr, U8 Length, U8 xdata *WrBuf);
extern bit MTPRead(U8 RdAddr, U8 Length, U8 xdata *RdBuf);
extern void InitAFE(void);
extern void EnableAFEWdtCadc(void);


#endif

