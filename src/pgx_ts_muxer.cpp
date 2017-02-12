#include <string.h>
#include "pgx_ts_muxer.h"


CTinyTSMuxer::CTinyTSMuxer(int nStreamID):m_devliverFunc(NULL),m_afterTSPacketFunc(NULL)
{
	m_pat = new CTinyPATPacket(nStreamID);
	m_sdt = new CSDTPacket();
	m_tsPIDMap[0] = new CTinyTSPacket(0);

	m_pesBuffer.clear();
	m_patBuff.clear();
	m_pmtBuff.clear();
	m_sdtBuff.clear();
}


CTinyTSMuxer::~CTinyTSMuxer(void)
{
 	if (m_pat != NULL)
 	{
 		delete m_pat;
 	}

	if (m_sdt!=NULL)
	{
		delete m_sdt;
	}
 
 	for (map<int, CTinyTSPacket*>::iterator it=m_tsPIDMap.begin(); it!=m_tsPIDMap.end(); ++it)
 	{
 		if (it->second != NULL)
 		{
 			delete it->second;
 			it->second = NULL;
 		}		
 	}

	for (int i = 0; i<m_pmtPackets.size(); ++i)
	{
		CTinyPMTPacket *&pPMT = m_pmtPackets[i];
		if (pPMT!=NULL)
		{
			delete pPMT;
			pPMT = NULL;
		}
	}

	for (int i = 0; i<m_pesPackets.size(); ++i)
	{
		CTinyPESPacket *&pPES = m_pesPackets[i];
		if (pPES!=NULL)
		{
			delete pPES;
			pPES = NULL;
		}
	}
}

 
CTinyPMTPacket* CTinyTSMuxer::NewProgram(int nPid, int nPcrPid)
{
	m_tsPIDMap[nPid] = new CTinyTSPacket(nPid);

	CTinyPMTPacket *pmt =  new CTinyPMTPacket(nPid, nPcrPid);
	m_pmtPackets.push_back(pmt);

	return pmt;
}

CTinyPESPacket *CTinyTSMuxer::NewStream(int nStreamID, int nPid, u32 uStreamType)
{
	CTinyPESPacket *pes = NULL;
	
	switch (uStreamType)
	{
	case ts_pmt_stream_t::STREAM_TYPE_H264:
		pes = new H264PESPacket(nStreamID,nPid);
		break;
	case ts_pmt_stream_t::STREAM_TYPE_H265:
		pes = new H265PESPacket(nStreamID, nPid);
		break;
	case ts_pmt_stream_t::STREAM_TYPE_AAC:
		pes = new AACPESPacket(nStreamID, nPid);
		break;
	default:
		pes = NULL;
	}

	if (nPid != NULL)
	{
		m_tsPIDMap[nPid] = new CTinyTSPacket(nPid);
	}

	m_pesPackets.push_back(pes);
	return pes;
}

void CTinyTSMuxer::AddProgram(CTinyPMTPacket* program)
{
	m_pat->AddProgram(program);
}

int CTinyTSMuxer::DeliverSDT()
{
	int nSDTLength = m_sdt->FillPATPacket();
	u8 *data = m_sdt->GetSDTBuff();

	CTinyTSPacket *ts = m_tsPIDMap[m_sdt->GetSDTPid()];
	if (ts == NULL)
	{
		return -2;
	}

	return Deliver(TS_TYPE_SDT, m_sdtBuff, SIZE_OF_MAX_SDT_BUFFER, ts, 0, data, nSDTLength, 0, 0, 0);
}

CSDTPacket* CTinyTSMuxer::NewSDT(const char *szProviderName, const char *szServiceName)
{
	int nPid = m_sdt->GetSDTPid();
	m_sdt->SetDescriptor(szProviderName, szServiceName);
	m_tsPIDMap[nPid] = new CTinyTSPacket(nPid);

	return m_sdt;
}

int CTinyTSMuxer::DeliverPAT()
{
	if (m_tsPIDMap.count(0) <= 0)
	{
		return -1;
	}

	CTinyTSPacket *ts = m_tsPIDMap[0];
	if (ts == NULL)
	{
		return -2;
	}

	int nPatLength = m_pat->FillPATPacket();
	u8 *data = m_pat->GetPATBuff();

	return Deliver(TS_TYPE_PAT, m_patBuff, SIZE_OF_MAX_PAT_BUFFER, ts, 0, data, nPatLength, 0, 0, 0);
}

int CTinyTSMuxer::DeliverPMT()
{
	vector<ts_pat_program_t *> pmtVec;

	m_pat->GetProgramPMT(pmtVec);

	for (int i=0; i<pmtVec.size(); ++i)
	{
		CTinyPMTPacket *pmt = (CTinyPMTPacket*)(pmtVec[i]);
		int pid = pmt->GetPMTPID();
		if (m_tsPIDMap.count(pid) <= 0)
		{
			continue;
		}

		CTinyTSPacket *ts = m_tsPIDMap[pid];
		if (ts == NULL)
		{
			continue;
		}

		int nRet = pmt->FillPMTPacket();
		u8 *data = pmt->GetPMTBuff();
		Deliver(TS_TYPE_PMT, m_pmtBuff, SIZE_OF_MAX_PMT_BUFFER, ts, 0, data, nRet, 0, 0, 0);
	}

	return 0;
}

int CTinyTSMuxer::Deliver(u32 uTSType, string &buff, int nBuffLen, CTinyTSPacket *ts, u8 adaption, u8 *data, 
	int dataLength, u64 pcr, u64 pts, u64 dts)
{
	int nOffset = 0;
	int nRet = ts->FillTSPacket(data, dataLength, 1, adaption, pcr);
	int nTSCount = 0;

	buff.clear();

	buff.append((const char*)(ts->GetTSBuff()), CTinyTSPacket::SIZE_OF_TS_PACKET);
	
	++nTSCount;
	uTSType |= nTSCount;

	nOffset += nRet;
	if (m_devliverFunc != NULL)
	{
		m_devliverFunc(ts->GetTSBuff(), CTinyTSPacket::SIZE_OF_TS_PACKET, uTSType, pts/90,  m_devliverFuncParam);
	}

	adaption = _ts_adaption_t::TS_ADAPTION_ZERO;
	while(nOffset < dataLength)
	{
		nRet = ts->FillTSPacket(data+nOffset, dataLength-nOffset, 0, adaption);
		nOffset += nRet;

		buff.append((const char*)(ts->GetTSBuff()), CTinyTSPacket::SIZE_OF_TS_PACKET);

		++nTSCount;
		uTSType |= nTSCount;

		if (m_devliverFunc != NULL)
		{
			m_devliverFunc(ts->GetTSBuff(), CTinyTSPacket::SIZE_OF_TS_PACKET, uTSType, pts/90, m_devliverFuncParam);
		}
	}

	if (m_afterTSPacketFunc)
	{
		m_afterTSPacketFunc((const unsigned char*)(buff.data()), buff.size(), uTSType, pts/90, m_afterTSPacketParam);
	}

	return 0;
}

int CTinyTSMuxer::Mux(u32 uFrameType, CTinyPESPacket *pes, int pcrPid, u8 *data, 
					int dataLength, u64 pcr, u64 pts, u64 dts)
{
	int nPesLength = pes->FillPESPacket(data, dataLength, pts, dts);
	u8 *pesData = pes->GetPESPacket();
	int pid = pes->GetPID();
	if (m_tsPIDMap.count(pid) <= 0)
	{
		return -1;
	}

	CTinyTSPacket *ts = m_tsPIDMap[pid];
	if (ts == NULL)
	{
		return -2;
	}

	u8 adaption = _ts_adaption_t::TS_ADAPTION_ZERO;
	if (pcrPid == pes->GetPID())
	{
		adaption |= _ts_adaption_t::TS_ADAPTION_PCR_FLAG_1;
		adaption |= _ts_adaption_t::TS_ADAPTION_RANDOM_ACCESS_INDICATOR_1;
	}

	return Deliver(uFrameType, m_pesBuffer, SIZE_OF_MAX_PES_BUFFER, ts, adaption, pesData, nPesLength, pcr, pts, dts);
}
