/*	$Csoft: world.h,v 1.26 2003/04/12 01:45:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WORLD_H_
#define _AGAR_WORLD_H_ 
#include "begin_code.h"

struct world {
	struct object	obj;

	pthread_mutex_t		lock;
	SLIST_HEAD(, object)	wobjs;		/* Game objects */
	int			nobjs;
};

extern struct world *world;	/* Consistent while running */

__BEGIN_DECLS
extern DECLSPEC void	 world_init(struct world *, char *);
extern DECLSPEC void	 world_destroy(void *);
extern DECLSPEC int	 world_load(void *, struct netbuf *);
extern DECLSPEC int	 world_save(void *, struct netbuf *);
extern DECLSPEC void	 world_attach(void *);
extern DECLSPEC void	 world_detach(void *);

extern DECLSPEC int	 world_attached(void *);
extern DECLSPEC void	*world_find(char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WORLD_H_ */
