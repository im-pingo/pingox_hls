//#include "../libLog/PrintLog.h"
#include "pgx_ts_packer.h"

CTSPacker::CTSPacker(u32 videoStreamType, u32 audioStreamType)
{
	m_AVFlags  = STREAM_ZERO_FLAG;

	m_PMTPID   = TS_PARAM_PID_PMT;
	m_VideoPID = TS_PARAM_PID_VIDEO;
	m_AudioPID = TS_PARAM_PID_AUDIO;
	m_PCRPID   = TS_PARAM_PID_PCR;
	m_VideoStreamID = STREAM_ID_VIDEO;
	m_AudioStreamID = STREAMID_AUDIO;

	m_Muxer = new CTinyTSMuxer();
	//m_Muxer->NewSDT("cczjp89@hotmail.com", "PingoX");
	m_Program = m_Muxer->NewProgram(m_PMTPID, m_PCRPID);
	m_StreamVideo = (H264PESPacket *)m_Muxer->NewStream(m_VideoStreamID, m_VideoPID, videoStreamType);
	m_StreamAudio = (AACPESPacket *)m_Muxer->NewStream(m_AudioStreamID, m_AudioPID, audioStreamType);

	m_Muxer->AddProgram(m_Program);
	m_Muxer->SetAfterPacket(RealPacketsDeliverer, this);

	m_ulLastAudioPTS = 0;
	m_ulLastVideoPTS = 0;
}

CTSPacker::~CTSPacker(void)
{
	if (m_Muxer)
	{
		delete m_Muxer;
		m_Muxer = NULL;
	}
}

int CTSPacker::DeliverSDT()
{
	return m_Muxer->DeliverSDT();
}

int CTSPacker::DeliverPAT()
{
	return m_Muxer->DeliverPAT();
}

int CTSPacker::DeliverPMT()
{
	return m_Muxer->DeliverPMT();
}

int CTSPacker::RealPacketsDeliverer(const unsigned char* data, const unsigned long size,
	unsigned int uTSType, const u64 millSec, void* streaming)
{
	CTSPacker *thiz = (CTSPacker *)streaming;
//	int nTSCount = uTSType&0x0000ffff;
	
	int nRet = 0;

	switch (uTSType&CTinyTSMuxer::TS_TYPE_PES)
	{
	case CTinyTSMuxer::TS_TYPE_PES_VIDEO_I_FRAME:
		nRet = thiz->AfterPacketIFrame(data, size, millSec);
		break;

	case CTinyTSMuxer::TS_TYPE_PES_VIDEO_P_FRAME:
		//	printlog("CHLSHelper::RealPacketsDeliverer(%d) => ts count = %d , datalen=%d.\n", thiz->GetCurrtentCapacity(), (int)(uTSType&0xff), size);
		nRet = thiz->AfterPacketPFrame(data, size, millSec);
		break;

	case CTinyTSMuxer::TS_TYPE_PES_AUDIO:
		nRet = thiz->AfterPacketAudioFrame(data, size, millSec);
		break;

	case CTinyTSMuxer::TS_TYPE_PES_VIDEO_E_FRAME:
		nRet = thiz->AfterPacketEmptyFrame(data, size, millSec);
		break;

	}

	switch (uTSType&0xffff0000)
	{
	case CTinyTSMuxer::TS_TYPE_PAT:
		nRet = thiz->AfterPacketPAT(data, size);
		break;

	case CTinyTSMuxer::TS_TYPE_PMT:
		nRet = thiz->AfterPacketPMT(data, size);
		break;

	case CTinyTSMuxer::TS_TYPE_SDT:
		nRet = thiz->AfterPacketSDT(data, size);
		break;
	}

	return nRet;
}

int CTSPacker::VideoMux(unsigned int uiFrameType, u8 *data, int dataLength, u64 pcr, u64 pts, u64 dts)
{
	if(CTinyTSMuxer::TS_TYPE_PES_VIDEO_I_FRAME != uiFrameType && 
		CTinyTSMuxer::TS_TYPE_PES_VIDEO_P_FRAME != uiFrameType)
	{
// 		PrintSysLog("CTSPacker::VideoMux(%s) => Error FrameType (0x%08x) .\n",
// 			m_strMediaName.c_str(), uiFrameType);

		return -1;
	}

	this->EnsureVideoStream();

	int nRet = m_Muxer->Mux(uiFrameType, m_StreamVideo, m_PCRPID, data, dataLength, pcr, pts, dts);

	if (nRet < 0)
	{
// 		PrintSysLog("CTSPacker::VideoMux(%s) => Error Mux() .\n",
// 			m_strMediaName.c_str());

		return -1;
	}

	return 0;
}

int CTSPacker::AudioMux(u8 *data, int dataLength, /*u64 pcr, */u64 pts, u64 dts/*=~0*/)
{
	EnsureAudioStream();

	int nRet = m_Muxer->Mux(CTinyTSMuxer::TS_TYPE_PES_AUDIO,
		m_StreamAudio, m_PCRPID, data, dataLength, 0, pts, dts);

	if (nRet < 0)
	{
// 		PrintSysLog("CTSPacker::AudioMux(%s) => Error Mux() .\n",
// 			m_strMediaName.c_str());

		return -1;
	}

	return 0;
}

void CTSPacker::EnsureAudioStream()
{
	if (!(m_AVFlags&STREAM_AUDIO_FLAG))
	{
		m_AVFlags |= STREAM_AUDIO_FLAG;
		m_Program->AddStream(m_StreamAudio);
		if (m_AVFlags&STREAM_VIDEO_FLAG)
		{
			m_Muxer->DeliverPAT();
			m_Muxer->DeliverPMT();
		}		
	}

	return;
}

void CTSPacker::EnsureVideoStream()
{
	if (!(m_AVFlags&STREAM_VIDEO_FLAG))
	{
		m_AVFlags |= STREAM_VIDEO_FLAG;
		m_Program->AddStream(m_StreamVideo);
	//	m_Program->AddStream(m_StreamAudio);
		m_Muxer->DeliverPAT();
		m_Muxer->DeliverPMT();
	}
}
