/*	Public domain	*/

#include <agar/core/begin.h>

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
			char  *path;		/* Unix socket path */
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
	char *sNum;				/* Numerical form */
	char *sName;				/* Reverse DNS (UTF-8) */
	AG_TAILQ_ENTRY(ag_net_addr) addrs;
} AG_NetAddr;

/* List of socket addresses. */
typedef AG_TAILQ_HEAD(ag_net_addr_list, ag_net_addr) AG_NetAddrList;

/* Endpoint for communication */
typedef struct ag_net_socket {
	enum ag_net_addr_family family;		/* Address family */
	enum ag_net_socket_type type;		/* Socket type */
	int proto;				/* Socket protocol number */
	AG_Mutex lock;
	Uint flags;
#define AG_NET_SOCKET_BOUND	0x01		/* Bound to a local address */
#define AG_NET_SOCKET_CONNECTED	0x02		/* Connection established */
	Uint poll;
#define AG_NET_POLL_READ	0x01		/* Poll read condition */
#define AG_NET_POLL_WRITE	0x02		/* Poll write condition */
#define AG_NET_POLL_EXCEPTIONS	0x04		/* Poll exceptions */

	AG_NetAddr *addrLocal;			/* Bound local address */
	AG_NetAddr *addrRemote;			/* Connected remote address */
	int fd;					/* File descriptor (if any) */
	int listenBacklog;			/* For AG_NET_BACKLOG */
	void *p;				/* User pointer */

	AG_TAILQ_ENTRY(ag_net_socket) sockets;
	AG_TAILQ_ENTRY(ag_net_socket) read;	/* Poll read results */
	AG_TAILQ_ENTRY(ag_net_socket) write;	/* Poll write results */
	AG_TAILQ_ENTRY(ag_net_socket) except;	/* Poll exception results */
} AG_NetSocket;

/* List of sockets. */
typedef AG_TAILQ_HEAD(ag_net_socket_set, ag_net_socket) AG_NetSocketSet;

typedef struct ag_net_ops {
	const char *name;

	int           (*init)(void);
	void          (*destroy)(void);
	int           (*getIfConfig)(AG_NetAddrList *);
	int           (*resolve)(AG_NetAddrList *, const char *, const char *, Uint);
	char         *(*getAddrNumerical)(AG_NetAddr *);
	int           (*initSocket)(AG_NetSocket *);
	void          (*destroySocket)(AG_NetSocket *);
	int           (*connect)(AG_NetSocket *, const AG_NetAddr *);
	int           (*bind)(AG_NetSocket *, const AG_NetAddr *);
	int           (*getOption)(AG_NetSocket *, enum ag_net_socket_option, void *);
	int           (*setOption)(AG_NetSocket *, enum ag_net_socket_option, const void *);
	int           (*poll)(AG_NetSocketSet *, AG_NetSocketSet *,
	                      AG_NetSocketSet *, AG_NetSocketSet *, Uint32);
	AG_NetSocket *(*accept)(AG_NetSocket *);
	int           (*read)(AG_NetSocket *, void *, size_t, size_t *);
	int           (*write)(AG_NetSocket *, const void *, size_t, size_t *);
	void          (*close)(AG_NetSocket *);
} AG_NetOps;

__BEGIN_DECLS
extern const AG_NetOps *agNetOps;
extern const AG_NetOps  agNetOps_bsd;
extern const AG_NetOps  agNetOps_winsock1;
extern const AG_NetOps  agNetOps_winsock2;
extern const AG_NetOps  agNetOps_dummy;

extern const char *agNetAddrFamilyNames[];
extern const char *agNetSocketTypeNames[];

int             AG_InitNetworkSubsystem(const AG_NetOps *);
void		AG_DestroyNetworkSubsystem(void);

AG_NetSocket   *AG_NetSocketNew(enum ag_net_addr_family,
                                enum ag_net_socket_type, int);
void            AG_NetSocketFree(AG_NetSocket *);
void            AG_NetSocketSetInit(AG_NetSocketSet *);
void            AG_NetSocketSetClear(AG_NetSocketSet *);
#define         AG_NetSocketSetFree(nss) AG_NetSocketSetClear(nss)

AG_NetAddr     *AG_NetAddrNew(void);
AG_NetAddr     *AG_NetAddrDup(const AG_NetAddr *);
int             AG_NetAddrCompare(const AG_NetAddr *, const AG_NetAddr *);
int             AG_NetAddrIsAny(const AG_NetAddr *);
void		AG_NetAddrFree(AG_NetAddr *);
const char     *AG_NetAddrNumerical(AG_NetAddr *);

AG_NetAddrList *AG_NetAddrListNew(void);
void            AG_NetAddrListClear(AG_NetAddrList *);
void            AG_NetAddrListFree(AG_NetAddrList *);

AG_NetAddrList *AG_NetResolve(const char *, const char *, Uint);
AG_NetAddrList *AG_NetGetIfConfig(void);
int             AG_NetConnect(AG_NetSocket *, const AG_NetAddrList *);
int             AG_NetBind(AG_NetSocket *, const AG_NetAddr *);
int             AG_NetGetOption(AG_NetSocket *, enum ag_net_socket_option, void *);
int             AG_NetGetOptionInt(AG_NetSocket *, enum ag_net_socket_option, int *);
int             AG_NetSetOption(AG_NetSocket *, enum ag_net_socket_option, const void *);
int             AG_NetSetOptionInt(AG_NetSocket *, enum ag_net_socket_option, int);
int             AG_NetPoll(AG_NetSocketSet *, AG_NetSocketSet *, AG_NetSocketSet *,
                           AG_NetSocketSet *, Uint32);
AG_NetSocket   *AG_NetAccept(AG_NetSocket *);
int             AG_NetRead(AG_NetSocket *, void *, size_t , size_t *)
                           BOUNDED_ATTRIBUTE(__buffer__,2,3);
int             AG_NetWrite(AG_NetSocket *, const void *, size_t , size_t *)
                            BOUNDED_ATTRIBUTE(__buffer__,2,3);
void            AG_NetClose(AG_NetSocket *);
__END_DECLS

#include <agar/core/close.h>
