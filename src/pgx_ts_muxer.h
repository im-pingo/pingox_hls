 #ifndef _TS_MUXER_H
#define _TS_MUXER_H
#include "string"
#include "pgx_ts_packet.h"
#include "pgx_pat_packet.h"
#include "pgx_pmt_packet.h"
#include "pgx_pes_packet.h"
#include <map>
#include "pgx_ts_common_def.h"
#include "pgx_sdt_packet.h"

using namespace std;

class LIB_TINY_TS_API CTinyTSMuxer
{
public:
	enum
	{
		DEFAULT_TRANSPORT_STREAM_ID = 1,
		DEFAULT_PMT_PID = 0X100,
		DEFAULT_PES_AUDIO_PID = 0X101,
		DEFAULT_PES_VIDEO_PID = 0X102,
	};

	enum{
		TS_TYPE_PAT = 0x01000000,
		TS_TYPE_PMT = 0x02000000,
		TS_TYPE_PES = 0x03f00000,
		TS_TYPE_SDT = 0x04000000,

		TS_TYPE_PES_AUDIO         = 0x03100000,
		TS_TYPE_PES_VIDEO_I_FRAME = 0x03200000,
		TS_TYPE_PES_VIDEO_P_FRAME = 0x03400000,
		TS_TYPE_PES_VIDEO_E_FRAME = 0x03800000,
	};

	enum{
		SIZE_OF_MAX_PES_BUFFER = 512*1024,
		SIZE_OF_MAX_PAT_BUFFER = 1024,
		SIZE_OF_MAX_PMT_BUFFER = 1024,
		SIZE_OF_MAX_SDT_BUFFER = 1024
	};

	CTinyTSMuxer(int nStreamID = DEFAULT_TRANSPORT_STREAM_ID);
	virtual ~CTinyTSMuxer(void);
	typedef int (*PacketsDeliverer_cb)(const unsigned char* data, const unsigned long dataLength,
		unsigned int uTSType, const u64 millSec, void*vptr);

	typedef int (*AfterTSPacket_cb)(const unsigned char* data, const unsigned long dataLength,
		unsigned int uTSType, const u64 millSec, void*vptr);

	void SetPacketsDeliverer(PacketsDeliverer_cb func, void* vptr)
	{
		m_devliverFunc = func;
		m_devliverFuncParam = vptr;		
	};

	void SetAfterPacket(AfterTSPacket_cb func, void* vptr)
	{
		m_afterTSPacketFunc  = func;
		m_afterTSPacketParam = vptr;
	};

	CTinyPMTPacket *NewProgram(int nPid, int nPcrPid);
	CTinyPESPacket *NewStream(int nStreamID, int nPid, u32 uStreamType);
	CSDTPacket *NewSDT(const char *szProviderName, const char *szServiceName);

	void AddProgram(CTinyPMTPacket* program);
	void AddStream(CTinyPMTPacket* program, CTinyPESPacket *pes);

	int Deliver(u32 uTSType, string &buff, int nBuffLen, CTinyTSPacket *ts, u8 adaption, u8 *data, 
		int dataLength, u64 pcr, u64 pts, u64 dts);
	int DeliverPAT();
	int DeliverPMT();
	int DeliverSDT();
	int Mux(u32 uFrameType, CTinyPESPacket *pes, int pcrPid,
		u8 *data, int dataLength, u64 pcr, u64 pts, u64 dts);
	
private:
	PacketsDeliverer_cb m_devliverFunc;
	void*               m_devliverFuncParam;

	AfterTSPacket_cb    m_afterTSPacketFunc;
	void*				m_afterTSPacketParam;
	map<int, CTinyTSPacket *> m_tsPIDMap;

	CTinyPATPacket *m_pat;
	CSDTPacket *m_sdt;
	string m_pesBuffer;
	string m_pmtBuff;
	string m_patBuff;
	string m_sdtBuff;

	vector<CTinyPMTPacket*> m_pmtPackets;
	vector<CTinyPESPacket*> m_pesPackets;
};

#endif