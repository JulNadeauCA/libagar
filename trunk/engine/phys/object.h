/*	$Csoft: phys_object.h,v 1.16 2003/09/02 02:07:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PHYS_OBJECT_H_
#define _AGAR_PHYS_OBJECT_H_
#include "begin_code.h"

struct phys_object {
	struct object obj;
	pthread_mutex_t	lock;
};

#define PHYS_OBJECT(ob)	((struct phys_object *)(ob))

__BEGIN_DECLS
struct phys_object	*phys_object_new(void *, const char *);
void			 phys_object_init(void *, const char *);
void			 phys_object_destroy(void *);
struct window		*phys_object_edit(void *);
int			 phys_object_load(void *, struct netbuf *);
int			 phys_object_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PHYS_OBJECT_H_ */
