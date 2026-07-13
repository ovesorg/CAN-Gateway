
#ifndef __OTA_H__

#define __OTA_H__

#define OTA_START_ADDR 0x8000000+1024*148
#define OTA_PAGE_NUM  100


typedef struct
{
	uint8_t devinfor[16];
	uint8_t time[16];
	uint8_t version[4];
	uint32_t length;
	uint8_t auth[16];
	uint8_t reserved[4];
	uint32_t crc32;// 升级固件CRC
}OTA_BIN_AUTH_TypeDef;

typedef struct
{
	uint8_t ver[8];
	uint8_t key[32];
	uint32_t filesize;
	uint32_t addr;
	uint32_t crc32;
	uint32_t time;
}OTA_COM_TypeDef;





void OtaInit(void);
void OtaProc(void);
void OtaAckPublishPayload(uint8_t *buf);

#endif


