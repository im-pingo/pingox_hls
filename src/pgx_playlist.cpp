//#include "../libLog/PrintLog.h"
#include "pgx_playlist.h"


CMediaPlaylist::CMediaPlaylist(int nCapacity, bool bAllowCache, int version)
{
	m_bAllowCache = bAllowCache;
	m_version = version;
	m_pCurrMedia = NULL;
	m_nMediaCount = 0;
	m_fTargetDuration = 0;
	m_strRootDir.clear();
	m_strSubDir.clear();
	m_strWholeDir.clear();
	m_strMediaName.clear();
	m_strPlaylistPath.clear();
	m_nCapacity = nCapacity;
	m_nMediaCacheIndex = 0;
	m_pMediaCaches = new CMedia[nCapacity+1];
	m_strM3U8Name.clear();
}


CMediaPlaylist::~CMediaPlaylist(void)
{
	StopStreamCache();

	if (m_pMediaCaches)
	{
		delete[] m_pMediaCaches;
		m_pMediaCaches = NULL;
	}

}

int CMediaPlaylist::GetCurrtentCapacity()
{
	return m_playlist.size();
}

int CMediaPlaylist::UpdatePlaylist()
{

	if (m_pCurrMedia==NULL)
	{
	//	PrintSysLog("CMediaPlaylist::UpdatePlaylist(%s) => Current Media is invalid .\n", m_strMediaName.c_str());
		return -1;
	}

	m_pCurrMedia->CheckReady();
	if (!m_pCurrMedia->IsReady())
	{
		//PrintSysLog("CMediaPlaylist::UpdatePlaylist(%s) => Current Media isnot ready .\n", m_strMediaName.c_str());
		return 0;
	}
	else
	{
		//PrintSysLog("CMediaPlaylist::UpdatePlaylist(%s) => Current Media is ready, update now .\n", m_strMediaName.c_str());
	}

	m_pCurrMedia->FinishMedia();
	
	CMedia *pDestroyMedia = NULL;

	if (m_playlist.size()<m_nCapacity)
	{
		m_playlist.push_back(m_pCurrMedia);
	}
	else
	{
		pDestroyMedia = m_playlist.front();
		m_playlist.pop_front();

		m_playlist.push_back(m_pCurrMedia);
	}
	
	m_pCurrMedia = NULL;

	if (UpdateM3u8() < 0)
	{
	//	PrintSysLog("CMediaPlaylist::UpdatePlaylist(%s) => Fail to UpdateM3u8() .\n", m_strMediaName.c_str());
		return -1;
	}

	if (pDestroyMedia)
	{
		pDestroyMedia->FreeMedia();
	}

	m_pCurrMedia = DispatchMediaCache();
	if (m_pCurrMedia == NULL)
	{
	//	PrintSysLog("CMediaPlaylist::UpdatePlaylist(%s) => Fail to DispatchMediaCache() .\n", m_strMediaName.c_str());
		return -1;
	}

	return 1;
}

string CMediaPlaylist::HeaderLineToString()
{
	char szConent[265] = {0};
	sprintf(szConent, "%s%s", HLS_M3U8_HEAD, HLS_M3U8__LINE_END_TAR);
	return szConent;
}

string CMediaPlaylist::AllowCacheToString()
{
	char szConent[265] = {0};
	sprintf(szConent, "%s%s", m_bAllowCache?HLS_M3U8_CACHE_YES:HLS_M3U8_CACHE_NO, HLS_M3U8__LINE_END_TAR);
	return szConent;
}

string CMediaPlaylist::VersionToString()
{
	char szVersion[256] = {0};
	sprintf(szVersion, "%s%d%s", HLS_M3U8_VERSION, m_version, HLS_M3U8__LINE_END_TAR);
	return szVersion;
}

string CMediaPlaylist::SequenceToString()
{
	int nSequence = 0;
	if (m_playlist.size() == 0)
	{
//		PrintSysLog("CMediaPlaylist::SequenceToString(%s) => playlist is empty .\n", m_strMediaName.c_str());
	}
	else
	{
		CMedia *pFirstMedia = m_playlist.front();
		if (pFirstMedia == NULL)
		{
	//		PrintSysLog("CMediaPlaylist::SequenceToString(%s) => first media is null .\n", m_strMediaName.c_str());
		}
		else
		{
			nSequence = pFirstMedia->GetMediaIndex();
		}
	}
	
	char szSequence[512] = {0};
	sprintf(szSequence, "%s%d%s", HLS_M3U8_SEQUENCE, nSequence, HLS_M3U8__LINE_END_TAR);

	return szSequence;
}

string CMediaPlaylist::TarDurationToString()
{
	char szTarDuration[512] = {0};
#if 0
	sprintf(szTarDuration, "%s%d%s", HLS_M3U8_TARDURATION, m_nTargetDuration, HLS_M3U8__LINE_END_TAR);
#else
	float fRealDuration = 0;

	for (list<CMedia*>::iterator it = m_playlist.begin(); it != m_playlist.end(); ++it)
	{
		if (fRealDuration<(*it)->GetDuration())
		{
			//nRealDuration = ceil((*it)->GetDuration());
			fRealDuration = (*it)->GetDuration();
// 			if (fRealDuration>=m_fTargetDuration)
// 			{
// 				PrintSysLog("CMediaPlaylist::TarDurationToString() => Warning ******** tarDuration = %d \n", nRealDuration);
// 			}
		}
	}
	sprintf(szTarDuration, "%s%d%s", HLS_M3U8_TARDURATION, (int)ceil(fRealDuration), HLS_M3U8__LINE_END_TAR);
//	sprintf(szTarDuration, "%s%.1f%s", HLS_M3U8_TARDURATION, nRealDuration, HLS_M3U8__LINE_END_TAR);
#endif

	return szTarDuration;
}

string CMediaPlaylist::MediasLineToString()
{
	string strMedias = "";
	char szMediasLine[2048] = {0};
	for (list<CMedia*>::iterator it = m_playlist.begin(); it != m_playlist.end(); ++it)
	{
		sprintf(szMediasLine, "%s%0.3f,%s%s%s",
			HLS_M3U8_EXTINF, (*it)->GetDuration(), HLS_M3U8__LINE_END_TAR,
			(*it)->GetStreamCacheName(), HLS_M3U8__LINE_END_TAR);
		strMedias += szMediasLine;
	}

	return strMedias;
}

string CMediaPlaylist::EndlistToString()
{
	char szConent[265] = {0};
	sprintf(szConent, "%s%s", HLS_M3U8_ENDLIST, HLS_M3U8__LINE_END_TAR);
	return szConent;
}

string CMediaPlaylist::GetM3U8File()
{
	return m_strM3U8Name;
}

string CMediaPlaylist::GetSubDir()
{
	return m_strSubDir;
}

int CMediaPlaylist::UpdateM3u8()
{
	string strM3U8 = "";

	strM3U8 = HeaderLineToString()+
	AllowCacheToString()+
	VersionToString()+
	SequenceToString()+
	TarDurationToString()+
	MediasLineToString()/*+
	EndlistToString()*/;

	if(!CStreamCache_TSFile::EnsureDirExist(m_strWholeDir))
	{
	//	PrintSysLog("CMediaPlaylist::UpdateM3u8(%s) => %s isnot exist .\n", m_strMediaName.c_str(), m_strWholeDir.c_str());
		return -1;
	}

	FILE *f = fopen(m_strPlaylistPath.c_str(), "wb+");
	if (f == NULL)
	{
	//	PrintSysLog("CMediaPlaylist::UpdateM3u8(%s) => Fail to fopen(%s) .\n", m_strMediaName.c_str(), m_strPlaylistPath.c_str());
		return -1;
	}

	int nRet = fwrite(strM3U8.c_str(), 1, strM3U8.length(), f);
	if (nRet <= 0)
	{
	//	PrintSysLog("CMediaPlaylist::UpdateM3u8(%s) => Fail to fwrite(%s) .\n", m_strMediaName.c_str(), m_strPlaylistPath.c_str());
	}

	fflush(f);
	fclose(f);
	f = NULL;

	return 0;
}

void CMediaPlaylist::ResetStreamCache(string strRootDir, string strSubDir, string strMediaName, float nTargetDuration)
{
	m_strRootDir = strRootDir;
	m_strSubDir  = strSubDir;
	m_strWholeDir = m_strRootDir+m_strSubDir;
	m_strMediaName = strMediaName;
	m_strM3U8Name = m_strMediaName+".m3u8";
	m_strPlaylistPath = m_strWholeDir+m_strM3U8Name;
	m_fTargetDuration = nTargetDuration;
	m_nMediaCacheIndex = 0;
	m_nMediaCount = 0;
	m_pCurrMedia = NULL;

	CStreamCache_TSFile::DeleteDir(m_strWholeDir);
}

string CMediaPlaylist::GetMediaName()
{
	return m_strMediaName;
}

int CMediaPlaylist::StopStreamCache()
{
	for (int i= 0; i<m_nCapacity; ++i)
	{
		m_pMediaCaches[i].FreeMedia();
	}
#if 1
	if (m_strWholeDir.length()>0)
	{
		if(CStreamCache_TSFile::DeleteDir(m_strWholeDir) != 0)
		{
// 			PrintSysLog("CMediaPlaylist::StopStreamCache(%s) => Fail to delete dir %s .\n", 
// 				m_strMediaName.c_str(), m_strWholeDir.c_str());
		}
		else
		{
// 			PrintSysLog("CMediaPlaylist::StopStreamCache(%s) => Succ to delete dir %s .\n", 
// 				m_strMediaName.c_str(), m_strWholeDir.c_str());
		}
	}
#endif

	m_nMediaCount = 0;
	m_fTargetDuration = 0;
	m_strRootDir.clear();
	m_strSubDir.clear();
	m_strWholeDir.clear();
	m_strMediaName.clear();
	m_strPlaylistPath.clear();
	m_nMediaCacheIndex = 0;
	m_pCurrMedia = NULL;
	m_strM3U8Name.clear();
	
	return 0;
}

float CMediaPlaylist::GetCurrentMediaDuration()
{
	if (m_pCurrMedia == NULL)
	{
		return 0;
	}
	return m_pCurrMedia->GetDuration();
}

bool CMediaPlaylist::IsCurrentMediaReady()
{
	if (m_pCurrMedia == NULL)
	{
		m_pCurrMedia = DispatchMediaCache();
		if (m_pCurrMedia == NULL)
		{
		//	PrintSysLog("CMediaPlaylist::IsCurrentMediaReady(%s) => Fail to DispatchMediaCache() .\n", m_strMediaName.c_str());
			return false;
		}
		
		return false;
	}
	
	return m_pCurrMedia->IsReady();
}

void CMediaPlaylist::UpdateDuration(unsigned long long uPTS)
{
	if (m_pCurrMedia == NULL)
	{
		m_pCurrMedia = DispatchMediaCache();
		if (m_pCurrMedia == NULL)
		{
	//		PrintSysLog("CMediaPlaylist::UpdateDuration(%s) => Fail to DispatchMediaCache() .\n", m_strMediaName.c_str());
			return ;
		}
	}

	m_pCurrMedia->UpdateDuration(uPTS);
}

int CMediaPlaylist::UpdateStreamCache(char const *pStreamData, int nDatalen)
{
	int nRet = 0;

	if (m_pCurrMedia == NULL)
	{
		m_pCurrMedia = DispatchMediaCache();
		if (m_pCurrMedia == NULL)
		{
	//		PrintSysLog("CMediaPlaylist::UpdateStreamCache(%s) => Fail to DispatchMediaCache() .\n", m_strMediaName.c_str());
			return -1;
		}
	}
	
	nRet = m_pCurrMedia->InputStream(pStreamData, nDatalen);
	if (nRet < 0)
	{
	//	PrintSysLog("CMediaPlaylist::UpdateStreamCache(%s) => Fail to InputStream() .\n", m_strMediaName.c_str());
	}

	return 0;
}

CMedia *CMediaPlaylist::DispatchMediaCache()
{
	if (m_nMediaCacheIndex<0)
	{
		m_nMediaCacheIndex = 0;
	}

	if (m_nMediaCacheIndex>m_nCapacity)
	{
		m_nMediaCacheIndex = 0;
	}

	CMedia *pRet = NULL;
	for (int i = 0; i<m_nCapacity+1; ++i)
	{
		if(m_pMediaCaches[i].IsReady() == false)
		{
			pRet = &(m_pMediaCaches[i]);
			m_nMediaCacheIndex = i;
			break;
		}
	}

	if (pRet!=NULL)
	{
		if(OpenMediaCache(pRet)<0)
		{
		//	PrintSysLog("CMediaPlaylist::DispatchMediaCache(%s) => Fail to OpenMediaCache() .\n", m_strMediaName.c_str());
			return NULL;
		}
	}

	return pRet;
}

int CMediaPlaylist::OpenMediaCache(CMedia* pMedia)
{
	char szStreamCacheName[1024] = {0};
	sprintf(szStreamCacheName, "%s_%d.ts", m_strMediaName.c_str(), m_nMediaCount);

	if(pMedia->OpenMediaCache(m_strRootDir.c_str(), m_strSubDir.c_str(), szStreamCacheName, m_fTargetDuration, m_nMediaCount)<0)
	{
	//	PrintSysLog("CMediaPlaylist::OpenMediaCache(%s) => Fail to OpenMedisCache() .\n", m_strMediaName.c_str());
		return -1;
	}

	m_nMediaCount++;

	return 0;
}
