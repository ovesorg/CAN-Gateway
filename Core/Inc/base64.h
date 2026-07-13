
#ifndef __BASE64_H__

#define __BASE64_H__



void Base64Init(void);
void Base64Encode(const char* data,char* out,uint16_t data_len);
uint16_t Base64Decode(char *data,char* output);


#endif


