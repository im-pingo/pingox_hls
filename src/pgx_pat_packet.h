 #ifndef _PAT_PACKET_H
#define _PAT_PACKET_H
#include "pgx_ts_common.h"
#include "pgx_ts_common_def.h"

class LIB_TINY_TS_API CTinyPATPacket
{
public:
	enum
	{
		SIZE_OF_MAX_PAT_PACKET = 184,
	};

	CTinyPATPacket(int streamID = 0x01);
	virtual ~CTinyPATPacket();

	void InitPAT();
	int  FillPATPacket();
	u8 *GetPATBuff();
	void AddProgram(ts_pat_program_t *program);
	int GetStreamID(){return m_pat.transport_stream_id;};
	int GetProgramPMT(vector<ts_pat_program_t *> &pmtVec);

private:
	ts_pat_t m_pat;
	u8 m_patBuff[SIZE_OF_MAX_PAT_PACKET];
	int m_nBuffOffset;
	int m_streamID;
};

#endif