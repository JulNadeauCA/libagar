/*	Public domain	*/

enum ns_log_lvl {
	NS_DEBUG,
	NS_INFO,
	NS_NOTICE,
	NS_WARNING,
	NS_ERR,
	NS_CRIT,
	NS_ALERT,
	NS_EMERG
};

typedef struct agn_server_cmd {
	const char *name;
	int (*func)(NS_Command *, void *);
	void *arg;
} NS_Cmd;

typedef struct agn_server_auth {
	const char *name;
	int (*func)(void *);
	void *arg;
} NS_Auth;

#ifndef _AGAR_NET_PUBLIC
extern pid_t server_pid;
#endif

__BEGIN_DECLS
void	NS_Log(enum ns_log_lvl, const char *, ...);
void	NS_SetErrorFn(void (*)(void));
void	NS_RegCmd(const char *, int (*)(NS_Command *, void *), void *);
void	NS_RegAuth(const char *, int (*)(void *), void *);
void	NS_RegCallback(void (*)(void), int, int);
int	NS_Listen(const char *, const char *, const char *,
	                 const char *);
void	NS_Die(int, const char *, ...);
void	NS_BinaryMode(size_t);
void	NS_CommandMode(void);

void	NS_BeginList(void);
void	NS_EndList(void);
void	NS_ListItem(void *, size_t);
void	NS_ListString(const char *, ...);
__END_DECLS
