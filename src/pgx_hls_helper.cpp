#include "pgx_hls_helper.h"


CHLSHelper::CHLSHelper(int nCapacity, bool bAllowCache, int version,
	u32 videoStreamType, u32 audioStreamType):
CMediaPlaylist(nCapacity, bAllowCache, version),
CTSPacker(videoStreamType, audioStreamType)
{
}


CHLSHelper::~CHLSHelper(void)
{
}

int CHLSHelper::AfterPacketSDT(const unsigned char* data, const unsigned long size)
{

#if 0
	if (CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
		return -1;
	}
#endif

	return 0;
}

int CHLSHelper::AfterPacketPAT(const unsigned char* data, const unsigned long size)
{
	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketPAT(%s) => Fail to UpdateStreamCache() .\n",
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}

	return 0;
}

int CHLSHelper::AfterPacketPMT(const unsigned char* data, const unsigned long size)
{
	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketPMT(%s) => Fail to UpdateStreamCache() .\n",
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}

	return 0;
}

int CHLSHelper::AfterPacketIFrame(const unsigned char* data, const unsigned long size, const u64 millSec)
{

	CMediaPlaylist::UpdateDuration(millSec);
	int nRet = CMediaPlaylist::UpdatePlaylist();
	if (nRet < 0)
	{
	//	PrintSysLog("CHLSHelper::AfterPacketIFrame(%s) => Error UpdatePlaylist() .\n", CMediaPlaylist::GetMediaName().c_str());
	}
	else if(nRet == 1)
	{
		//CTSPacker::DeliverSDT();
		CTSPacker::DeliverPAT();
		CTSPacker::DeliverPMT();
	}

	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketIFrame(%s) => Fail to UpdateStreamCache() .\n",
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}

	return 0;
}

int CHLSHelper::AfterPacketPFrame(const unsigned char* data, const unsigned long size, const u64 millSec)
{
#if 0
	CMediaPlaylist::UpdateDuration(millSec);
	
	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketPFrame(%s) => Fail to UpdateStreamCache() .\n",
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}
#else
	CMediaPlaylist::UpdateDuration(millSec);
	int nRet = CMediaPlaylist::UpdatePlaylist();
	if (nRet < 0)
	{
	//	PrintSysLog("CHLSHelper::AfterPacketPFrame(%s) => Error UpdatePlaylist() .\n", CMediaPlaylist::GetMediaName().c_str());
	}
	else if(nRet == 1)
	{
	//	CTSPacker::DeliverSDT();
		CTSPacker::DeliverPAT();
		CTSPacker::DeliverPMT();
	}

	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketPFrame(%s) => Fail to UpdateStreamCache() .\n",
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}

#endif
	return 0;
}

int CHLSHelper::AfterPacketAudioFrame(const unsigned char* data, const unsigned long size, const u64 millSec)
{
	if(CMediaPlaylist::UpdateStreamCache((const char*)data, size)<0)
	{
// 		PrintSysLog("CHLSHelper::AfterPacketAudioFrame(%s) => Fail to UpdateStreamCache() .\n", 
// 			CMediaPlaylist::GetMediaName().c_str());
		return -1;
	}

	return 0;
}

int CHLSHelper::AfterPacketEmptyFrame(const unsigned char* data, const unsigned long size, const u64 millSec)
{
	return 0;
}
