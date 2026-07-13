#include"main.h"
#include"string.h"



uint8_t base64testSrc[256];
uint8_t base64testEncode[256];


static const char base64en[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',  'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P',  'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X',   'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f','g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v',  'w', 'x', 'y', 'z', '0', '1', '2',
        '3', '4', '5', '6', '7', '8', '9', '+', '/'};

// 解码表
static const unsigned char base64_suffix_map[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
        255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
        52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
        255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
        7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
        19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
        37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
        49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255 };




void Base64Init(void)
{
	uint16_t i=0;
	memset(base64testSrc,0x00,256);
	memset(base64testEncode,0x00,256);
	for(i=0;i<128;i++)
		base64testSrc[i]=i;

	
	Base64Encode(base64testSrc,base64testEncode,128);

	memset(base64testSrc,0x00,256);
	Base64Decode(base64testEncode,base64testSrc);

	LogPrintf("base64:%s",base64testEncode);
}


void Base64Encode(const char* data,char* out,uint16_t data_len)
{
    // 长度计算
    int index = 0;
    char c = '\0';
    char last_c = '\0';

	int i=0;
   // int data_len = strlen(data);
    if(data_len == 0)
	{
        out[0] = '\0';
        return;
    }



    for(i=0;i<data_len;i++)
	{
        c = data[i];
        switch (i%3)
		{
            case 0:
                out[index++] = base64en[(c>>2) & 0x3f];
                break;
            case 1:
                out[index++] = base64en[(last_c & 0x3) << 4 | ((c >> 4) & 0xf)];
                break;
            case 2:
                out[index++] = base64en[((last_c & 0xf)) << 2 | ((c >> 6) & 0x3)];
                out[index++] = base64en[c & 0x3f];
                break;
        }
        last_c = c;
    }

    if(data_len % 3 == 1){
        out[index++] = base64en[(c & 0x3) << 4];
        out[index++] = '=';
        out[index++] = '=';
    }

    if(data_len % 3 == 2){
        out[index++] = base64en[(c & 0xf) << 2];
        out[index++] = '=';
    }


}


uint16_t Base64Decode(char *data,char* output)
{
    int data_len = strlen(data);
    unsigned char c = '\0';

    int t = 0, y = 0, i = 0;
    int g = 3;
	int x=0;

    for( x=0;x<data_len;x++)
	{
        c = base64_suffix_map[data[x]];

        if(c == 255) 
			output[0] = '\0';
        if (c == 253)
			continue;// 对应的值是换行或者回车
        if (c == 254) 
		{ 	c = 0;
			g--; }// 对应的值是'='

        t = (t<<6) | c;

        if(++y == 4)
		{
            output[i++] = (t >> 16) & 0xff;
			
            if (g > 1) 
				output[i++] = (unsigned char)((t>>8)&0xff);
            if (g > 2)
				output[i++] = (unsigned char)(t&0xff);
			
            y = t = 0;
        }
    }

	return i;

}


