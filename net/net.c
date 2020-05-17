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
 * Network access interface.
 */

#include <agar/core/core.h>
#include <agar/net/net.h>

#include <agar/config/have_getaddrinfo.h>
#include <agar/config/have_winsock1.h>
#include <agar/config/have_winsock2.h>

const char *agNetAddrFamilyNames[] = {
	"none",
	"local",
	"inet4",
	"inet6"
};
const char *agNetSocketTypeNames[] = {
	"none",
	"stream",
	"dgram",
	"raw",
	"rdm",
	"seqpacket"
};

const AG_NetOps *agNetOps = NULL;

/* Create a new, unconnected socket. */
AG_NetSocket *
AG_NetSocketNew(enum ag_net_addr_family af, enum ag_net_socket_type type,
    int proto)
{
	AG_NetSocket *ns;

	if ((ns = TryMalloc(sizeof(AG_NetSocket))) == NULL) {
		return (NULL);
	}
	ns->family = af;
	ns->type = type;
	AG_MutexInitRecursive(&ns->lock);
	ns->proto = proto;
	ns->flags = 0;
	ns->poll = 0;
	ns->addrLocal = NULL;
	ns->addrRemote = NULL;
	ns->fd = -1;
	ns->listenBacklog = 10;
	ns->p = NULL;

	if (agNetOps->initSocket != NULL &&
	    agNetOps->initSocket(ns) == -1) {
		free(ns);
		return (NULL);
	}
	return (ns);
}

void
AG_NetSocketFree(AG_NetSocket *ns)
{
	if (agNetOps->destroySocket != NULL) {
		agNetOps->destroySocket(ns);
	}
	if (ns->addrLocal != NULL) { AG_NetAddrFree(ns->addrLocal); }
	if (ns->addrRemote != NULL) { AG_NetAddrFree(ns->addrRemote); }
	AG_MutexDestroy(&ns->lock);
	free(ns);
}

/* Allocate a new socket set. */
void
AG_NetSocketSetInit(AG_NetSocketSet *nss)
{
	TAILQ_INIT(nss);
}

/* Clear a network socket set. */
void
AG_NetSocketSetClear(AG_NetSocketSet *nss)
{
	AG_NetSocket *ns, *nsNext;

	for (ns = TAILQ_FIRST(nss);
	     ns != TAILQ_END(nss);
	     ns = nsNext) {
		nsNext = TAILQ_NEXT(ns, sockets);
		AG_NetSocketFree(ns);
	}
	TAILQ_INIT(nss);
}

/* Allocate a new network address list. */
AG_NetAddrList *
AG_NetAddrListNew(void)
{
	AG_NetAddrList *nal;

	if ((nal = TryMalloc(sizeof(AG_NetAddrList))) == NULL) {
		return (NULL);
	}
	TAILQ_INIT(nal);
	return (nal);
}

/* Clear a network address list. */
void
AG_NetAddrListClear(AG_NetAddrList *nal)
{
	AG_NetAddr *addr, *addrNext;

	for (addr = TAILQ_FIRST(nal);
	     addr != TAILQ_END(nal);
	     addr = addrNext) {
		addrNext = TAILQ_NEXT(addr, addrs);
		AG_NetAddrFree(addr);
	}
	TAILQ_INIT(nal);
}

/* Free a network address list. */
void
AG_NetAddrListFree(AG_NetAddrList *nal)
{
	AG_NetAddrListClear(nal);
	free(nal);
}

/* Allocate a new network address. */
AG_NetAddr *
AG_NetAddrNew(void)
{
	AG_NetAddr *addr;

	if ((addr = TryMalloc(sizeof(AG_NetAddr))) == NULL) {
		return (NULL);
	}
	addr->family = AG_NET_AF_NONE;
	addr->port = 0;
	addr->sNum = NULL;
	addr->sName = NULL;
	return (addr);
}

void
AG_NetAddrFree(AG_NetAddr *na)
{
	if (na->family == AG_NET_LOCAL) {
		Free(na->na_local.path);
	}
	Free(na->sNum);
	Free(na->sName);
	free(na);
}

/* Duplicate a network address. */
AG_NetAddr *
AG_NetAddrDup(const AG_NetAddr *na)
{
	AG_NetAddr *naDup;

	if ((naDup = AG_NetAddrNew()) == NULL) {
		return (NULL);
	}
	naDup->family = na->family;
	naDup->port = na->port;
	switch (na->family) {
	case AG_NET_LOCAL:
		if (na->na_local.path != NULL) {
			naDup->na_local.path = TryStrdup(na->na_local.path);
			if (naDup->na_local.path == NULL)
				goto fail;
		} else {
			naDup->na_local.path = NULL;
		}
		break;
	case AG_NET_INET4:
		naDup->na_inet4.addr = na->na_inet4.addr;
		break;
	case AG_NET_INET6:
		memcpy(naDup->na_inet6.addr, na->na_inet6.addr, sizeof(naDup->na_inet6.addr));
		break;
	default:
		break;
	}
	if (na->sNum != NULL) { naDup->sNum = TryStrdup(na->sNum); }
	if (na->sName != NULL) { naDup->sName = TryStrdup(na->sName); }
	return (naDup);
fail:
	free(naDup);
	return (NULL);
}

/* Compare two network addresses. */
int
AG_NetAddrCompare(const AG_NetAddr *a, const AG_NetAddr *b)
{
	int diff;

	if (a->family != b->family) {
		return (b->family - a->family);
	}
	if (a->port != b->port) {
		return (b->port - a->port);
	}
	switch (a->family) {
	case AG_NET_LOCAL:
		return strcmp(b->na_local.path, a->na_local.path);
	case AG_NET_INET4:
		diff = (b->na_inet4.addr - a->na_inet4.addr);
		if (diff != 0) { return (diff); }
	case AG_NET_INET6:
		diff = (b->na_inet6.addr - a->na_inet6.addr);
		if (diff != 0) { return (diff); }
	default:
		break;
	}
	return (0);
}

/* Test if the given address represents "any" address. */
int
AG_NetAddrIsAny(const AG_NetAddr *na)
{
	int i;

	switch (na->family) {
	case AG_NET_INET4:
		return (na->na_inet4.addr == 0x00000000);
	case AG_NET_INET6:
		for (i = 0; i < sizeof(na->na_inet6.addr); i++) {
			if (na->na_inet6.addr[i] != 0)
				return (0);
		}
		return (1);
	default:
		return (0);
	}
}

/*
 * Return a numerical representation of the given network address.
 * The string is allocated and freed internally with the AG_NetAddr.
 */
const char *
AG_NetAddrNumerical(AG_NetAddr *na)
{
	return agNetOps->getAddrNumerical(na);
}

/* Resolve the specified hostname and port name/number. */
AG_NetAddrList *
AG_NetResolve(const char *host, const char *port, Uint flags)
{
	AG_NetAddrList *nal;

	if ((nal = AG_NetAddrListNew()) == NULL) {
		return (NULL);
	}
	if (agNetOps->resolve(nal, host, port, flags) == -1) {
		goto fail;
	}
	return (nal);
fail:
	AG_NetAddrListFree(nal);
	return (NULL);
}

/* Return the list of addresses associated with local network interfaces. */
AG_NetAddrList *
AG_NetGetIfConfig(void)
{
	AG_NetAddrList *nal;
	
	if ((nal = AG_NetAddrListNew()) == NULL) {
		return (NULL);
	}
	if (agNetOps->getIfConfig(nal) == -1) {
		goto fail;
	}
	return (nal);
fail:
	AG_NetAddrListFree(nal);
	return (NULL);
}

/* Establish a connection to one of the network addresses specified. */
int
AG_NetConnect(AG_NetSocket *ns, const AG_NetAddrList *nal)
{
	AG_NetAddr *na;

	AG_MutexLock(&ns->lock);

	if (ns->flags & AG_NET_SOCKET_BOUND) {
		AG_SetError(_("Socket is already bound"));
		goto fail;
	}
	if (ns->flags & AG_NET_SOCKET_CONNECTED) {
		AG_SetError(_("Socket is already connected"));
		goto fail;
	}
	TAILQ_FOREACH(na, nal, addrs) {
		if (agNetOps->connect(ns, na) == 0)
			break;
	}
	if (na == NULL) {
		AG_SetError(_("Connection failed"));
		goto fail;
	}
	ns->flags |= AG_NET_SOCKET_CONNECTED;
	ns->addrRemote = AG_NetAddrDup(na);

	AG_MutexUnlock(&ns->lock);
	return (0);
fail:
	AG_MutexUnlock(&ns->lock);
	return (-1);
}

/*
 * Assign a local address to a socket. If the address happens to be
 * of AG_NET_LOCAL family, a Unix socket is created in the filesystem.
 */
int
AG_NetBind(AG_NetSocket *ns, const AG_NetAddr *na)
{
	AG_MutexLock(&ns->lock);

	if (ns->flags & AG_NET_SOCKET_CONNECTED) {
		AG_SetError(_("Socket is already connected"));
		goto fail;
	}
	if (ns->flags & AG_NET_SOCKET_BOUND) {
		AG_SetError(_("Socket is already bound"));
		goto fail;
	}
	if (agNetOps->bind(ns, na) != 0) {
		goto fail;
	}
	ns->flags |= AG_NET_SOCKET_BOUND;
	ns->addrLocal = AG_NetAddrDup(na);

	AG_MutexUnlock(&ns->lock);
	return (0);
fail:
	AG_MutexUnlock(&ns->lock);
	return (-1);
}

/* Retrieve a socket option. */
int
AG_NetGetOption(AG_NetSocket *ns, enum ag_net_socket_option opt, void *arg)
{
	int rv;
	
	AG_MutexLock(&ns->lock);
	rv = agNetOps->getOption(ns, opt, arg);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Retrieve a socket option (integer). */
int
AG_NetGetOptionInt(AG_NetSocket *ns, enum ag_net_socket_option opt, int *arg)
{
	int rv;
	
	AG_MutexLock(&ns->lock);
	rv = agNetOps->getOption(ns, opt, arg);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Set a socket option. */
int
AG_NetSetOption(AG_NetSocket *ns, enum ag_net_socket_option opt, const void *arg)
{
	int rv;
	
	AG_MutexLock(&ns->lock);
	rv = agNetOps->setOption(ns, opt, arg);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Set a socket option (integer). */
int
AG_NetSetOptionInt(AG_NetSocket *ns, enum ag_net_socket_option opt, int val)
{
	int rv;
	
	AG_MutexLock(&ns->lock);
	rv = agNetOps->setOption(ns, opt, &val);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Poll a set of sockets for read/write/exception events. */
int
AG_NetPoll(AG_NetSocketSet *nsInput, AG_NetSocketSet *nsRead,
    AG_NetSocketSet *nsWrite, AG_NetSocketSet *nsExcept, Uint32 timeout)
{
	return agNetOps->poll(nsInput, nsRead, nsWrite, nsExcept, timeout);
}

/* Accept a connection on a bound socket. */
AG_NetSocket *
AG_NetAccept(AG_NetSocket *ns)
{
	AG_NetSocket *nsNew;

	AG_MutexLock(&ns->lock);
	nsNew = agNetOps->accept(ns);
	AG_MutexUnlock(&ns->lock);
	return (nsNew);
}

/* Read data from a socket. */
int
AG_NetRead(AG_NetSocket *ns, void *p, AG_Size size, AG_Size *nRead)
{
	int rv;

	AG_MutexLock(&ns->lock);
	rv = agNetOps->read(ns, p, size, nRead);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Write data to a socket. */
int
AG_NetWrite(AG_NetSocket *ns, const void *p, AG_Size size, AG_Size *nWrote)
{
	int rv;

	AG_MutexLock(&ns->lock);
	rv = agNetOps->write(ns, p, size, nWrote);
	AG_MutexUnlock(&ns->lock);
	return (rv);
}

/* Close a connection on a socket. */
void
AG_NetClose(AG_NetSocket *ns)
{
	AG_MutexLock(&ns->lock);

	if ((ns->flags & AG_NET_SOCKET_CONNECTED) == 0) {
		goto out;
	}
	agNetOps->close(ns);

	if (ns->addrLocal != NULL) {
		AG_NetAddrFree(ns->addrLocal);
		ns->addrLocal = NULL;
	}
	if (ns->addrRemote != NULL) {
		AG_NetAddrFree(ns->addrRemote);
		ns->addrRemote = NULL;
	}
	ns->flags &= ~(AG_NET_SOCKET_CONNECTED);
out:
	AG_MutexUnlock(&ns->lock);
}

int
AG_InitNetworkSubsystem(const AG_NetOps *ops)
{
	if (ops == NULL) {
#if defined(HAVE_WINSOCK2)
		ops = &agNetOps_winsock2;
#elif defined(HAVE_WINSOCK1)
		ops = &agNetOps_winsock1;
#elif defined(HAVE_GETADDRINFO)
		ops = &agNetOps_bsd;
#else
		ops = &agNetOps_dummy;
#endif
	}
	if (agNetOps == ops) {
		return (0);
	}
	if (agNetOps != NULL && agNetOps->destroy != NULL) {
		agNetOps->destroy();
	}
	agNetOps = ops;
	return (ops->init != NULL) ? ops->init() : 0;
}

void
AG_DestroyNetworkSubsystem(void)
{
	if (agNetOps != NULL && agNetOps->destroy != NULL) {
		agNetOps->destroy();
	}
	agNetOps = NULL;
}
