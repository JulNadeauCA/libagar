/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Dummy network interface driver.
 */

#include <agar/core/core.h>

static int
GetIfConfig(AG_NetAddrList *nal)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
Resolve(AG_NetAddrList *nal, const char *host, const char *port, Uint flags)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static char *
GetAddrNumerical(AG_NetAddr *na)
{
	AG_SetError(_("Operation not supported"));
	return (NULL);
}

static int
Connect(AG_NetSocket *ns, const AG_NetAddr *na)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
Bind(AG_NetSocket *ns, const AG_NetAddr *na)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
GetOption(AG_NetSocket *ns, enum ag_net_socket_option so, void *p)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
SetOption(AG_NetSocket *ns, enum ag_net_socket_option so, const void *p)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
Poll(AG_NetSocketSet *nsInput, AG_NetSocketSet *nsRead, AG_NetSocketSet *nsWrite,
    AG_NetSocketSet *nsExcept, Uint32 timeout)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static AG_NetSocket *
Accept(AG_NetSocket *ns)
{
	AG_SetError(_("Operation not supported"));
	return (NULL);
}

static int
Read(AG_NetSocket *ns, void *p, size_t size, size_t *nRead)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
Write(AG_NetSocket *ns, const void *p, size_t size, size_t *nRead)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static void
Close(AG_NetSocket *ns)
{
}

const AG_NetOps agNetOps_dummy = {
	"dummy",
	NULL,			/* init */
	NULL,			/* destroy */
	GetIfConfig,
	Resolve,
	GetAddrNumerical,
	NULL,			/* initSocket */
	NULL,			/* destroySocket */
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
