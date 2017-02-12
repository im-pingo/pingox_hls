#include <string.h>
#include "pgx_ts_common.h"
#include "pgx_ts_packet.h"

CTinyTSPacket::CTinyTSPacket(int pid)
{
	SetPid(pid);
	return;
} 

CTinyTSPacket::~CTinyTSPacket()
{
	return;
}

void CTinyTSPacket::SetPid(int pid)
{
	m_pid = pid;
}

void CTinyTSPacket::InitTSHeader()
{
	m_ts.Init();
	m_ts.pid = m_pid;
	m_adaption.Init();
	memset(m_tsBuff, 0, 188);
	m_nTSBufOffset = 0;	
}

int CTinyTSPacket::FillTSPacket(u8 *dataBuff, int nBufflen,
							int startIndicator, u8 adaption, u64 timeStamp)
{
	InitTSHeader();

	//计算负载buffer的长度
	int nPayloadLen = SIZE_OF_TS_PACKET - (SIZE_OF_TS_HEADER/8);
	int nAdaptionLen = 0;
	u8  adaptionFieldControl = 0x00;
	u8  adaptionFlags = ts_adaption_t::TS_ADAPTION_ZERO;

	if (nBufflen>0)
	{
		adaptionFieldControl |= 0x01; //有负载数据
	}

	u8 adaptionBuff[SIZE_OF_TS_PACKET-(SIZE_OF_TS_HEADER/8)] = {0};
	memset(adaptionBuff, 0xff, sizeof(adaptionBuff));
	int nAdaptionOffset = 0;

	for (int i = 0; i<8; i++)
	{

		switch(adaption&(0x01<<i))
		{
		case ts_adaption_t::TS_ADAPTION_FIELD_EXTENSION_FLAG_1 :
			break;

		case ts_adaption_t::TS_ADAPTION_TRANSPORT_PRIVATE_DATA_FLAG_1 :
			break;

		case ts_adaption_t::TS_ADAPTION_SPLICING_POINT_FLAG_1 :
			break;

		case ts_adaption_t::TS_ADAPTION_OPCR_FLAG_1: //OPCR			
			break;

		case ts_adaption_t::TS_ADAPTION_PCR_FLAG_1: //PCR
			nAdaptionLen = (SIZE_OF_PCR)/8;
			adaptionFlags |= ts_adaption_t::TS_ADAPTION_PCR_FLAG_1;
			adaptionFieldControl |= 0x02;//有适应字段
			
			memset(adaptionBuff+nAdaptionOffset, 0, 6); // PCR占用了6bytes， 这里进行初始化
			SetPCR(adaptionBuff+nAdaptionOffset, nAdaptionOffset, timeStamp);

			break;

		case ts_adaption_t::TS_ADAPTION_ELEMENTARY_STREAM_PRIORITY_INDICATOR_1 :
			break;

		case ts_adaption_t::TS_ADAPTION_RANDOM_ACCESS_INDICATOR_1 :
			adaptionFlags |= ts_adaption_t::TS_ADAPTION_RANDOM_ACCESS_INDICATOR_1;
			break;

		case ts_adaption_t::TS_ADAPTION_DISCONTINUITY_INDICATOR_1 :
			break;

		}
	}

	if ((adaptionFieldControl & 0x02) == 0x02)
	{
		nAdaptionLen += SIZE_OF_ADAPTION_FIELD_FLAGS/8;
		nPayloadLen  -= nAdaptionLen+SIZE_OF_ADAPTION_LENGTH/8;
	}
	else
	{
		if (nPayloadLen>nBufflen)
		{
			adaptionFieldControl |= 0x02;
			
			nAdaptionLen += SIZE_OF_ADAPTION_FIELD_FLAGS/8;
			nPayloadLen  -= nAdaptionLen + SIZE_OF_ADAPTION_LENGTH/8;			
		}
		
	}
	
	if (nPayloadLen>nBufflen)
	{
		nAdaptionLen += nPayloadLen-nBufflen;
		nPayloadLen  -= nPayloadLen-nBufflen;
	}

	m_adaption.adaption_len   = nAdaptionLen;
	m_adaption.adaption_flags = adaptionFlags;
	m_adaption.adaption_data  = adaptionBuff;

	m_ts.adaption_field_control = adaptionFieldControl;
	m_ts.payload_data           = dataBuff;
	m_ts.payload_len            = nPayloadLen;
	m_ts.payload_unit_start_indicator = startIndicator;
//	printf("start = %d .\n", m_ts.payload_unit_start_indicator);
	SetTSHeader();
	SetAdaption();
	SetTSPayload();

	if (m_ts.adaption_field_control&0x01 ||
		m_adaption.adaption_flags&ts_adaption_t::TS_ADAPTION_DISCONTINUITY_INDICATOR_1)
	{
		m_ts.ContinuityCounterACC();
	}
	
	return nPayloadLen;
}

void CTinyTSPacket::SetPCR(u8 *buff, int &offset, u64 timeStamp)
{
	memset(&m_pcr, 0 , sizeof(m_pcr));
	
	m_pcr.program_clock_reference_base = timeStamp;
	m_pcr.program_clock_reference_extension = timeStamp*300%300;
	m_pcr.reserved = 0x3f;

	AppendBits(buff, offset, 8, m_pcr.program_clock_reference_base>>25);
	AppendBits(buff, offset, 8, m_pcr.program_clock_reference_base>>17);
	AppendBits(buff, offset, 8, m_pcr.program_clock_reference_base>>9);
	AppendBits(buff, offset, 8, m_pcr.program_clock_reference_base>>1);
	AppendBits(buff, offset, 1, m_pcr.program_clock_reference_base);

	AppendBits(buff, offset, 6, m_pcr.reserved);

	AppendBits(buff, offset, 8, m_pcr.program_clock_reference_extension>>1);
	AppendBits(buff, offset, 1, m_pcr.program_clock_reference_extension);
}

void CTinyTSPacket::SetTSHeader()
{
	AppendBits(m_tsBuff, m_nTSBufOffset, 8, m_ts.sync_byte);	
	AppendBits(m_tsBuff, m_nTSBufOffset, 1, m_ts.transport_error_indicator);	

	AppendBits(m_tsBuff, m_nTSBufOffset, 1, m_ts.payload_unit_start_indicator);

	AppendBits(m_tsBuff, m_nTSBufOffset, 1, m_ts.transport_priority);	
	
	AppendBits(m_tsBuff, m_nTSBufOffset, 1, m_ts.pid>>12);
	AppendBits(m_tsBuff, m_nTSBufOffset, 8, m_ts.pid>>4);
	AppendBits(m_tsBuff, m_nTSBufOffset, 4, m_ts.pid);

	AppendBits(m_tsBuff, m_nTSBufOffset, 2, m_ts.transport_scrambling_control);
	AppendBits(m_tsBuff, m_nTSBufOffset, 2, m_ts.adaption_field_control);

	AppendBits(m_tsBuff, m_nTSBufOffset, 4, m_ts.continuity_counter );	

}

void CTinyTSPacket::SetAdaption()
{
	if(m_adaption.adaption_len)
	{
		AppendBits(m_tsBuff, m_nTSBufOffset, 8, m_adaption.adaption_len);
		AppendBits(m_tsBuff, m_nTSBufOffset, 8, m_adaption.adaption_flags);
		for (int i=0; i< m_adaption.adaption_len-1; ++i)
		{
			AppendBits(m_tsBuff, m_nTSBufOffset, 8, m_adaption.adaption_data[i]);
		}
	}
}

void CTinyTSPacket::SetTSPayload()
{
	memcpy(m_tsBuff+(m_nTSBufOffset/8), m_ts.payload_data, m_ts.payload_len);
}

u8 *CTinyTSPacket::GetTSBuff()
{
	return m_tsBuff;
}
