/*	$Csoft: gobject.h,v 1.1 2005/02/08 15:57:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_GOBJECT_H_
#define _AGAR_GOBJECT_H_

#include <engine/space/space.h>

#include "begin_code.h"

struct gobject_ops {
	struct object_ops ops;
	int (*specify)(void *, void *, void *);
};

struct gobject {
	struct object obj;

	pthread_mutex_t	 lock;
};

#define GOBJECT(ob) ((struct gobject *)(ob))

__BEGIN_DECLS
void		 gobject_init(void *, const char *, const char *,
		              const struct gobject_ops *);
void		 gobject_reinit(void *);
void		 gobject_destroy(void *);
struct window	*gobject_edit(void *);
int		 gobject_load(void *, struct netbuf *);
int		 gobject_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_GOBJECT_H_ */
