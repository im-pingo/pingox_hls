#ifndef _MEDIA_PLAYLIST_H
#define _MEDIA_PLAYLIST_H
#include <list>
#include "pgx_media.h"
#include "pgx_ts_common_def.h"

using namespace std;

#define HLS_M3U8_HEAD "#EXTM3U"
#define HLS_M3U8_CACHE_YES "#EXT-X-ALLOW-CACHE:YES"
#define HLS_M3U8_CACHE_NO  "#EXT-X-ALLOW-CACHE:NO"
#define HLS_M3U8_VERSION "#EXT-X-VERSION:"
#define HLS_M3U8_SEQUENCE "#EXT-X-MEDIA-SEQUENCE:"
#define HLS_M3U8_TARDURATION "#EXT-X-TARGETDURATION:"
#define HLS_M3U8_EXTINF "#EXTINF:"
#define HLS_M3U8_ENDLIST "#EXT-X-ENDLIST"
#define HLS_M3U8__LINE_END_TAR "\n"

class LIB_TINY_TS_API CMediaPlaylist
{

public:
	CMediaPlaylist(int nCapacity, bool bAllowCache, int version);
	virtual ~CMediaPlaylist(void);
	virtual void UpdateDuration(unsigned long long uPTS);
	virtual int UpdateStreamCache(char const *pStreamData, int nDatalen);

	virtual void ResetStreamCache(string strRootDir, string strSubDir, string strMediaName, float nTargetDuration);

	virtual bool IsCurrentMediaReady();

	virtual CMedia *DispatchMediaCache();

	virtual int OpenMediaCache(CMedia* pMedia);
	virtual float GetCurrentMediaDuration();
	virtual int UpdateM3u8();
	virtual string GetMediaName();

	virtual int StopStreamCache();

	virtual string GetM3U8File();
	virtual string GetSubDir();

	virtual int    GetCurrtentCapacity();
	virtual int UpdatePlaylist();

protected:

	virtual string HeaderLineToString();
	virtual string AllowCacheToString();
	virtual string VersionToString();
	virtual string SequenceToString();
	virtual string TarDurationToString();
	virtual string MediasLineToString();
	virtual string EndlistToString();

private:
	bool    m_bAllowCache;
	int     m_version;
	int     m_nCapacity;
	CMedia* m_pMediaCaches;
	int     m_nMediaCacheIndex;

	CMedia* m_pCurrMedia;
	list<CMedia*> m_playlist;
	int m_nMediaCount;
	float m_fTargetDuration;
	string m_strRootDir;
	string m_strSubDir;
	string m_strWholeDir;
	string m_strMediaName;
	string m_strPlaylistPath;
	string m_strM3U8Name;

};

#endif
