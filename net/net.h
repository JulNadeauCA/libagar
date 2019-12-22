/*	Public domain	*/

#include <agar/net/begin.h>

/* Address families */
enum ag_net_addr_family {
	AG_NET_AF_NONE,			/* Undefined */
	AG_NET_LOCAL,			/* Host-local protocols */
	AG_NET_INET4,			/* IPv4 */
	AG_NET_INET6			/* IPv6 */
};

/* Socket types */
enum ag_net_socket_type {
	AG_NET_SOCKET_NONE,		/* Undefined */
	AG_NET_STREAM,			/* Stream socket */
	AG_NET_DGRAM,			/* Datagram socket */
	AG_NET_RAW,			/* Raw-protocol interface */
	AG_NET_RDM,			/* Reliably-delivered packet */
	AG_NET_SEQPACKET		/* Sequenced packet stream */
};

/* Socket options */
enum ag_net_socket_option {
	AG_NET_SO_NONE,
	/*
	 * Standard options
	 */
	AG_NET_DEBUG,		/* Enable debugging on this socket */
	AG_NET_REUSEADDR,	/* Reuse local addresses */
	AG_NET_KEEPALIVE,	/* Keep connections alive */
	AG_NET_DONTROUTE,	/* Routing bypass for outgoing messages */
	AG_NET_BROADCAST,	/* Transmit broadcast messages */
	AG_NET_BINDANY,		/* Allow binding to any address */
	AG_NET_SNDBUF,		/* Buffer size for sending (int) */
	AG_NET_RCVBUF,		/* Buffer size for reception (int) */
	AG_NET_SNDLOWAT,	/* Low watermark for send (int) */
	AG_NET_RCVLOWAT,	/* Low watermark for receive (int) */
	AG_NET_SNDTIMEO,	/* Timeout for output in ms (Uint32) */
	AG_NET_RCVTIMEO,	/* Timeout for input in ms (Uint32) */
	AG_NET_BACKLOG,		/* Limit on incoming connection backlog (int) */
	/*
	 * Platform-dependent options
	 */
	AG_NET_OOBINLINE,	/* Receive OOB data inline */
	AG_NET_REUSEPORT,	/* Allow duplicate address/port bindings */
	AG_NET_TIMESTAMP,	/* Receive datagram timestamps */
	AG_NET_NOSIGPIPE,	/* Disable generation of SIGPIPE */
	AG_NET_LINGER,		/* Linger on close() if data present (seconds) */
	AG_NET_ACCEPTFILTER,	/* Kernel-based accept filter (AG_NetAcceptFilter) */
	AG_NET_LAST
};

/* Argument to AG_NET_ACCEPTFILTER */
typedef struct ag_net_accept_filter {
	char name[16];			/* Filter module name */
	char arg[240];			/* Argument */
} AG_NetAcceptFilter;

/* Resolver options */
#define AG_NET_ADDRCONFIG	0x01	/* Resolve IPv4 vs. IPv6 addresses
					   based on current ifconfig status */
#define AG_NET_NUMERIC_HOST	0x02	/* Host must be numeric */
#define AG_NET_NUMERIC_PORT	0x02	/* Port must be numeric */

/* General socket address (network or host-local) */
typedef struct ag_net_addr {
	enum ag_net_addr_family family;		/* Address family */
	int port;				/* Port number (if any) */
	union {
		struct {
			char *_Nullable path;	/* Unix socket path */
		} local;
		struct {
			Uint32 addr;		/* IPv4 address */
		} inet4;
		struct {
			Uint8  addr[16];	/* IPv6 address */
		} inet6;
	} data;
#ifdef _AGAR_INTERNAL
# define na_local data.local
# define na_inet4 data.inet4
# define na_inet6 data.inet6
#endif
	char *_Nullable sNum;			/* Numerical form */
	char *_Nullable sName;			/* Reverse DNS (UTF-8) */
	AG_TAILQ_ENTRY(ag_net_addr) addrs;
} AG_NetAddr;

/* List of socket addresses. */
typedef AG_TAILQ_HEAD(ag_net_addr_list, ag_net_addr) AG_NetAddrList;

/* Endpoint for communication */
typedef struct ag_net_socket {
	enum ag_net_addr_family family;		/* Address family */
	enum ag_net_socket_type type;		/* Socket type */
	_Nonnull_Mutex AG_Mutex lock;
	int proto;				/* Socket protocol number */
	Uint flags;
#define AG_NET_SOCKET_BOUND	0x01		/* Bound to a local address */
#define AG_NET_SOCKET_CONNECTED	0x02		/* Connection established */
	Uint poll;
#define AG_NET_POLL_READ	0x01		/* Poll read condition */
#define AG_NET_POLL_WRITE	0x02		/* Poll write condition */
#define AG_NET_POLL_EXCEPTIONS	0x04		/* Poll exceptions */
	Uint32 _pad;
	AG_NetAddr *_Nullable addrLocal;	/* Bound local address */
	AG_NetAddr *_Nullable addrRemote;	/* Connected remote address */
	int fd;					/* File descriptor (if any) */
	int listenBacklog;			/* For AG_NET_BACKLOG */
	void *_Nullable p;			/* User pointer */

	AG_TAILQ_ENTRY(ag_net_socket) sockets;
	AG_TAILQ_ENTRY(ag_net_socket) read;	/* Poll read results */
	AG_TAILQ_ENTRY(ag_net_socket) write;	/* Poll write results */
	AG_TAILQ_ENTRY(ag_net_socket) except;	/* Poll exception results */
} AG_NetSocket;

/* List of sockets. */
typedef AG_TAILQ_HEAD(ag_net_socket_set, ag_net_socket) AG_NetSocketSet;

typedef struct ag_net_ops {
	const char *_Nonnull name;

	int  (*_Nullable init)(void);
	void (*_Nullable destroy)(void);

	int  (*_Nonnull getIfConfig)(AG_NetAddrList *_Nonnull);
	int  (*_Nonnull resolve)(AG_NetAddrList *_Nonnull, const char *_Nonnull,
	                         const char *_Nonnull, Uint);

	char *_Nonnull (*_Nonnull getAddrNumerical)(AG_NetAddr *_Nonnull);

	int  (*_Nullable initSocket)(AG_NetSocket *_Nonnull);
	void (*_Nullable destroySocket)(AG_NetSocket *_Nonnull);

	int (*_Nonnull connect)(AG_NetSocket *_Nonnull,
	                        const AG_NetAddr *_Nonnull);
	int (*_Nonnull bind)(AG_NetSocket *_Nonnull,
	                     const AG_NetAddr *_Nonnull);

	int (*_Nonnull getOption)(AG_NetSocket *_Nonnull,
	                          enum ag_net_socket_option, void *_Nonnull);
	int (*_Nonnull setOption)(AG_NetSocket *_Nonnull,
	                          enum ag_net_socket_option, const void *_Nonnull);

	int (*_Nonnull poll)(AG_NetSocketSet *_Nonnull,
	                     AG_NetSocketSet *_Nullable,
	                     AG_NetSocketSet *_Nullable,
	                     AG_NetSocketSet *_Nullable, Uint32);

	AG_NetSocket *_Nullable (*_Nonnull accept)(AG_NetSocket *_Nonnull);

	int (*_Nonnull read)(AG_NetSocket *_Nonnull, void *_Nonnull, AG_Size,
			     AG_Size *_Nullable);
	int (*_Nonnull write)(AG_NetSocket *_Nonnull, const void *_Nonnull, AG_Size,
			      AG_Size *_Nonnull);

	void (*_Nonnull close)(AG_NetSocket *_Nonnull);
} AG_NetOps;

__BEGIN_DECLS
extern const AG_NetOps *_Nullable agNetOps;
extern const AG_NetOps agNetOps_bsd;
extern const AG_NetOps agNetOps_winsock1;
extern const AG_NetOps agNetOps_winsock2;
extern const AG_NetOps agNetOps_dummy;

extern const char *_Nonnull agNetAddrFamilyNames[];
extern const char *_Nonnull agNetSocketTypeNames[];

int  AG_InitNetworkSubsystem(const AG_NetOps *_Nullable);
void AG_DestroyNetworkSubsystem(void);

AG_NetSocket *_Nullable AG_NetSocketNew(enum ag_net_addr_family,
                                        enum ag_net_socket_type, int);

void AG_NetSocketFree(AG_NetSocket *_Nonnull);

void    AG_NetSocketSetInit(AG_NetSocketSet *_Nonnull);
void    AG_NetSocketSetClear(AG_NetSocketSet *_Nonnull);
#define AG_NetSocketSetFree(nss) AG_NetSocketSetClear(nss)

AG_NetAddr *_Nullable AG_NetAddrNew(void); /* _Malloc_Like_Attribute; */
AG_NetAddr *_Nullable AG_NetAddrDup(const AG_NetAddr *_Nonnull);

int  AG_NetAddrCompare(const AG_NetAddr *_Nonnull, const AG_NetAddr *_Nonnull)
                      _Pure_Attribute;
int  AG_NetAddrIsAny(const AG_NetAddr *_Nonnull)
                    _Pure_Attribute;
void AG_NetAddrFree(AG_NetAddr *_Nonnull);

const char *_Nonnull AG_NetAddrNumerical(AG_NetAddr *_Nonnull);

AG_NetAddrList *_Nullable AG_NetAddrListNew(void); /* _Malloc_Like_Attribute */
void                      AG_NetAddrListClear(AG_NetAddrList *_Nonnull);
void                      AG_NetAddrListFree(AG_NetAddrList *_Nonnull);

AG_NetAddrList *_Nullable AG_NetGetIfConfig(void);
AG_NetAddrList *_Nullable AG_NetResolve(const char *_Nonnull,
                                        const char *_Nonnull, Uint);

int AG_NetConnect(AG_NetSocket *_Nonnull, const AG_NetAddrList *_Nonnull);
int AG_NetBind(AG_NetSocket *_Nonnull, const AG_NetAddr *_Nonnull);

int AG_NetGetOption(AG_NetSocket *_Nonnull, enum ag_net_socket_option, void *_Nonnull);
int AG_NetSetOption(AG_NetSocket *_Nonnull, enum ag_net_socket_option, const void *_Nonnull);
int AG_NetGetOptionInt(AG_NetSocket *_Nonnull, enum ag_net_socket_option, int *_Nonnull);
int AG_NetSetOptionInt(AG_NetSocket *_Nonnull, enum ag_net_socket_option, int);

int AG_NetPoll(AG_NetSocketSet *_Nonnull,
               AG_NetSocketSet *_Nullable,
               AG_NetSocketSet *_Nullable,
               AG_NetSocketSet *_Nullable, Uint32);

AG_NetSocket *_Nullable AG_NetAccept(AG_NetSocket *_Nonnull);

int AG_NetRead(AG_NetSocket *_Nonnull, void *_Nonnull, AG_Size,
               AG_Size *_Nonnull);
int AG_NetWrite(AG_NetSocket *_Nonnull, const void *_Nonnull, AG_Size,
                AG_Size *_Nonnull);

void AG_NetClose(AG_NetSocket *_Nonnull);
__END_DECLS

#include <agar/net/close.h>
