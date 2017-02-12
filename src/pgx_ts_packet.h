 #ifndef _TS_PACKET_H
#define _TS_PACKET_H
#include "pgx_ts_common.h"
#include "pgx_ts_common_def.h"

class LIB_TINY_TS_API CTinyTSPacket
{
public:
	enum
	{
		SIZE_OF_TS_PACKET = 188,//bytes
		SIZE_OF_ADAPTION_LENGTH = 8,//bites
		SIZE_OF_ADAPTION_FIELD_FLAGS = 8,//bits
		SIZE_OF_PCR = 48,//bits
		SIZE_OF_TS_HEADER = 32,//bits
	};
public:
	CTinyTSPacket(int pid = 0x00);
	virtual ~CTinyTSPacket();
	void  SetPid(int pid);
	void  InitTSHeader();
	int   FillTSPacket(u8 *dataBuff, int nBufflen, int startIndicator, u8 adaptation = 0x00, u64 timeStamp = 0);
	u8* GetTSBuff();

	void SetTSHeader();
	void SetAdaption();
	void SetTSPayload();

	void SetPCR(u8 *buff, int &offset, u64 timeStamp);

private:
	u8      m_tsBuff[188];
	int       m_nTSBufOffset;
	ts_header m_ts;
	ts_adaption_t m_adaption;
	int m_pid;
	ts_pcr_t m_pcr;
};

#endif