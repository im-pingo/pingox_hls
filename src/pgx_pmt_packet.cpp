#include <string.h>
#include "pgx_pmt_packet.h"

CTinyPMTPacket::CTinyPMTPacket(int pid, int pcrPID)
{
	m_pmt.Init();

	m_pid = pid;
	m_pmt.PCR_PID = pcrPID;
	this->program_pid = pid;
	this->program_number = this->PROGRAM_NUM_PMT; 

	InitPacket();
	
}

CTinyPMTPacket::~CTinyPMTPacket()
{

}

void CTinyPMTPacket::InitPacket()
{
	memset(m_pmtBuff, 0x00, SIZE_OF_MAX_PMT_PACKET);
	m_nBuffOffset = 0;
	m_pmt.Init();
}

void CTinyPMTPacket::AddStream(ts_pmt_stream_t *stream)
{
	m_pmt.PMT_Stream.push_back(stream);
}

void CTinyPMTPacket::CleanStream()
{
	m_pmt.PMT_Stream.clear();
}

int CTinyPMTPacket::FillPMTPacket()
{
	InitPacket();
	vector<ts_pmt_stream_t*> vecStreams = m_pmt.PMT_Stream;
	for (int m=0; m<vecStreams.size(); m++)
	{
		m_pmt.section_length += 5+vecStreams[m]->descriptor.size();
	}
	m_pmt.section_length += m_pmt.descriptor.size()+13;

	AppendBits(m_pmtBuff, m_nBuffOffset, 8, 0x00);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.table_id);
	AppendBits(m_pmtBuff, m_nBuffOffset, 1, m_pmt.section_syntax_indicator);
	AppendBits(m_pmtBuff, m_nBuffOffset, 1, m_pmt.zero);
	AppendBits(m_pmtBuff, m_nBuffOffset, 2, m_pmt.reserved_1);

	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.section_length>>4);
	AppendBits(m_pmtBuff, m_nBuffOffset, 4, m_pmt.section_length);
	
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.program_number>>8);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.program_number);
	
	AppendBits(m_pmtBuff, m_nBuffOffset, 2, m_pmt.reserved_2);
	AppendBits(m_pmtBuff, m_nBuffOffset, 5, m_pmt.version_number);
	AppendBits(m_pmtBuff, m_nBuffOffset, 1, m_pmt.current_next_indicator);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.section_number);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.last_section_number);
	AppendBits(m_pmtBuff, m_nBuffOffset, 3, m_pmt.reserved_3);

	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.PCR_PID>>5);
	AppendBits(m_pmtBuff, m_nBuffOffset, 5, m_pmt.PCR_PID);
	
	AppendBits(m_pmtBuff, m_nBuffOffset, 4, m_pmt.reserved_4);

	if (m_pmt.descriptor.size() != m_pmt.program_info_length)
	{
		m_pmt.program_info_length = m_pmt.descriptor.size();
	}

	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.program_info_length>>4);
	AppendBits(m_pmtBuff, m_nBuffOffset, 4, m_pmt.program_info_length);

	for (int i = 0; i<m_pmt.program_info_length; ++i)
	{
		AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.descriptor[i]);
	}

	for (int j = 0 ; j < m_pmt.PMT_Stream.size(); ++j)
	{
		ts_pmt_stream_t *tmpStream = m_pmt.PMT_Stream[j];
		AppendBits(m_pmtBuff, m_nBuffOffset, 8, tmpStream->stream_type);
		AppendBits(m_pmtBuff, m_nBuffOffset, 3, tmpStream->reserved_3);

		AppendBits(m_pmtBuff, m_nBuffOffset, 8, tmpStream->elementary_PID>>5);
		AppendBits(m_pmtBuff, m_nBuffOffset, 5, tmpStream->elementary_PID);
		
		AppendBits(m_pmtBuff, m_nBuffOffset, 4, tmpStream->reserved_4);
		
		if (tmpStream->ES_info_length!=tmpStream->descriptor.size())
		{
			tmpStream->ES_info_length = tmpStream->descriptor.size();
		}

		AppendBits(m_pmtBuff, m_nBuffOffset, 8, tmpStream->ES_info_length>>4);
		AppendBits(m_pmtBuff, m_nBuffOffset, 4, tmpStream->ES_info_length);

		for (int cnt = 0; cnt<tmpStream->descriptor.size(); cnt++)
		{
			AppendBits(m_pmtBuff, m_nBuffOffset, 8, tmpStream->descriptor[cnt]);
		}
		
	}

	m_pmt.CRC_32 = CRC32(m_pmtBuff+1, (m_nBuffOffset/8)-1);

	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.CRC_32>>24);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.CRC_32>>16);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.CRC_32>>8);
	AppendBits(m_pmtBuff, m_nBuffOffset, 8, m_pmt.CRC_32);

	memset(m_pmtBuff+(m_nBuffOffset/8), 0xff, SIZE_OF_MAX_PMT_PACKET-(m_nBuffOffset/8));

	return SIZE_OF_MAX_PMT_PACKET/*m_nBuffOffset/8*/;
}

u8* CTinyPMTPacket::GetPMTBuff()
{
	return m_pmtBuff;
}
