/*	$Csoft: netbuf.h,v 1.1 2003/06/21 06:50:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_NETBUF_H_
#define _AGAR_LOADER_NETBUF_H_
#include "begin_code.h"

enum netbuf_endian {
	NETBUF_BIG_ENDIAN,
	NETBUF_LITTLE_ENDIAN
} endian;

struct netbuf {
	enum netbuf_endian byte_order;
	int		   fd;
	FILE		  *file;
};

__BEGIN_DECLS
struct netbuf	*netbuf_open(const char *, const char *, enum netbuf_endian);
void		 netbuf_close(struct netbuf *);

void	 netbuf_flush(struct netbuf *);
void	 netbuf_read(void *, size_t, size_t, struct netbuf *);
void	 netbuf_write(const void *, size_t, size_t, struct netbuf *);
void	 netbuf_pwrite(const void *, size_t, size_t, off_t, struct netbuf *);

__inline__ ssize_t	netbuf_eread(void *, size_t, size_t, struct netbuf *);
__inline__ off_t	netbuf_tell(struct netbuf *);
__inline__ void		netbuf_seek(struct netbuf *, off_t, int);
__inline__ void		netbuf_flush(struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_NETBUF_H_ */
