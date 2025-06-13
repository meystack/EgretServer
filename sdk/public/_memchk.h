#ifndef	_MEMLEAK_H_
#define	_MEMLEAK_H_

#ifdef	_MLIB_DUMP_MEMORY_LEAKS_

	#include <malloc.h>
	#include <string>

	#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
	#define _CRTDBG_MAP_ALLOC
	#ifdef WIN32
	#include <crtdbg.h>
	#endif
	//解决placement new与内存检查机制冲突的问题
	#ifndef __PLACEMENT_NEW_INLINE
		#define new DEBUG_CLIENTBLOCK
	#endif

#endif

#endif
