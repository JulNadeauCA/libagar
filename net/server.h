/*	Public domain	*/

#ifndef _AGAR_NET_PUBLIC
#ifdef HAVE_SYSLOG
#include <syslog.h>
#else
#define LOG_ERR 1
#define LOG_DEBUG 2
#define LOG_INFO 3
#define LOG_NOTICE 4
#endif
#endif /* _AGAR_NET_PUBLIC */

typedef struct agn_server_cmd {
	const char *name;
	int (*func)(AGN_Command *, void *);
	void *arg;
} AGN_ServerCmd;

typedef struct agn_server_auth {
	const char *name;
	int (*func)(void *);
	void *arg;
} AGN_ServerAuth;

#ifndef _AGAR_NET_PUBLIC
extern pid_t server_pid;
#endif

__BEGIN_DECLS
void	AGN_ServerSetErrorFn(void (*)(void));
void	AGN_ServerRegCmd(const char *, int (*)(AGN_Command *, void *), void *);
void	AGN_ServerRegAuth(const char *, int (*)(void *), void *);
void	AGN_ServerRegCallback(void (*)(void), int, int);
int	AGN_ServerListen(const char *, const char *, const char *,
	                 const char *);
void	AGN_ServerDie(int, const char *, ...);
void	AGN_ServerLog(int, const char *, ...);
void	AGN_ServerBinaryMode(size_t);
void	AGN_ServerCommandMode(void);

void	AGN_ServerBeginList(void);
void	AGN_ServerEndList(void);
void	AGN_ServerListItem(void *, size_t);
void	AGN_ServerListString(const char *, ...);
__END_DECLS
