#ifndef _SDT_PACKET_H
#define _SDT_PACKET_H

#include "pgx_ts_common.h"

class LIB_TINY_TS_API CSDTPacket
{
public:
	enum{
		SIZE_OF_MAX_SDT_PACKET = 184
	};

	CSDTPacket(int nPid=0x11);
	virtual ~CSDTPacket();

	void Initialize();
	int  GetSDTPid();
	int FillPATPacket();
	u8* GetSDTBuff();
	void SetDescriptor(const char *szProviderName, const char *szServiceName);

protected:

private:
	ts_sdt_t m_sdt;
	u8 m_SDTBuff[SIZE_OF_MAX_SDT_PACKET];
	int m_nBuffOffset;
	int m_streamID;

	int m_nPid;
	
};

#endif