//
// 2022-06-01, jjuiddong
// common definition
//
#pragma once


// 매크로 정의
#ifndef SAFE_DELETE
	#define SAFE_DELETE(p) {if (p) { delete p; p=NULL;} }
#endif
#ifndef SAFE_DELETEA
	#define SAFE_DELETEA(p) {if (p) { delete[] p; p=NULL;} }
#endif
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p) {if (p) { p->Release(); p=NULL;} }
#endif
#ifndef SAFE_RELEASE2
	#define SAFE_RELEASE2(p) {if (p) { p->release(); p=NULL;} }
#endif
#ifndef DX_SAFE_RELEASE
	#define DX_SAFE_RELEASE(p) {if (p) { p->Release(); p=NULL;} }
#endif

#ifndef IN
	#define IN
#endif

#ifndef OUT
	#define OUT
#endif

#ifndef INOUT
	#define INOUT
#endif

#define RET(exp)	{if((exp)) return; } // exp가 true이면 리턴
#define RET2(exp)	{if((exp)) {assert(0); return;} } // exp가 true이면 리턴
#define RETV(exp,val)	{if((exp)) return val; }
#define RETV2(exp,val)	{if((exp)) {assert(0); return val;} }
#define ASSERT_RET(exp)	{assert(exp); RET(!(exp) ); }
#define ASSERT_RETV(exp,val)	{assert(exp); RETV(!(exp),val ); }
#define BRK(exp)	{if((exp)) break; } // exp가 break

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
#define _TRUNCATE ((size_t)-1) // corecrt.h

typedef unsigned int hashcode;

#ifndef uchar
	typedef unsigned char uchar;
#endif

#ifndef ushort
	typedef unsigned short ushort;
#endif

#ifndef wchar
	typedef wchar_t wchar;
#endif

#ifndef uint
	typedef unsigned int uint;
#endif

#ifndef WIN32
    #define __int64 long long
#endif

// #ifndef int64
// 	typedef __int64 int64;
// #endif
#ifndef int64
	typedef __int64 int64;
#endif

#ifndef uint64
	typedef unsigned __int64 uint64;
#endif 

#ifndef WPARAM
	typedef int WPARAM;
#endif

#ifndef LPARAM
	typedef int LPARAM;
#endif

// #ifndef interface
// 	typedef interface struct
// #endif

#ifndef __forceinline
	#define __forceinline __attribute__((always_inline))
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef BYTE
	typedef unsigned char BYTE;
#endif

#ifndef DWORD
	typedef unsigned int DWORD;
#endif

#ifndef UINT
	typedef unsigned int UINT;
#endif

#ifndef LONG
	typedef long LONG;
#endif

#ifndef RECT
	struct RECT {
		int left, top, right, bottom;
	};
#endif


#define MAX_PATH	260
#define INFINITE    0xFFFFFFFF  // Infinite timeout

#ifndef memcpy_s
	#define memcpy_s(dst, dstSize, src, cpySize) \
		memcpy(dst, src, cpySize)
#endif

#ifndef strncat_s
	#define strncat_s(dst, src, size) \
		strncat(dst, src, size)
#endif

#ifndef ZeroMemory
	#define ZeroMemory(src, size) \
		memset(src, 0, size)
#endif

#ifndef vsnprintf_s
	#define vsnprintf_s(buffer, sizeOfBuffer, count, format, ...) \
		vsnprintf(buffer, sizeOfBuffer, format, ##__VA_ARGS__)
#endif

#ifndef sprintf_s
	#define sprintf_s(buffer, size, format, ...) \
		sprintf(buffer, format, ##__VA_ARGS__)
#endif

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <set>
#include <queue>
#include <thread>
#include <iterator>
#include <stdarg.h>
#include <cstring>
#include <assert.h>
#include <atomic>
#include <semaphore.h> // POSIX semaphore

#include <boost/functional/hash.hpp>   //boost::hash
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

using std::string;
using std::wstring;


using std::vector;
using std::map;
using std::list;
using std::set;
using std::queue;
using std::deque;
using std::stringstream;
using std::wstringstream;
using std::min;
using std::max;


#include "container/vectorhelper.h"
#include "container/vectormap.h"
#include "container/circularqueue.h"
#include "container/circularqueue2.h"
#include "etc/filepath1.h"
#include "container/simplestring.h"
#include "etc/autocs.h"
#include "math/Math.h"
#include "etc/recttype.h"
#include "etc/genid.h"

#include "etc/memorypool3.h"
#include "etc/memorypool4.h"

#include "etc/date.h"
#include "etc/date2.h"
#include "etc/timer.h"
#include "etc/stringfunc.h"

#include "thread/semaphore.h"
#include "thread/task.h"
#include "thread/mutex.h"
#include "thread/thread1.h"
#include "thread/wqsemaphore.h"
#include "thread/tpsemaphore.h"


#include "etc/dbg.h"
