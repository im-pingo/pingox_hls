 #ifndef _PMT_PACKET_H
#define _PMT_PACKET_H
#include "pgx_ts_common.h"
#include "pgx_ts_common_def.h"

class LIB_TINY_TS_API CTinyPMTPacket:public ts_pat_program_t
{
public:
	enum
	{
		SIZE_OF_MAX_PMT_PACKET = 184
	};

	CTinyPMTPacket(int PID, int pcrPID);
	virtual ~CTinyPMTPacket();

	void InitPacket();
	void AddStream(ts_pmt_stream_t *stream);
	void CleanStream();
	int GetPMTPID(){return m_pid;};
	int  FillPMTPacket();
	u8*  GetPMTBuff();

private:
	ts_pmt_t m_pmt;
	u8 m_pmtBuff[SIZE_OF_MAX_PMT_PACKET];
	int m_nBuffOffset;
	int m_pid;
};
#endif
