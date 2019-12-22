/*
 * Copyright (c) 2012-2018 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Network access under Windows via WinSock 2.2.
 */

#include <sys/types.h>

#include <agar/core/core.h>
#include <agar/net/net.h>

#undef SLIST_ENTRY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32_WCE
# include <errno.h>
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iphlpapi.h>

static int agSockOptionMap[] = {
	0,
	SO_DEBUG,
	SO_REUSEADDR,
	SO_KEEPALIVE,
	SO_DONTROUTE,
	SO_BROADCAST,
	SO_SNDBUF,
	SO_RCVBUF
};
static _Nullable AG_Mutex agNetWin32Lock;

/* Convert an AF_* value to an enum ag_net_addr_family. */
static __inline__ enum ag_net_addr_family _Const_Attribute
GetAddrFamily(int family)
{
	switch (family) {
	case AF_INET:	return (AG_NET_INET4);
	case AF_INET6:	return (AG_NET_INET6);
	default:	return (0);
	}
}

/* Convert an AG_NetAddr to a struct sockaddr. */
static void
NetAddrToSockAddr(const AG_NetAddr *_Nonnull na,
    struct sockaddr_storage        *_Nonnull sa,
    socklen_t                      *_Nonnull saLen)
{
	memset(sa, 0, sizeof(struct sockaddr_storage));

	switch (na->family) {
	case AG_NET_INET4:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)sa;
			sin->sin_family = AF_INET;
			sin->sin_port = AG_SwapBE16(na->port);
			sin->sin_addr.s_addr = na->na_inet4.addr;
			*saLen = sizeof(struct sockaddr_in);
		}
		break;
	case AG_NET_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
			sin6->sin6_family = AF_INET6;
			sin6->sin6_port = AG_SwapBE16(na->port);
			memcpy(&sin6->sin6_addr, na->na_inet6.addr, 16);
			*saLen = sizeof(struct sockaddr_in6);
		}
		break;
	}
}
		
/* Convert a struct sockaddr to AG_NetAddr. */
static AG_NetAddr *_Nullable
SockAddrToNetAddr(enum ag_net_addr_family af, const void *_Nonnull sa)
{
	AG_NetAddr *na;

	switch (af) {
	case AG_NET_INET4:
		{
			const struct sockaddr_in *sin;

			if ((na = AG_NetAddrNew()) == NULL) {
				return (NULL);
			}
			sin = (const struct sockaddr_in *)sa;
			na->family = AG_NET_INET4;
			na->port = AG_SwapBE16(sin->sin_port);
			na->na_inet4.addr = sin->sin_addr.s_addr;
			return (na);
		}
		break;
	case AG_NET_INET6:
		{
			const struct sockaddr_in6 *sin6;

			if ((na = AG_NetAddrNew()) == NULL) {
				return (NULL);
			}
			sin6 = (const struct sockaddr_in6 *)sa;
			na->family = AG_NET_INET6;
			na->port = AG_SwapBE16(sin6->sin6_port);
			memcpy(na->na_inet6.addr, &sin6->sin6_addr, 16);
			return (na);
		}
		break;
	}
	AG_SetError("Bad address family");
	return (NULL);
}

/* Convert a 32-bit millisecond value to a timeval. */
static void
GetTimeval(struct timeval *_Nonnull tv, Uint32 ms)
{
	tv->tv_sec = (ms/1000);
	tv->tv_usec = (ms % 1000)*1000;
	while (tv->tv_usec > 1000000) {
		tv->tv_usec -= 1000000;
		tv->tv_sec += 1;
	}
}

static int
Init(void)
{
	WORD verPref = MAKEWORD(2,2);
	WSADATA wsaData;

	if (AG_MutexTryInit(&agNetWin32Lock) == -1) {
		return (-1);
	}
	if (WSAStartup(verPref, &wsaData) != 0) {
		AG_SetError("Winsock 2.2 initialization failed");
		return (-1);
	}
	return (0);
}

static void
Destroy(void)
{
	if (WSACleanup() == SOCKET_ERROR &&
	    WSAGetLastError() == WSAEINPROGRESS) {
#ifndef _WIN32_WCE
		WSACancelBlockingCall();
#endif
		WSACleanup();
	}
	AG_MutexDestroy(&agNetWin32Lock);
}

static int
GetIfConfig(AG_NetAddrList *_Nonnull nal)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter;
	PIP_ADDR_STRING pAddress;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	
	AG_NetAddrListClear(nal);

	if ((pAdapterInfo = TryMalloc(sizeof(IP_ADAPTER_INFO))) == NULL)
		return (-1);

	AG_MutexLock(&agNetWin32Lock);

	dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
		PIP_ADAPTER_INFO pAdapterInfoNew;
		if ((pAdapterInfoNew = TryRealloc(pAdapterInfo, ulOutBufLen))
		    == NULL) {
			free(pAdapterInfo);
			goto fail;
		}
		pAdapterInfo = pAdapterInfoNew;
		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}
	if (dwRetVal != NO_ERROR) {
		AG_SetError("GetAdaptersInfo() failed");
		goto fail;
	}
	for (pAdapter = pAdapterInfo;
	     pAdapter != NULL;
	     pAdapter = pAdapter->Next) {
		for (pAddress = &pAdapterInfo->IpAddressList;
		     pAddress != NULL;
		     pAddress = pAddress->Next) {
			AG_NetAddr *na;

			if ((na = AG_NetAddrNew()) == NULL) {
				AG_NetAddrListClear(nal);
				goto fail;
			}
			na->family = AG_NET_INET4;
			na->port = 0;
			na->na_inet4.addr = inet_addr(pAddress->IpAddress.String);
			TAILQ_INSERT_TAIL(nal, na, addrs);
		}
	}

	AG_MutexUnlock(&agNetWin32Lock);
	free(pAdapterInfo);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
Resolve(AG_NetAddrList *_Nonnull nal, const char *_Nonnull host,
    const char *_Nonnull port, Uint flags)
{
	struct addrinfo hints, *res, *res0;
	enum ag_net_addr_family af;
	AG_NetAddr *na;
	int rv;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (flags & AG_NET_NUMERIC_HOST) { hints.ai_flags |= AI_NUMERICHOST; }

	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		AG_SetError("%s:%s: %s", host, port, gai_strerror(rv));
		return (-1);
	}
	for (res = res0; res != NULL; res = res->ai_next) {
		if ((af = GetAddrFamily(res->ai_family)) == 0) {
			continue;
		}
		if ((na = SockAddrToNetAddr(af, res->ai_addr)) != NULL)
			TAILQ_INSERT_TAIL(nal, na, addrs);
	}
	freeaddrinfo(res0);
	return (0);
}

static char *_Nullable
GetAddrNumerical(AG_NetAddr *_Nonnull na)
{
	struct sockaddr_storage sa;
	socklen_t saLen;
	
	Free(na->sNum);
	if ((na->sNum = TryMalloc(NI_MAXHOST)) == NULL) {
		return (NULL);
	}
	
	NetAddrToSockAddr(na, &sa, &saLen);
	if (getnameinfo((struct sockaddr *)&sa, saLen,
	    na->sNum, NI_MAXHOST, NULL, 0,
	    NI_NUMERICHOST) != 0) {
		AG_SetError("inet_ntop: %s", strerror(errno));
		return (NULL);
	}
	return (na->sNum);
}

static int
InitSocket(AG_NetSocket *_Nonnull ns)
{
	int sockDomain, sockType;

	if (ns->family == 0) {
		/* Defer actual socket creation until connect() or bind() */
		return (0);
	}
	switch (ns->family) {
	case AG_NET_INET4:	sockDomain = PF_INET;	break;
	case AG_NET_INET6:	sockDomain = PF_INET6;	break;
	default:
		AG_SetError("Bad address family: %d", ns->family);
		return (-1);
	}
	switch (ns->type) {
	case AG_NET_STREAM:	sockType = SOCK_STREAM;	break;
	case AG_NET_DGRAM:	sockType = SOCK_DGRAM;	break;
	default:
		AG_SetError("Bad socket type: %d", ns->type);
		return (-1);
	}
	if ((ns->fd = socket(sockDomain, sockType, ns->proto)) == INVALID_SOCKET) {
		AG_SetError("socket: %s", strerror(errno));
		return (-1);
	}
	return (0);
}

static void
DestroySocket(AG_NetSocket *_Nonnull ns)
{
	if (ns->fd != -1) {
		closesocket(ns->fd);
		ns->fd = -1;
	}
}

static int
Connect(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	struct sockaddr_storage sa;
	socklen_t saLen = 0;
	
	AG_MutexLock(&agNetWin32Lock);
	if (ns->family == 0) {			/* Inherit from address */
		ns->family = na->family;
		if (InitSocket(ns) == -1)
			goto fail;
	}
	NetAddrToSockAddr(na, &sa, &saLen);
	if (connect(ns->fd, (struct sockaddr *)&sa, saLen) < 0) {
		AG_SetError("connect: %s", strerror(errno));
		goto fail;
	}
	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
Bind(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	struct sockaddr_storage sa;
	socklen_t saLen = 0;

	AG_MutexLock(&agNetWin32Lock);
	if (ns->family == 0) {			/* Inherit from address */
		ns->family = na->family;
		if (InitSocket(ns) == -1)
			goto fail;
	}
	NetAddrToSockAddr(na, &sa, &saLen);
	if (bind(ns->fd, (struct sockaddr *)&sa, saLen) == SOCKET_ERROR) {
		AG_SetError("bind: %s", strerror(errno));
		goto fail;
	}
	if (ns->type == AG_NET_STREAM ||
	    ns->type == AG_NET_SEQPACKET) {
		if (listen(ns->fd, ns->listenBacklog) == -1)
			goto fail;
	}
	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
GetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    void *_Nonnull p)
{
	socklen_t optLen;
	int rv = 0;
	
	AG_MutexLock(&agNetWin32Lock);

	switch (so) {
	case AG_NET_BACKLOG:
		*(int *)p = ns->listenBacklog;
		break;
	case AG_NET_DEBUG:
	case AG_NET_REUSEADDR:
	case AG_NET_KEEPALIVE:
	case AG_NET_DONTROUTE:
	case AG_NET_BROADCAST:
	case AG_NET_SNDBUF:
	case AG_NET_RCVBUF:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
		    p, &optLen);
		break;
	case AG_NET_OOBINLINE:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, SO_OOBINLINE, p, &optLen);
		break;
	case AG_NET_LINGER:
		{
			struct linger ling;

			optLen = sizeof(struct linger);
			rv = getsockopt(ns->fd, SOL_SOCKET, SO_LINGER,
			    (char *)&ling, &optLen);
			if (rv == 0) {
				*(int *)p = ling.l_onoff ? ling.l_linger : 0;
			}
		}
		break;
	default:
		AG_SetError("Bad socket option");
		goto fail;
	}
	if (rv == SOCKET_ERROR) {
		AG_SetError("getsockopt(%u): %s", (Uint)so, strerror(errno));
		goto fail;
	}
	
	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
SetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    const void *_Nonnull p)
{
	int rv = 0;
	
	AG_MutexLock(&agNetWin32Lock);

	switch (so) {
	case AG_NET_BACKLOG:
		ns->listenBacklog = *(const int *)p;
		break;
	case AG_NET_DEBUG:
	case AG_NET_REUSEADDR:
	case AG_NET_KEEPALIVE:
	case AG_NET_DONTROUTE:
	case AG_NET_BROADCAST:
	case AG_NET_SNDBUF:
	case AG_NET_RCVBUF:
		rv = setsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
		    p, sizeof(int));
		break;
	case AG_NET_OOBINLINE:
		rv = setsockopt(ns->fd, SOL_SOCKET, SO_OOBINLINE, p, sizeof(int));
		break;
	case AG_NET_LINGER:
		{
			int val = *(const int *)p;
			struct linger ling;

			ling.l_onoff = (val > 0);
			ling.l_linger = val;
			rv = setsockopt(ns->fd, SOL_SOCKET, SO_LINGER,
			    (const char *)&ling, sizeof(struct linger));
		}
		break;
	default:
		AG_SetError("Bad socket option");
		goto fail;
	}
	if (rv != SOCKET_ERROR) {
		AG_SetError("setsockopt(%u): %s", (Uint)so, strerror(errno));
		goto fail;
	}

	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
Read(AG_NetSocket *_Nonnull ns, void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nRead)
{
	int rv;

	AG_MutexLock(&agNetWin32Lock);

	rv = recv(ns->fd, (char *)p, size, 0);
	if (rv == SOCKET_ERROR) {
		AG_SetError("recv: failed (%d)", rv);
		goto fail;
	}
	if (nRead != NULL) { *nRead = (AG_Size)rv; }

	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static int
Write(AG_NetSocket *_Nonnull ns, const void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nWrote)
{
	int rv;

	AG_MutexLock(&agNetWin32Lock);

	rv = send(ns->fd, (const char *)p, size, 0);
	if (rv == SOCKET_ERROR) {
		AG_SetError("send: failed (%d)", rv);
		goto fail;
	}
	if (nWrote != NULL) { *nWrote = (AG_Size)rv; }

	AG_MutexUnlock(&agNetWin32Lock);
	return (0);
fail:
	AG_MutexUnlock(&agNetWin32Lock);
	return (-1);
}

static void
Close(AG_NetSocket *_Nonnull ns)
{
	AG_MutexLock(&agNetWin32Lock);

	if (ns->fd != -1) {
		closesocket(ns->fd);
		ns->fd = -1;
	}
	if (InitSocket(ns) == -1)
		AG_FatalError(NULL);
	
	AG_MutexUnlock(&agNetWin32Lock);
}

static int
Poll(AG_NetSocketSet *_Nonnull nsInput, AG_NetSocketSet *_Nullable nsRead,
    AG_NetSocketSet *_Nullable nsWrite, AG_NetSocketSet *_Nullable nsExcept,
    Uint32 timeout)
{
	fd_set readFds, writeFds, exceptFds;
	AG_NetSocket *ns;
	struct timeval tv;
	int maxfd = 0, count;
	long rv;

	TAILQ_FOREACH(ns, nsInput, sockets)
		if (ns->poll != 0 && ns->fd > maxfd)
			maxfd = ns->fd;
poll:
	if (nsRead) { TAILQ_INIT(nsRead); FD_ZERO(&readFds); }
	if (nsWrite) { TAILQ_INIT(nsWrite); FD_ZERO(&writeFds); }
	if (nsExcept) { TAILQ_INIT(nsExcept); FD_ZERO(&exceptFds); }

	TAILQ_FOREACH(ns, nsInput, sockets) {
		if (nsRead && (ns->poll & AG_NET_POLL_READ))
			FD_SET(ns->fd, &readFds);
		if (nsWrite && (ns->poll & AG_NET_POLL_WRITE))
			FD_SET(ns->fd, &writeFds);
		if (nsExcept && (ns->poll & AG_NET_POLL_EXCEPTIONS))
			FD_SET(ns->fd, &exceptFds);
	}
	if (timeout != 0) {
		GetTimeval(&tv, timeout);
	}

	AG_MutexLock(&agNetWin32Lock);

	rv = select(maxfd+1,
	    nsRead ? &readFds : NULL,
	    nsWrite ? &writeFds : NULL,
	    nsExcept ? &exceptFds : NULL,
	    (timeout != 0) ? &tv : NULL);
	if (rv == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEINTR) {
			AG_MutexUnlock(&agNetWin32Lock);
			goto poll;
		}
		AG_SetError("select: failed (%d)", WSAGetLastError());
		count = -1;
		goto out;
	} else if (rv == 0) {
		count = 0;
		goto out;
	}

	count = 0;
	TAILQ_FOREACH(ns, nsInput, sockets) {
		if (nsRead && FD_ISSET(ns->fd, &readFds)) {
			TAILQ_INSERT_TAIL(nsRead, ns, read);
			count++;
		}
		if (nsWrite && FD_ISSET(ns->fd, &writeFds)) {
			TAILQ_INSERT_TAIL(nsWrite, ns, write);
			count++;
		}
		if (nsExcept && FD_ISSET(ns->fd, &exceptFds)) {
			TAILQ_INSERT_TAIL(nsExcept, ns, except);
			count++;
		}
	}
out:
	AG_MutexUnlock(&agNetWin32Lock);
	return (count);
}

static AG_NetSocket *_Nullable
Accept(AG_NetSocket *_Nonnull ns)
{
	struct sockaddr_storage sa;
	AG_NetSocket *nsNew;
	socklen_t saLen = sizeof(struct sockaddr_storage);
	SOCKET sock;

	AG_MutexLock(&agNetWin32Lock);

	memset(&sa, 0, saLen);
	if ((sock = accept(ns->fd, (struct sockaddr *)&sa, &saLen)) == INVALID_SOCKET) {
		AG_SetError("accept: %s", strerror(errno));
		goto fail;
	}
	if ((nsNew = AG_NetSocketNew(0, ns->type, ns->proto)) == NULL) {
		goto fail;
	}
	if ((nsNew->addrRemote = SockAddrToNetAddr(ns->family, &sa)) == NULL) {
		AG_NetSocketFree(nsNew);
		goto fail;
	}
	nsNew->family = ns->family;
	nsNew->fd = (int)sock;
	nsNew->flags |= AG_NET_SOCKET_CONNECTED;

	AG_MutexUnlock(&agNetWin32Lock);
	return (nsNew);
fail:
	if (sock != -1) { closesocket(sock); }
	AG_MutexUnlock(&agNetWin32Lock);
	return (NULL);
}

const AG_NetOps agNetOps_winsock2 = {
	"winsock2",
	Init,
	Destroy,
	GetIfConfig,
	Resolve,
	GetAddrNumerical,
	InitSocket,
	DestroySocket,
	Connect,
	Bind,
	GetOption,
	SetOption,
	Poll,
	Accept,
	Read,
	Write,
	Close
};
