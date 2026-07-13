#include"main.h"


OTA_COM_TypeDef g_OtaInfor;



void OtaInit(void)
{
	memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));
}


void OtaAckPublishPayload(uint8_t *buf)
{
	uint8_t*json=GattGetJsonBuff();
	
	memset(json,0x00,JSON_LEN);

	memcpy(json,"{\"cmd\":{",8);
	json+=strlen((char*)json);

	if(strlen((char*)buf)<JSON_LEN-10)
		memcpy(json,buf,strlen((char*)buf));

	json+=strlen((char*)json);

	memcpy(json,"}}",2);
	/*
	
	memcpy(json,"{\"ota\":\"ready\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d,\"size\":%d",g_OtaInfor.time,g_OtaInfor.filesize);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"ack\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d,\"addr\":%d",g_OtaInfor.time,g_OtaInfor.addr);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"boot\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d",g_OtaInfor.time);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"succ\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"ver\":%s",g_OtaInfor.time);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"err\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"code\":%s","crc");
	json+=strlen((char*)json);
	memcpy(json,"}}",2);*/

}

void OtaParse(uint8_t * buf)
{
	uint32_t size,time,crc;
	uint32_t addr,len;
	uint8_t i,ver[8]={0},key[16]={0};
	uint8_t firmware[256]={0};
	uint8_t tempBuff[128]={0};
	
	if(buf!=NULL)
	{
		uint8_t *p=NULL;
		p=strstr(buf,"ota\":");

		if(p!=NULL)
		{

			p=strstr(buf,"time");
			if(p!=NULL)
			{
				time=atol(p+7);

				g_OtaInfor.time=time;
				}
			
			p=strstr(buf,"upgrade");

			if(p!=NULL)
			{
				memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));
				
				for(i=0;i<OTA_PAGE_NUM/2;i++)
				{	
					FlashPageErase(OTA_START_ADDR+i*PAGE_SIZE);
					}
				p=strstr(buf,"ver");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(g_OtaInfor.ver,p+6,4);
					}

				p=strstr(buf,"key");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(g_OtaInfor.key,p+6,16);
					}

				p=strstr(buf,"size");  //文件大小
				if(p!=NULL)
				{
					g_OtaInfor.filesize=atol(p+6);
					}
				
				p=strstr(buf,"time");  //文件大小
				if(p!=NULL)
				{
					g_OtaInfor.time=atol(p+6);
					}

				memset(tempBuff,0x00,128);
				sprintf((char*)tempBuff,"\"ota\":\"ready\",\"time\":%d,\"size\":%d",g_OtaInfor.time,g_OtaInfor.filesize);
				OtaAckPublishPayload(tempBuff);

				/*p=strstr(buf,"crc");  //文件CRC32 
				if(p!=NULL)
				{
					g_OtaInfor.crc32=atol(p+5);
					}*/
				}


			p=strstr(buf,"firmware");

			if(p!=NULL)
			{
				p=strstr(buf,"addr");//数据包固件起始地址
				if(p!=NULL)
				{
					addr=atol(p+6);
					}

				p=strstr(buf,"len");//DATA数据包长度
				if(p!=NULL)
				{
					len=atol(p+5);
					}

				p=strstr(buf,"crc"); //base64 数据包 CRC
				if(p!=NULL)
				{
					crc=atol(p+7);
					}

				p=strstr(buf,"data");//base64 数据包
				if(p!=NULL)
				{
					uint8_t pack[256]={0};
					
					if(g_OtaInfor.addr==addr)
					{
						memcpy(pack,p+7,len);

						if(CRC16(pack, len)==crc)
						{
							len=Base64Decode(pack,firmware);
							FlashPageProgram(OTA_START_ADDR+g_OtaInfor.addr,len/4,(uint32_t*)firmware);
							g_OtaInfor.addr+=len;
							}
						}
					
					}

				memset(tempBuff,0x00,128);
				sprintf((char*)tempBuff,"\"ota\":\"ack\",\"time\":%d,\"addr\":%d",g_OtaInfor.time,g_OtaInfor.addr);
				OtaAckPublishPayload(tempBuff);
				}

			p=strstr(buf,"complete");

			if(p!=NULL)
			{

				p=strstr(buf,"ver");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(ver,p+6,4);
					}

				p=strstr(buf,"key");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(key,p+6,16);
					}

				/*p=strstr(buf,"size");  //文件大小
				if(p!=NULL)
				{
					size=atol(p+6);
					}*/

				if(g_OtaInfor.filesize==size)
				{
					OTA_BIN_AUTH_TypeDef auth;

					//memcpy((uint8_t*)&auth,(__IO uint8_t*)OTA_START_ADDR,sizeof(OTA_BIN_AUTH_TypeDef));

					//if()校验正确
					{
						//
						}

					}

				memset(tempBuff,0x00,128);
				sprintf((char*)tempBuff,"\"ota\":\"reboot\",\"time\":%d,",g_OtaInfor.time);
				OtaAckPublishPayload(tempBuff);
				}

			

			
			}
		}
}
void OtaProc(void)
{


}




