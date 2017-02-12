#include <string.h>
#include <malloc.h>
#include <memory.h> 
#include "pgx_pat_packet.h"

CTinyPATPacket::CTinyPATPacket(int streamID)
{
	InitPAT();

	m_pat.Init();
	m_pat.transport_stream_id = streamID;
}

CTinyPATPacket::~CTinyPATPacket()
{

}

void CTinyPATPacket::InitPAT()
{
	memset(m_patBuff, 0x00, SIZE_OF_MAX_PAT_PACKET);
	m_nBuffOffset = 0;
	m_pat.Init();
}

void CTinyPATPacket::AddProgram(ts_pat_program_t *program)
{
	m_pat.program.push_back(program);
}

int CTinyPATPacket::FillPATPacket()
{
	InitPAT();
	vector<ts_pat_program_t*> vecPatProgram = m_pat.program ;	
	
	int nPatProgram = 4*vecPatProgram.size();
	m_pat.section_length = nPatProgram+9;

	AppendBits(m_patBuff, m_nBuffOffset, 8, 0x00);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.table_id);
	AppendBits(m_patBuff, m_nBuffOffset, 1, m_pat.section_syntax_indicator);
	AppendBits(m_patBuff, m_nBuffOffset, 1, m_pat.zero);
	AppendBits(m_patBuff, m_nBuffOffset, 2, m_pat.reserved_1);

	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.section_length>>4);
	AppendBits(m_patBuff, m_nBuffOffset, 4, m_pat.section_length);
	
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.transport_stream_id>>8);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.transport_stream_id);
	
	AppendBits(m_patBuff, m_nBuffOffset, 2, m_pat.reserved_2);
	AppendBits(m_patBuff, m_nBuffOffset, 5, m_pat.version_number);
	AppendBits(m_patBuff, m_nBuffOffset, 1, m_pat.current_next_indicator);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.section_number);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.last_section_number);

	for (int i=0; i< vecPatProgram.size(); ++i)
	{
		AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.program[i]->program_number>>8);
		AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.program[i]->program_number);

		AppendBits(m_patBuff, m_nBuffOffset, 3, m_pat.program[i]->reserved);

		AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.program[i]->program_pid>>5);
		AppendBits(m_patBuff, m_nBuffOffset, 5, m_pat.program[i]->program_pid);
	}

	m_pat.CRC_32 = CRC32(m_patBuff+1, (m_nBuffOffset/8)-1);
	
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.CRC_32>>24);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.CRC_32>>16);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.CRC_32>>8);
	AppendBits(m_patBuff, m_nBuffOffset, 8, m_pat.CRC_32);

	memset(m_patBuff+(m_nBuffOffset/8), 0xff, SIZE_OF_MAX_PAT_PACKET-(m_nBuffOffset/8));
	
	return /*m_nBuffOffset/8;*/SIZE_OF_MAX_PAT_PACKET;
}

u8 *CTinyPATPacket::GetPATBuff()
{
	return m_patBuff;
}


int CTinyPATPacket::GetProgramPMT(vector<ts_pat_program_t *> &pmtVec)
{
	int i = 0;
	for(i = 0; i < m_pat.program.size(); ++i)
	{
		if (m_pat.program[i]->program_number == ts_pat_program_t::PROGRAM_NUM_PMT)
		{
			pmtVec.push_back(m_pat.program[i]);
		}
	}

	return pmtVec.size();
}


