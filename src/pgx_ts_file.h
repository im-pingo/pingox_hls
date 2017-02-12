#ifndef _TS_FILE_CACHE_H
#define _TS_FILE_CACHE_H

#include "pgx_stream_cache.h"

#define MAX_PATH_LENGTH 1024
#ifndef gap_char
#define gap_char ('/')
#endif
#ifndef gap_str
#define gap_str ("/")
#endif

class CStreamCache_TSFile :
	public CStreamCache
{
public:
	CStreamCache_TSFile(void);
	~CStreamCache_TSFile(void);
	
	virtual int OpenStreamCache(string strRootPath, string strSubPath, string strStreamCacheName);
	virtual int InputStream(char const *vpStreamBuff, int nBuffSize) ;
	virtual void FreeStreamCache() ;
	virtual void FinishStreamCache();
	
	static int DeleteFile(string strFile);
	static int DeleteDir(string strDir);
	static int EnsureDirExist ( string dir );

	const char *GetStreamCacheName();
	const char *GetWholeDir();
	const char *GetWholeFilePath();
	const char *GetSubDir();
	const char *GetRootDir();

	int  UpdateLastTS();

private:
	
	FILE *m_fTSCache;
	string m_strStreamCacheName;
	string m_strRootDir;
	string m_strSubDir;
	string m_strWholeDir;
	string m_strWholeFilePath;
	string m_strLastTS;
};

#endif