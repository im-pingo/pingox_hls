#ifndef _STREAM_CACHE_H
#define _STREAM_CACHE_H

#include <string>
using namespace std;

class CStreamCache
{
public:
	CStreamCache();
	~CStreamCache(void);

public:
	virtual int InputStream(char const *vpStreamBuff, int nBuffSize) = 0;
	virtual void FreeStreamCache()   = 0;
	virtual void FinishStreamCache() = 0;
};

#endif
