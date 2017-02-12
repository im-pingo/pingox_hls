#include <string.h>
#include <malloc.h> 
#include "pgx_pes_packet.h"


CTinyPESPacket::CTinyPESPacket(int nStreamID, int nPID, u32 uStreamType)
{
	m_pesBuff = NULL;

	m_pes.Init();
	m_pes.stream_id = nStreamID;

	this->stream_type = uStreamType;
	this->elementary_PID = nPID;

	InitPacket();
}

CTinyPESPacket::~CTinyPESPacket()
{
	if (m_pesBuff != NULL)
	{
		free(m_pesBuff);
		m_pesBuff = NULL;
	}
}

void CTinyPESPacket::InitPacket()
{
	if (m_pesBuff == NULL)
	{
		m_pesBuff = (u8*)malloc(SIZE_OF_DEFAULT_PES_PACKET);
		m_nBuffLength = SIZE_OF_DEFAULT_PES_PACKET;
	}
	m_pes.Init();
	m_nBuffOffset = 0;
	
	memset(m_pesBuff, 0, m_nBuffLength);
}

int CTinyPESPacket::FillPESPacket(u32 frameType, u8 const *data, 
	int nDataLength, u64 uPTS, u64 uDTS)
{
	InitPacket();
	
	int nPESLength = 0;

	m_pes.pes_header_length = 5;


	m_pes.pes_optional_flags |= pes_header_t::PES_OPT_PTS_DTS_FLAGS_PTS_10;//pts

	if (uDTS != 0xffffffffffffffff)
	{
		m_pes.pes_header_length += 5;
		m_pes.pes_optional_flags |= pes_header_t::PES_OPT_PTS_DTS_FLAGS_DTS_01;//dts
	}

	if (frameType&FRAME_TYPE_VIDEO)
	{
		nPESLength += 9+m_pes.pes_header_length+nDataLength+SIZE_OF_AUD_LENGTH;
	}
	else
	{
		nPESLength += 9+m_pes.pes_header_length+nDataLength;
	}
	
	if (nPESLength>m_nBuffLength)
	{
		m_pesBuff = (u8*)realloc(m_pesBuff, nPESLength);
		m_pes.pes_length = 0;
		m_nBuffLength = nPESLength;
	}

	if (nPESLength-6>SIZE_OF_DEFAULT_PES_PACKET)
	{
		m_pes.pes_length = 0;
	}
	else
	{
		m_pes.pes_length = nPESLength-6;
	}
	
	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.prefix[0]);
	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.prefix[1]);
	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.prefix[2]);

	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.stream_id);

	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.pes_length>>8);
	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.pes_length);
	
	AppendBits(m_pesBuff, m_nBuffOffset, 2, m_pes.reserved_2);
	
	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.pes_optional_flags>>6);
	AppendBits(m_pesBuff, m_nBuffOffset, 6, m_pes.pes_optional_flags);

	AppendBits(m_pesBuff, m_nBuffOffset, 8, m_pes.pes_header_length);


	u8 startPTS = 0x00;
	if (m_pes.pes_optional_flags & pes_header_t::PES_OPT_PTS_DTS_FLAGS_PTS_10)
	{
		startPTS = 0x02;
	}

	if (m_pes.pes_optional_flags & pes_header_t::PES_OPT_PTS_DTS_FLAGS_DTS_01)
	{
		startPTS |= 0x01;
	}

	AppendBits(m_pesBuff, m_nBuffOffset, 4, startPTS);
	AppendBits(m_pesBuff, m_nBuffOffset, 3, uPTS>>30);
	AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);
	AppendBits(m_pesBuff, m_nBuffOffset, 8, uPTS>>22);
	AppendBits(m_pesBuff, m_nBuffOffset, 7, uPTS>>15);
	AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);
	AppendBits(m_pesBuff, m_nBuffOffset, 8, uPTS>>7);
	AppendBits(m_pesBuff, m_nBuffOffset, 7, uPTS);
	AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);

	if (uDTS != 0xffffffffffffffff)
	{
		AppendBits(m_pesBuff, m_nBuffOffset, 4, 0x01);
		AppendBits(m_pesBuff, m_nBuffOffset, 3, uDTS>>30);
		AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);
		AppendBits(m_pesBuff, m_nBuffOffset, 8, uDTS>>22);
		AppendBits(m_pesBuff, m_nBuffOffset, 7, uDTS>>15);
		AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);
		AppendBits(m_pesBuff, m_nBuffOffset, 8, uDTS>>7);
		AppendBits(m_pesBuff, m_nBuffOffset, 7, uDTS);
		AppendBits(m_pesBuff, m_nBuffOffset, 1, 0x01);
	}

	if (frameType&FRAME_TYPE_VIDEO)
	{
		memcpy(m_pesBuff + (m_nBuffOffset/8), g_AUD, SIZE_OF_AUD_LENGTH);
		m_nBuffOffset += SIZE_OF_AUD_LENGTH*8;
	}

	memcpy(m_pesBuff + (m_nBuffOffset/8), data, nDataLength);
	m_nBuffOffset += nDataLength*8;

	return nPESLength;
}

u8 *CTinyPESPacket::GetPESPacket()
{
	return m_pesBuff;
}
