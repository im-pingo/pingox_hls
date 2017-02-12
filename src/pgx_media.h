#ifndef _MEDIA_H
#define _MEDIA_H
#include "pgx_ts_file.h"
#include <math.h>

class CMedia:public CStreamCache_TSFile
{
public:
	CMedia();
	virtual ~CMedia(void);
	
public:
	virtual bool IsReady();
	virtual int InputStream(char const *vpStreamBuff, int nBuffSize);
	virtual void UpdateDuration(unsigned int uiPTS);

	virtual int OpenMediaCache(const char *szRootDir, const char *szSubDir,
		const char *szStreamCacheName, float nTargetDuration, int nMediaIndex);

	virtual float GetDuration();
	virtual int   GetMediaIndex();
	virtual void FreeMedia();
	virtual void FinishMedia();
	virtual void CheckReady();

private:
	float   m_fTargetDuration;
	float m_fCurrentDuration;
	long long m_lDiff ;

	unsigned int m_uiLastPTS;

	bool  m_bReady;
	int  m_nMediaIndex;

};

#endif
