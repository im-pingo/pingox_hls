#ifndef _HLS_HELPER_H
#define _HLS_HELPER_H
#include "pgx_ts_packer.h"
#include "pgx_playlist.h"

class LIB_TINY_TS_API CHLSHelper:public CTSPacker, public CMediaPlaylist
{

public:
	CHLSHelper(int nCapacity, bool bAllowCache, int version, u32 videoStreamType, u32 audioStreamType);

	~CHLSHelper(void);
	
protected:

	virtual int AfterPacketSDT(const unsigned char* data, const unsigned long size);
	virtual int AfterPacketPAT(const unsigned char* data, const unsigned long size);
	virtual int AfterPacketPMT(const unsigned char* data, const unsigned long size);
	virtual int AfterPacketIFrame(const unsigned char* data, const unsigned long size, const u64 millSec);
	virtual int AfterPacketPFrame(const unsigned char* data, const unsigned long size, const u64 millSec);
	virtual int AfterPacketAudioFrame(const unsigned char* data, const unsigned long size, const u64 millSec);
	virtual int AfterPacketEmptyFrame(const unsigned char* data, const unsigned long size, const u64 millSec);
	
};

#endif