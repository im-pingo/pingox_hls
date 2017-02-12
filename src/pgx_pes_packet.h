 #ifndef _PES_PACKET_H
#define _PES_PACKET_H
#include "pgx_ts_common.h"
#include "pgx_ts_common_def.h"

class LIB_TINY_TS_API CTinyPESPacket:public ts_pmt_stream_t
{
public:
	enum
	{
		SIZE_OF_DEFAULT_PES_PACKET = 65535,

		FRAME_TYPE_VIDEO = 0X10000000,
		FRAME_TYPE_AUDIO = 0X20000000,
	};

	CTinyPESPacket(int nStreamID, int nPID, u32 uStreamType);
	virtual ~CTinyPESPacket();

	virtual void InitPacket();
	virtual int FillPESPacket(u32 frameType, u8 const *data, int nDataLength, u64 uPTS, u64 uDTS = 0xffffffffffffffff);
	virtual u8 *GetPESPacket();
	virtual int GetPID(){return this->elementary_PID;};
	virtual int GetStreamType(){return this->stream_type;};

	virtual int FillPESPacket(u8 const *data, int nDataLength, u64 uPTS, u64 uDTS  = 0xffffffffffffffff ) = 0;

private:
	pes_header_t m_pes;
	u8 *m_pesBuff;
	int m_nBuffLength;
	int m_nBuffOffset;

};

class AACPESPacket:public CTinyPESPacket
{
public:
	AACPESPacket(int nStreamID, int nPID):CTinyPESPacket(nStreamID, nPID, STREAM_TYPE_AAC){};
	int FillPESPacket(u8 const *data, int nDataLength, u64 uPTS, u64 uDTS  = 0xffffffffffffffff )
	{
		return CTinyPESPacket::FillPESPacket(FRAME_TYPE_AUDIO, data, nDataLength, uPTS, uDTS);
	};
};

class H264PESPacket:public CTinyPESPacket
{
public:
	H264PESPacket(int nStreamID, int nPID):CTinyPESPacket(nStreamID, nPID, STREAM_TYPE_H264){};
	int FillPESPacket(u8 const *data, int nDataLength, u64 uPTS, u64 uDTS  = 0xffffffffffffffff )
	{
		return CTinyPESPacket::FillPESPacket(FRAME_TYPE_VIDEO, data, nDataLength, uPTS, uDTS);
	};
};

class H265PESPacket :public CTinyPESPacket
{
public:
	H265PESPacket(int nStreamID, int nPID) :CTinyPESPacket(nStreamID, nPID, STREAM_TYPE_H265) {};
	int FillPESPacket(u8 const *data, int nDataLength, u64 uPTS, u64 uDTS = 0xffffffffffffffff)
	{
		return CTinyPESPacket::FillPESPacket(FRAME_TYPE_VIDEO, data, nDataLength, uPTS, uDTS);
	}
};

#endif