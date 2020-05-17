/*	Public domain	*/

/*
 * Dummy network interface driver.
 */

#include <agar/core/core.h>
#include <agar/net/net.h>

/* List local network interfaces */
static int
GetIfConfig(AG_NetAddrList *_Nonnull nal)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/* Resolve a host:port into a list of addresses. */
static int
Resolve(AG_NetAddrList *_Nonnull nal, const char *_Nonnull host,
    const char *_Nonnull port, Uint flags)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/* Convert an address from network to presentation format. */
static char *_Nullable
GetAddrNumerical(AG_NetAddr *_Nonnull na)
{
	AG_SetErrorS(_("Operation not supported"));
	return (NULL);
}

/* Establish a connection to a host (from a list of addresses to try). */
static int
Connect(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/* Assign local protocol address to a socket. */
static int
Bind(AG_NetSocket *_Nonnull ns, const AG_NetAddr *_Nonnull na)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/* Return the effective value of a socket option. */
static int
GetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    void *_Nonnull p)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/* Set the value of a socket option. */
static int
SetOption(AG_NetSocket *_Nonnull ns, enum ag_net_socket_option so,
    const void *_Nonnull p)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

/*
 * Block the execution of the current thread until one or more event is
 * reported on the socket(s) in nsInput. Return the results into the
 * nsRead, nsWrite and nsExcept.
 */
static int
Poll(AG_NetSocketSet *_Nonnull nsInput, AG_NetSocketSet *_Nullable nsRead,
    AG_NetSocketSet *_Nullable nsWrite, AG_NetSocketSet *_Nullable nsExcept,
    Uint32 timeout)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

static AG_NetSocket *_Nullable
Accept(AG_NetSocket *_Nonnull ns)
{
	AG_SetErrorS(_("Operation not supported"));
	return (NULL);
}

static int
Read(AG_NetSocket *_Nonnull ns, void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nRead)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

static int
Write(AG_NetSocket *_Nonnull ns, const void *_Nonnull p, AG_Size size,
    AG_Size *_Nullable nRead)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

static void
Close(AG_NetSocket *_Nonnull ns)
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
