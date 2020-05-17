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
 * Network access via standard BSD sockets.
 */

#ifdef __APPLE__
# ifndef _DARWIN_C_SOURCE
# define _DARWIN_C_SOURCE		/* For SIOCGIFCONF */
# endif
#endif
#ifdef __NetBSD__
# ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE
# endif
#endif

#include <agar/core/core.h>
#include <agar/net/net.h>

#include <agar/core/queue_close.h>	/* Avoid <sys/queue.h> conflicts */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>

#include <agar/core/queue_close.h>	/* Avoid <sys/queue.h> conflicts */
#include <agar/core/queue.h>

#include <agar/config/have_select.h>
#include <agar/config/have_siocgifconf.h>
#include <agar/config/have_setsockopt.h>
#ifdef HAVE_SETSOCKOPT
#include <agar/config/have_so_oobinline.h>
#include <agar/config/have_so_reuseport.h>
#include <agar/config/have_so_timestamp.h>
#include <agar/config/have_so_nosigpipe.h>
#include <agar/config/have_so_linger.h>
#include <agar/config/have_so_acceptfilter.h>

static int agSockOptionMap[] = {
	0, SO_DEBUG, SO_REUSEADDR, SO_KEEPALIVE, SO_DONTROUTE, SO_BROADCAST,
	SO_SNDBUF, SO_RCVBUF, SO_SNDLOWAT, SO_RCVLOWAT, SO_SNDTIMEO, SO_RCVTIMEO
};
#endif /* HAVE_SETSOCKOPT */

/* Convert an AF_* value to an enum ag_net_addr_family. */
static __inline__ enum ag_net_addr_family _Const_Attribute
GetAddrFamily(int family)
{
	switch (family) {
	case AF_INET:	return (AG_NET_INET4);
#ifdef AF_INET6
	case AF_INET6:	return (AG_NET_INET6);
#endif
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
#ifdef AF_INET6
	case AG_NET_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
			sin6->sin6_family = AF_INET6;
			sin6->sin6_port = AG_SwapBE16(na->port);
			memcpy(&sin6->sin6_addr, na->na_inet6.addr, 16);
			*saLen = sizeof(struct sockaddr_in6);
		}
		break;
#endif
	case AG_NET_LOCAL:
		{
			struct sockaddr_un *sun = (struct sockaddr_un *)sa;
			sun->sun_family = AF_UNIX;
			Strlcpy(sun->sun_path, na->na_local.path, sizeof(sun->sun_path));
			*saLen = sizeof(struct sockaddr_un);
		}
		break;
	default:
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
#ifdef AF_INET6
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
#endif /* AF_INET6 */
	default:
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
	void (*fn)(int);
	struct sockaddr_in sin;
#ifdef AF_INET6
	struct sockaddr_in6 sin6;
#endif
	if ((fn = signal(SIGPIPE, SIG_IGN)) != SIG_DFL)
		signal(SIGPIPE, fn);

	/* Sanity check the sockaddr lengths. */
	if (sizeof(sin.sin_addr.s_addr) != 4) {
		AG_SetError("Bad sockaddr_in size");
		return (-1);
	}
#ifdef AF_INET6
	if (sizeof(sin6.sin6_addr) != 16) {
		AG_SetError("Bad sockaddr_in6 size");
		return (-1);
	}
#endif
	return (0);
}

static void
Destroy(void)
{
	void (*fn)(int);

	if ((fn = signal(SIGPIPE, SIG_DFL)) != SIG_IGN)
		signal(SIGPIPE, fn);
}

static int
GetIfConfig(AG_NetAddrList *_Nonnull nal)
{
#ifdef HAVE_SIOCGIFCONF
	enum ag_net_addr_family af;
	AG_NetAddr *na;
	char buf[4096];
	struct ifconf conf;
	struct ifreq *ifr;
	int sock;

	AG_NetAddrListClear(nal);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		AG_SetError("socket: %s", strerror(errno));
		return (-1);
	}
	conf.ifc_len = sizeof(buf);
	conf.ifc_buf = (caddr_t)buf;
	if (ioctl(sock, SIOCGIFCONF, &conf) < 0) {
		AG_SetError("SIOCGIFCONF: %s", strerror(errno));
		goto fail;
	}

#if !defined(_SIZEOF_ADDR_IFREQ)
#define _SIZEOF_ADDR_IFREQ sizeof
#endif
	for (ifr = (struct ifreq *)buf;
	     (char *)ifr < &buf[conf.ifc_len];
	     ifr = (struct ifreq *)((char *)ifr + _SIZEOF_ADDR_IFREQ(*ifr))) {
		if ((af = GetAddrFamily(ifr->ifr_addr.sa_family)) == 0) {
			continue;
		}
		if ((na = SockAddrToNetAddr(af, &ifr->ifr_addr)) == NULL) {
			goto fail;
		}
		na->port = 0;
		TAILQ_INSERT_TAIL(nal, na, addrs);
	}
	close(sock);
	return (0);
fail:
	close(sock);
	return (-1);
#else
	AG_SetError("GetIfConfig: SIOCGIFCONF unavailable");
	return (-1);
#endif /* !HAVE_SIOCGIFCONF */
}

static int
Resolve(AG_NetAddrList *_Nonnull nal,
    const char *_Nonnull host,
    const char *_Nonnull port,
    Uint flags)
{
	struct addrinfo hints, *res, *res0;
	enum ag_net_addr_family af;
	AG_NetAddr *na;
	int rv;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
	if (flags & AG_NET_ADDRCONFIG) { hints.ai_flags |= AI_ADDRCONFIG; }
#endif
#ifdef AI_NUMERICHOST
	if (flags & AG_NET_NUMERIC_HOST) { hints.ai_flags |= AI_NUMERICHOST; }
#endif
#ifdef AI_NUMERICSERV
	if (flags & AG_NET_NUMERIC_PORT) { hints.ai_flags |= AI_NUMERICSERV; }
#endif

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
	const char *s;

	switch (na->family) {
	case AG_NET_INET4:
		Free(na->sNum);
		if ((na->sNum = TryMalloc(INET_ADDRSTRLEN)) == NULL) {
			return (NULL);
		}
		s = inet_ntop(AF_INET, &na->na_inet4.addr, na->sNum,
		    INET_ADDRSTRLEN);
		break;
#ifdef AF_INET6
	case AG_NET_INET6:
		Free(na->sNum);
		if ((na->sNum = TryMalloc(INET6_ADDRSTRLEN)) == NULL) {
			return (NULL);
		}
		s = inet_ntop(AF_INET6, &na->na_inet4.addr, na->sNum,
		    INET6_ADDRSTRLEN);
		break;
#endif
	default:
		AG_SetError("Bad address format");
		return (NULL);
	}
	if (s == NULL) {
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
	if ((ns->fd = socket(sockDomain, sockType, ns->proto)) == -1) {
		AG_SetError("socket: %s", strerror(errno));
		return (-1);
	}
	return (0);
}

static void
DestroySocket(AG_NetSocket *_Nonnull ns)
{
	if (ns->fd != -1) {
		close(ns->fd);
		ns->fd = -1;
	}
}

static int
Connect(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	struct sockaddr_storage sa;
	socklen_t saLen = 0;

	if (ns->family == 0) {			/* Inherit from address */
		ns->family = na->family;
		if (InitSocket(ns) == -1)
			return (-1);
	}
	NetAddrToSockAddr(na, &sa, &saLen);
	if (connect(ns->fd, (struct sockaddr *)&sa, saLen) < 0) {
		AG_SetError("connect: %s", strerror(errno));
		return (-1);
	}
	return (0);
}

static int
Bind(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	struct sockaddr_storage sa;
	socklen_t saLen = 0;

	if (ns->family == 0) {			/* Inherit from address */
		ns->family = na->family;
		if (InitSocket(ns) == -1)
			return (-1);
	}
	NetAddrToSockAddr(na, &sa, &saLen);
	if (bind(ns->fd, (struct sockaddr *)&sa, saLen) < 0) {
		AG_SetError("bind: %s", strerror(errno));
		return (-1);
	}
	if (ns->type == AG_NET_STREAM ||
	    ns->type == AG_NET_SEQPACKET) {
		if (listen(ns->fd, ns->listenBacklog) == -1)
			return (-1);
	}
	return (0);
}

static int
GetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    void *_Nonnull p)
{
#ifdef HAVE_SETSOCKOPT
	socklen_t optLen;
	int rv = 0;

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
	case AG_NET_SNDLOWAT:
	case AG_NET_RCVLOWAT:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
		    p, &optLen);
		break;
	case AG_NET_SNDTIMEO:
	case AG_NET_RCVTIMEO:
		{
			struct timeval tv;

			optLen = sizeof(struct timeval);
			rv = getsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
			    &tv, &optLen);
			if (rv == 0) {
				*(Uint32 *)p = tv.tv_sec*1000 + tv.tv_usec/1000;
			}
		}
		break;
#ifdef HAVE_SO_OOBINLINE
	case AG_NET_OOBINLINE:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, SO_OOBINLINE, p, &optLen);
		break;
#endif
#ifdef HAVE_SO_REUSEPORT
	case AG_NET_REUSEPORT:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, SO_REUSEPORT, p, &optLen);
		break;
#endif
#ifdef HAVE_SO_TIMESTAMP
	case AG_NET_TIMESTAMP:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, SO_TIMESTAMP, p, &optLen);
		break;
#endif
#ifdef HAVE_SO_NOSIGPIPE
	case AG_NET_NOSIGPIPE:
		optLen = sizeof(int);
		rv = getsockopt(ns->fd, SOL_SOCKET, SO_NOSIGPIPE, p, &optLen);
		break;
#endif
#ifdef HAVE_SO_LINGER
	case AG_NET_LINGER:
		{
			struct linger ling;

			optLen = sizeof(struct linger);
			rv = getsockopt(ns->fd, SOL_SOCKET, SO_LINGER, &ling, &optLen);
			if (rv == 0) {
				*(int *)p = ling.l_onoff ? ling.l_linger : 0;
			}
		}
		break;
#endif
#ifdef HAVE_SO_ACCEPTFILTER
	case AG_NET_ACCEPTFILTER:
		{
			AG_NetAcceptFilter *naf = (AG_NetAcceptFilter *)p;
			struct accept_filter_arg afa;
			
			optLen = sizeof(struct accept_filter_arg);
			rv = getsockopt(ns->fd, SOL_SOCKET, SO_ACCEPTFILTER,
			    &afa, &optLen);
			if (rv == 0) {
				Strlcpy(naf->name, afa.af_name, sizeof(naf->name));
				Strlcpy(naf->arg, afa.af_arg, sizeof(naf->arg));
			}
		}
		break;
#endif
	default:
		AG_SetError("Bad socket option");
		return (-1);
	}
	if (rv != 0) {
		AG_SetError("getsockopt: %s", strerror(errno));
		return (-1);
	}
	return (0);
#else
	AG_SetError("getsockopt() is not available");
	return (-1);
#endif
}

static int
SetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    const void *_Nonnull p)
{
#ifdef HAVE_SETSOCKOPT
	int rv = 0;

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
	case AG_NET_SNDLOWAT:
	case AG_NET_RCVLOWAT:
		rv = setsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
		    p, sizeof(int));
		break;
	case AG_NET_SNDTIMEO:
	case AG_NET_RCVTIMEO:
		{
			Uint32 Tms = *(const Uint32 *)p;
			struct timeval tv;

			GetTimeval(&tv, Tms);
			rv = setsockopt(ns->fd, SOL_SOCKET, agSockOptionMap[so],
			    &tv, sizeof(struct timeval));
		}
		break;
#ifdef HAVE_SO_OOBINLINE
	case AG_NET_OOBINLINE:
		rv = setsockopt(ns->fd, SOL_SOCKET, SO_OOBINLINE, p, sizeof(int));
		break;
#endif
#ifdef HAVE_SO_REUSEPORT
	case AG_NET_REUSEPORT:
		rv = setsockopt(ns->fd, SOL_SOCKET, SO_REUSEPORT, p, sizeof(int));
		break;
#endif
#ifdef HAVE_SO_TIMESTAMP
	case AG_NET_TIMESTAMP:
		rv = setsockopt(ns->fd, SOL_SOCKET, SO_TIMESTAMP, p, sizeof(int));
		break;
#endif
#ifdef HAVE_SO_NOSIGPIPE
	case AG_NET_NOSIGPIPE:
		rv = setsockopt(ns->fd, SOL_SOCKET, SO_NOSIGPIPE, p, sizeof(int));
		break;
#endif
#ifdef HAVE_SO_LINGER
	case AG_NET_LINGER:
		{
			int val = *(const int *)p;
			struct linger ling;

			ling.l_onoff = (val > 0);
			ling.l_linger = val;
			rv = setsockopt(ns->fd, SOL_SOCKET, SO_LINGER,
			    &ling, sizeof(struct linger));
		}
		break;
#endif
#ifdef HAVE_SO_ACCEPTFILTER
	case AG_NET_ACCEPTFILTER:
		{
			const AG_NetAcceptFilter *naf = (const AG_NetAcceptFilter *)p;
			struct accept_filter_arg afa;

			Strlcpy(afa.af_name, naf->name, sizeof(afa.af_name));
			Strlcpy(afa.af_arg, naf->arg, sizeof(afa.af_arg));
			rv = setsockopt(ns->fd, SOL_SOCKET, SO_ACCEPTFILTER,
			    &afa, sizeof(struct accept_filter_arg));
		}
		break;
#endif
	default:
		AG_SetError("Bad socket option");
		return (-1);
	}
	if (rv != 0) {
		AG_SetError("setsockopt: %s", strerror(errno));
		return (-1);
	}
	return (0);
#else
	AG_SetError("setsockopt() is not available");
	return (-1);
#endif /* !HAVE_SETSOCKOPT */
}

static int
Read(AG_NetSocket *_Nonnull ns, void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nRead)
{
	ssize_t rv;

	rv = read(ns->fd, p, size);
	if (rv < 0) {
		AG_SetError("read: %s", strerror(errno));
		return (-1);
	}
	if (nRead != NULL) { *nRead = (AG_Size)rv; }
	return (0);
}

static int
Write(AG_NetSocket *_Nonnull ns, const void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nWrote)
{
	ssize_t rv;

	rv = write(ns->fd, p, size);
	if (rv < 0) {
		AG_SetError("write: %s", strerror(errno));
		return (-1);
	}
	if (nWrote != NULL) { *nWrote = (AG_Size)rv; }
	return (0);
}

static void
Close(AG_NetSocket *_Nonnull ns)
{
	if (ns->fd != -1) {
		close(ns->fd);
		ns->fd = -1;
	}
	if (InitSocket(ns) == -1)
		AG_FatalError(NULL);
}

static int
Poll(AG_NetSocketSet *_Nonnull nsInput, AG_NetSocketSet *_Nullable nsRead,
    AG_NetSocketSet *_Nullable nsWrite, AG_NetSocketSet *_Nullable nsExcept,
    Uint32 timeout)
{
#ifdef HAVE_SELECT
	fd_set readFds, writeFds, exceptFds;
	AG_NetSocket *ns;
	struct timeval tv;
	int maxfd = 0, rv, count;

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
	rv = select(maxfd+1,
	    nsRead ? &readFds : NULL,
	    nsWrite ? &writeFds : NULL,
	    nsExcept ? &exceptFds : NULL,
	    (timeout != 0) ? &tv : NULL);
	if (rv == -1) {
		if (errno == EINTR) {
			goto poll;
		}
		AG_SetError("select: %s", strerror(errno));
		return (-1);
	} else if (rv == 0) {
		return (0);
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
	return (count);
#else
	AG_SetError("select() is not available");
	return (-1);
#endif /* !HAVE_SELECT */
}

static AG_NetSocket *_Nullable
Accept(AG_NetSocket *_Nonnull ns)
{
	struct sockaddr_storage sa;
	AG_NetSocket *nsNew;
	socklen_t saLen = sizeof(struct sockaddr_storage);
	int sock;

	memset(&sa, 0, saLen);
	if ((sock = accept(ns->fd, (struct sockaddr *)&sa, &saLen)) == -1) {
		AG_SetError("accept: %s", strerror(errno));
		return (NULL);
	}
	if ((nsNew = AG_NetSocketNew(0, ns->type, ns->proto)) == NULL) {
		close(sock);
		return (NULL);
	}
	if ((nsNew->addrRemote = SockAddrToNetAddr(ns->family, &sa)) == NULL) {
		close(sock);
		AG_NetSocketFree(nsNew);
		return (NULL);
	}
	nsNew->family = ns->family;
	nsNew->fd = sock;
	nsNew->flags |= AG_NET_SOCKET_CONNECTED;
	return (nsNew);
}

const AG_NetOps agNetOps_bsd = {
	"bsd",
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
