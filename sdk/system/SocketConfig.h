#ifndef	_SOCKET_CONFIG_H_
#define	_SOCKET_CONFIG_H_

#include "Platform.h"

# ifdef WIN32
#   define socklen_t int
#	include <winsock.h>
# else
#	define SOCKET int
#	define SD_BOTH SHUT_RDWR
#   define INVALID_SOCKET   -1
#   define WSAGetLastError() errno
#   define WSAEWOULDBLOCK EWOULDBLOCK
#   define closesocket close
#   define OUT

    #include <errno.h>
    #include <sys/socket.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <netdb.h>
#	ifndef SOCKET_ERROR
#		define SOCKET_ERROR            (-1)
#	endif

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;

#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/ioctl.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <stdio.h>
# endif

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "_memchk.h"
#include "TypeDef.h"
#include "CustomSocket.h"
#include "wyString.h"

#endif //_SOCKET_CONFIG_H_
