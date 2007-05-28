/*	Public domain	*/

#ifndef _AGAR_NET_CLIENT_H_
#define _AGAR_NET_CLIENT_H_

#define AGN_HOSTNAME_MAX	256
#define AGN_PORTNUM_MAX		16
#define AGN_USERNAME_MAX	48
#define AGN_PASSWORD_MAX	64

typedef struct agc_session {
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
} AGC_Session;

typedef struct agc_result {
	unsigned int argc;
	char	**argv;
	size_t	 *argv_len;
} AGC_Result;

__BEGIN_DECLS
void	 	 AGC_Init(AGC_Session *, const char *, const char *);
void		 AGC_Destroy(AGC_Session *);
int		 AGC_Connect(AGC_Session *, const char *, const char *,
		             const char *, const char *);
int		 AGC_Reconnect(AGC_Session *);
void	 	 AGC_Disconnect(AGC_Session *);
ssize_t		 AGC_Read(AGC_Session *, size_t);
ssize_t		 AGC_ReadBinary(AGC_Session *, size_t);
int		 AGC_Write(AGC_Session *, const char *, ...);
AGC_Result	*AGC_Query(AGC_Session *, const char *, ...);
AGC_Result	*AGC_QueryBinary(AGC_Session *, const char *, ...);
void		 AGC_FreeResult(AGC_Result *);
__END_DECLS

#endif /* _AGAR_NET_CLIENT_H_ */
