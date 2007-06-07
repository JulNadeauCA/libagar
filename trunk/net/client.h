/*	Public domain	*/

#ifndef _AGAR_NET_CLIENT_H_
#define _AGAR_NET_CLIENT_H_

#define AGN_HOSTNAME_MAX	256
#define AGN_PORTNUM_MAX		16
#define AGN_USERNAME_MAX	48
#define AGN_PASSWORD_MAX	64

typedef struct nc_session {
	const char *name;
	char host[AGN_HOSTNAME_MAX];
	char port[AGN_PORTNUM_MAX];
	char user[AGN_USERNAME_MAX];
	char pass[AGN_PASSWORD_MAX];
	int sock;
	struct {
		char *buf;
		size_t len;
		size_t maxlen;
	} read;
	char server_proto[32];
	char client_proto[32];
} NC_Session;

typedef struct nc_result {
	unsigned int argc;
	char	**argv;
	size_t	 *argv_len;
} NC_Result;

__BEGIN_DECLS
void	 	 NC_Init(NC_Session *, const char *, const char *);
void		 NC_Destroy(NC_Session *);
int		 NC_Connect(NC_Session *, const char *, const char *,
		             const char *, const char *);
int		 NC_Reconnect(NC_Session *);
void	 	 NC_Disconnect(NC_Session *);
ssize_t		 NC_Read(NC_Session *, size_t);
ssize_t		 NC_ReadBinary(NC_Session *, size_t);
int		 NC_Write(NC_Session *, const char *, ...);
NC_Result	*NC_Query(NC_Session *, const char *, ...);
NC_Result	*NC_QueryBinary(NC_Session *, const char *, ...);
void		 NC_FreeResult(NC_Result *);
__END_DECLS

#endif /* _AGAR_NET_CLIENT_H_ */
