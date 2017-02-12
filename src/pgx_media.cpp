//#include "../libLog/PrintLog.h"
#include "pgx_media.h"
//#include "utils/log/PrintLog.h"


CMedia::CMedia()
{
	m_fCurrentDuration = 0;
	m_fTargetDuration = 0;
	m_bReady    = false;
	m_nMediaIndex = 0;
	m_uiLastPTS = 0;
	m_lDiff = 0;
}

CMedia::~CMedia(void)
{
}

void CMedia::FinishMedia()
{
	CStreamCache_TSFile::FinishStreamCache();
}

void CMedia::FreeMedia()
{
	CStreamCache_TSFile::FreeStreamCache();
	m_fCurrentDuration = 0;
	m_fTargetDuration = 0;
	m_bReady    = false;
	m_nMediaIndex = 0;
	m_uiLastPTS = 0;
	m_lDiff = 0;
}

bool CMedia::IsReady()
{
	return m_bReady;
}

int CMedia::GetMediaIndex()
{
	return m_nMediaIndex;
}

int CMedia::OpenMediaCache(const char *szRootDir, const char *szSubDir,
	const char *szStreamCacheName, float nTargetDuration, int nMediaIndex)
{
	m_lDiff = 0;
	m_uiLastPTS = 0;
	m_bReady = false;
	m_fTargetDuration = nTargetDuration;
	m_nMediaIndex = nMediaIndex;

	int nRet = OpenStreamCache(szRootDir, szSubDir, szStreamCacheName);
	
	if (nRet < 0)
	{
	//	printf("CMedia::OpenMediaCache() => Fail to OpenStreamCache(%s%s%s) .\n", szRootDir, szSubDir, szStreamCacheName);
	
		return -1;
	}

	return nRet;
}

int CMedia::InputStream(char const *vpStreamBuff, int nBuffSize)
{
	int nRet = CStreamCache_TSFile::InputStream(vpStreamBuff, nBuffSize);
	if (nRet < 0)
	{
		printf("CMedia::InputStream(%s) => Fail to InputStream .\n", GetWholeFilePath());
		return -1;
	}

	return 0;
}

void CMedia::UpdateDuration(unsigned int uiPTS)
{
	if(m_uiLastPTS == 0)
	{
		m_uiLastPTS = uiPTS;
	}

	int lDiff = uiPTS-m_uiLastPTS;
	if (lDiff<0)
	{
		lDiff = m_lDiff;
// 		printf("CMedia::UpdateDuration(%s) => lastPTS=%u, currPTS=%u, diff=%d, duration = %0.3f \n", 
// 			CStreamCache_TSFile::GetStreamCacheName(), m_uiLastPTS, uiPTS, lDiff, m_fCurrentDuration);
	}
	else if (lDiff>2000)
	{
		lDiff = m_lDiff;
// 		printf("CMedia::UpdateDuration(%s) => lastPTS=%u, currPTS=%u, diff=%d, duration = %0.3f \n", 
// 		  	CStreamCache_TSFile::GetStreamCacheName(), m_uiLastPTS, uiPTS, lDiff, m_fCurrentDuration);

	}
	else
	{
		m_lDiff = lDiff;
	}


	m_fCurrentDuration += (lDiff)/1000.0;
	if (m_fCurrentDuration>m_fTargetDuration)
	{
// 		printf("CMedia::UpdateDuration(%s) => Warnning *********** duration = %0.3f \n", 
// 			CStreamCache_TSFile::GetStreamCacheName(), m_fCurrentDuration);
// 		printf("CMedia::UpdateDuration(%s) => lastPTS=%u, currPTS=%u, diff=%d, duration = %0.3f \n", 
// 			CStreamCache_TSFile::GetStreamCacheName(), m_uiLastPTS, uiPTS, lDiff, m_fCurrentDuration);
	}
//   	printlog("CMedia::UpdateDuration(%s) => lastPTS=%lu, currPTS=%lu, diff=%d, duration = %0.3f \n", 
//   		CStreamCache_TSFile::GetStreamCacheName(), m_uiLastPTS, uiPTS, lDiff, m_fCurrentDuration);
	m_uiLastPTS = uiPTS;
	return;
}

float CMedia::GetDuration()
{
	if (m_fCurrentDuration>m_fTargetDuration)
	{
// 		printf("CMedia::GetDuration(%s) => Warning ********** current duration(%0.3f) > targetDuration(%d) .\n",
// 			CStreamCache_TSFile::GetStreamCacheName(),m_fCurrentDuration, m_nTargetDuration);
	}
	return m_fCurrentDuration;
}

void CMedia::CheckReady()
{
	//m_bReady = (ceil(m_fCurrentDuration)>=m_nTargetDuration);
	m_bReady = ((m_fCurrentDuration)>=m_fTargetDuration);
}
