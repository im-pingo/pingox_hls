#ifndef _TS_PACKER_H
#define _TS_PACKER_H

#include "pgx_ts_muxer.h"
#include "pgx_pmt_packet.h"

class LIB_TINY_TS_API CTSPacker
{

	enum{
		STREAM_ZERO_FLAG  = 0x00,
		STREAM_VIDEO_FLAG = 0x01,
		STREAM_AUDIO_FLAG = 0x02,
		TS_PARAM_PID_PMT   = 0x100,
		TS_PARAM_PID_VIDEO = 0x102,
		TS_PARAM_PID_AUDIO = 0x101,
		TS_PARAM_PID_PCR   = 0x102,
		STREAM_ID_VIDEO = 0xe0,
		STREAMID_AUDIO  = 0xc0
	};

	public:
		CTSPacker();
		~CTSPacker(void);

		virtual void EnsureAudioStream();
		virtual void EnsureVideoStream();
		
		virtual int VideoMux(unsigned int uiFrameType, u8 *data, int dataLength, u64 pcr, u64 pts, u64 dts);
		virtual int AudioMux(u8 *data, int dataLength, /*u64 pcr,*/ u64 pts, u64 dts=~0);

	protected:

		static int RealPacketsDeliverer(const unsigned char* data, const unsigned long size,
			unsigned int uTSType, const u64 millSec, void* streaming);

		virtual int DeliverSDT();
		virtual int DeliverPAT();
		virtual int DeliverPMT();
		virtual int AfterPacketSDT(const unsigned char* data, const unsigned long size){return 0;};
		virtual int AfterPacketPAT(const unsigned char* data, const unsigned long size) = 0;
		virtual int AfterPacketPMT(const unsigned char* data, const unsigned long size) = 0;
		virtual int AfterPacketIFrame(const unsigned char* data, const unsigned long size, const u64 millSec) = 0;
		virtual int AfterPacketPFrame(const unsigned char* data, const unsigned long size, const u64 millSec) = 0;
		virtual int AfterPacketAudioFrame(const unsigned char* data, const unsigned long size, const u64 millSec) = 0;
		virtual int AfterPacketEmptyFrame(const unsigned char* data, const unsigned long size, const u64 millSec) = 0;

	private:

		string m_strMediaName;

		CTinyTSMuxer *m_Muxer;
		AACPESPacket *m_StreamAudio;
		H264PESPacket *m_StreamVideo;
		CTinyPMTPacket *m_Program;

		int m_PMTPID;
		int m_VideoPID;
		int m_AudioPID;
		int m_PCRPID;
		int m_VideoStreamID;
		int m_AudioStreamID;

		unsigned int m_AVFlags;

		unsigned long long m_ulLastAudioPTS;
		unsigned long long m_ulLastVideoPTS;

};

#endif
