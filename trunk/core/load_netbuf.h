/*	Public domain	*/

#ifndef _AGAR_LOADER_NETBUF_H_
#define _AGAR_LOADER_NETBUF_H_
#include "begin_code.h"

enum ag_netbuf_endian {
	AG_NETBUF_BIG_ENDIAN,
	AG_NETBUF_LITTLE_ENDIAN
};

typedef struct ag_netbuf {
	char *path;
	enum ag_netbuf_endian byte_order;
	int fd;
	FILE *file;
} AG_Netbuf;

__BEGIN_DECLS
AG_Netbuf *AG_NetbufOpen(const char *, const char *, enum ag_netbuf_endian);
void	   AG_NetbufClose(AG_Netbuf *);

void	AG_NetbufPwrite(const void *, size_t, size_t, off_t, AG_Netbuf *);
void	AG_NetbufRead(void *, size_t, size_t, AG_Netbuf *);
void	AG_NetbufFlush(AG_Netbuf *);
void	AG_NetbufWrite(const void *, size_t, size_t, AG_Netbuf *);
long	AG_NetbufReadE(void *, size_t, size_t, AG_Netbuf *);
off_t	AG_NetbufTell(AG_Netbuf *);
void	AG_NetbufSeek(AG_Netbuf *, off_t, int);
void	AG_NetbufFlush(AG_Netbuf *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_NETBUF_H_ */
