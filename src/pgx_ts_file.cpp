#ifdef _WIN32
#define _AFXDLL
#include <afx.h>
#endif

//#include "../libLog/PrintLog.h"
#include "pgx_ts_file.h"
#include <string.h>
#ifndef _WIN32
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#else
#include <io.h>
#include <direct.h>
//#include <windows.h>


#endif


CStreamCache_TSFile::CStreamCache_TSFile(void)
{
	m_fTSCache = NULL;
	m_strStreamCacheName.clear();
	m_strRootDir.clear();
	m_strSubDir.clear();
	m_strWholeDir.clear();
	m_strWholeFilePath.clear();
	m_strLastTS.clear();
}


CStreamCache_TSFile::~CStreamCache_TSFile(void)
{
	FreeStreamCache();
}

int CStreamCache_TSFile::UpdateLastTS()
{
	if (m_strLastTS.length()>0)
	{
		if(DeleteFile(m_strLastTS))
		{
		//	PrintSysLog("CStreamCache_TSFile::UpdateLastTS() => Fail to delete %s .\n", m_strLastTS.c_str());
			//return -1;
		}
	}

	m_strLastTS = m_strWholeFilePath;

	return 0;
}

void CStreamCache_TSFile::FinishStreamCache()
{
	if (m_fTSCache)
	{
		fflush(m_fTSCache);
		fclose(m_fTSCache);
		m_fTSCache = NULL;
	}
}

void CStreamCache_TSFile::FreeStreamCache()
{
	if (m_fTSCache)
	{
		fclose(m_fTSCache);
		m_fTSCache = NULL;
	}
#if 0
	if (m_strWholeFilePath.length()>0)
	{
		if(DeleteFile(m_strWholeFilePath)<0)
		{
			printlog("CStreamCache_TSFile::FinishStreamCache() => Fail to delete %s .\n", m_strWholeFilePath.c_str());
		}
	}
#else
	UpdateLastTS();
#endif
	m_fTSCache = NULL;
	m_strStreamCacheName.clear();
	m_strRootDir.clear();
	m_strSubDir.clear();
	m_strWholeDir.clear();
	m_strWholeFilePath.clear();

}

int CStreamCache_TSFile::DeleteFile(string strFile)
{
	return unlink(strFile.c_str());
}

int CStreamCache_TSFile::DeleteDir(string strDir)
{
#ifdef _WIN32

	CFileFind tempFind; 
    string sTempFileFind = strDir+"*.*"; 
    bool IsFinded = tempFind.FindFile(sTempFileFind.c_str()); 
    while (IsFinded) 
    { 
        IsFinded = tempFind.FindNextFile(); 
        if (!tempFind.IsDots())
        { 
            char sFoundFileName[ _MAX_PATH ] = { 0 }; 
            strcpy(sFoundFileName,tempFind.GetFileName().GetBuffer(200)); 
            if (tempFind.IsDirectory()) 
            { 
                string sTempDir = strDir+sFoundFileName; 
                DeleteDir(sTempDir.c_str()); 
            } 
            else 
            { 
                string sTempFileName = strDir + sFoundFileName; 
                DeleteFile(sTempFileName); 
            } 
        } 
    } 
    tempFind.Close(); 
    if(!RemoveDirectory(strDir.c_str())) 
    { 
        return -1; 
	} 
    return 0; 

#else
    string rm_it = "rm -rf " + strDir;
    system(rm_it.c_str());
    return 0;
#endif
}


int CStreamCache_TSFile::EnsureDirExist ( string dir )
{
	int len = 0 ;
	char temp_dir[MAX_PATH_LENGTH] ;
	char * pindex = NULL ;
	int access_flag = 0 ;
	const char * p_dir = dir.c_str () ;

	if ( !access ( p_dir , 0 ) )
	{
		return 1 ;
	}
	//
	len = dir.size() ;
	if ( len >= MAX_PATH_LENGTH )
	{
		return 0 ;
	}

	//
	memset ( temp_dir , 0 , MAX_PATH_LENGTH ) ;
	memcpy ( temp_dir , p_dir , len ) ;

	//
	pindex = temp_dir ;
	pindex = strrchr ( pindex , gap_char ) ;
	if ( pindex >= (temp_dir + len - 1) )
	{
		*pindex = 0 ;
	}

	//
	pindex = temp_dir ;
	pindex = strchr( pindex , gap_char ) ;
	if ( !pindex)
	{
		return 0 ;
	}
	pindex ++ ;

	//
	access_flag = 0 ;
	do
	{
		pindex = strchr( pindex , gap_char ) ;

		if ( pindex )
		{
			*pindex = 0 ;
		}

		access_flag = access ( temp_dir , 0 ) ;

		if ( access_flag )
		{
#ifdef _WIN32
			if ( mkdir ( temp_dir  ) )
#else
			if(mkdir(temp_dir, 0x777))
#endif
			{
				return 0 ;
			}
		}

		if ( pindex )
		{
			*pindex = gap_char ;
			pindex ++ ;
		}
	} while ( pindex );

	//
	if ( access ( p_dir , 0 ) )
	{
		return 0 ;
	}
	return 1 ;
}



int CStreamCache_TSFile::InputStream(char const *vpStreamBuff, int nBuffSize) 
{
	if (m_fTSCache == NULL)
	{
	//	PrintSysLog("CStreamCache_TSFile::InputStream() => fTSCache is NULL .\n");
		return -1;
	}

	int nret = fwrite(vpStreamBuff, 1, nBuffSize, m_fTSCache);
	if (nret<=0)
	{
	//	PrintSysLog("CStreamCache_TSFile::InputStream() => Fail to fwrite %s .\n", m_strWholeFilePath.c_str());
		return -2;
	}
	else if(nret<nBuffSize)
	{
	//	PrintSysLog("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}

	fflush(m_fTSCache);

	return 0;
}

int CStreamCache_TSFile::OpenStreamCache(string strRootPath, string strSubPath, string strStreamCacheName)
{
	m_strRootDir = strRootPath;
	m_strSubDir  = strSubPath;
	m_strWholeDir = m_strRootDir+strSubPath;
	m_strStreamCacheName = strStreamCacheName;
	m_strWholeFilePath = m_strWholeDir + strStreamCacheName;

//	printlog("CStreamCache_TSFile::OpenStreamCache() => whole path : %s .\n", m_strWholeDir.c_str());

	if(!CStreamCache_TSFile::EnsureDirExist(m_strWholeDir))
	{
	//	PrintSysLog("CStreamCache_TSFile::OpenStreamCache() => Dir is not exsit %s .\n", m_strWholeDir.c_str());
		return -1;
	}


	if (m_fTSCache!=NULL)
	{
		fclose(m_fTSCache);
		m_fTSCache;
	}

	m_fTSCache = fopen(m_strWholeFilePath.c_str(), "wb+");
	if (m_fTSCache == NULL)
	{
	//	PrintSysLog("CStreamCache_TSFile::InputStream() => Fail to open %s .\n", m_strWholeFilePath.c_str());
		return -1;
	}

	return 0;
}

const char * CStreamCache_TSFile::GetStreamCacheName()
{
	return m_strStreamCacheName.c_str();
}

const char * CStreamCache_TSFile::GetWholeDir()
{
	return m_strWholeDir.c_str();
}

const char * CStreamCache_TSFile::GetWholeFilePath()
{
	return m_strWholeFilePath.c_str();
}

const char * CStreamCache_TSFile::GetSubDir()
{
	return m_strSubDir.c_str();
}

const char * CStreamCache_TSFile::GetRootDir()
{
	return m_strRootDir.c_str();
}

