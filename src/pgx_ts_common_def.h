#ifndef _TS_COMMON_DEF__H
#define _TS_COMMON_DEF__H

#ifdef WIN32

#ifndef LIB_TS_DLL_OUTPUT
#define LIB_TS_DLL_OUTPUT 1
#endif

#if LIB_TS_DLL_OUTPUT
#define LIB_TINY_TS_API __declspec(dllexport)
#else
#define LIB_TINY_TS_API __declspec(dllimport)
#endif
#else
#define LIB_TINY_TS_API
#endif

#endif
