/*	$Csoft: tileset.h,v 1.2 2004/11/22 03:56:50 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TILESET_H_
#define _AGAR_TILESET_H_
#include "begin_code.h"

struct tileset {
	struct object obj;
	pthread_mutex_t	lock;
};

__BEGIN_DECLS
struct tileset	 *tileset_new(void *, const char *);
void		  tileset_init(void *, const char *);
void		  tileset_destroy(void *);
int		  tileset_load(void *, struct netbuf *);
int		  tileset_save(void *, struct netbuf *);
struct window	 *tileset_edit(void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TILESET_H_ */
