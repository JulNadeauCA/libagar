/*	$Csoft: transform.h,v 1.7 2003/06/18 00:46:58 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TRANSFORM_H_
#define _AGAR_TRANSFORM_H_
#include "begin_code.h"

enum transform_type {
	TRANSFORM_HFLIP,
	TRANSFORM_VFLIP
};

struct transform {
	enum transform_type	type;
	void			(*func)(SDL_Surface **, int, Uint32 *);
	Uint32			*args;
	int			nargs;
	SLIST_ENTRY(transform)	transforms;
};

__BEGIN_DECLS
struct transform *transform_new(enum transform_type, int, Uint32 *);
int		  transform_init(struct transform *, enum transform_type, int,
		                 Uint32 *);
int		  transform_load(struct netbuf *, struct transform *);
void		  transform_save(struct netbuf *, const struct transform *);
void		  transform_destroy(struct transform *);
__inline__ int	  transform_compare(const struct transform *,
		                    const struct transform *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TRANSFORM_H_ */
