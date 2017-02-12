#include "pgx_ts_common.h"
const u8 g_AUD[SIZE_OF_AUD_LENGTH] ={0x00, 0x00, 0x00, 0x01, 0x09, 0xf0};

void AppendBits(u8 *buff, int &nOffset, int length, u8 data)
{
	int nTailLen = nOffset%8;
	int nPos = (nOffset-nTailLen)/8;
	u8 mask = 0xff;
	mask <<= length;
	mask ^= 0xff;
	data &= mask; 

	if (length <= 8-nTailLen)
	{
		buff[nPos] |= data<<(8-nTailLen-length);
		nOffset += length;
	}
	else
	{
		buff[nPos] |= data>>nTailLen;
		nOffset += 8-nTailLen;
		AppendBits(buff, nOffset, length-8+nTailLen, data);
	}
}

u32 CRC32(const u8 *data, int len)
{   
	int i;   
	u32 crc = 0xFFFFFFFF;
	for(i = 0; i < len; i++)
		crc = (crc << 8) ^ crc32table[((crc >> 24) ^ *data++) & 0xFF];       
	return crc;
}
