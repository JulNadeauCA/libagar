/*	$Csoft: transform.h,v 1.5 2003/04/12 01:45:31 vedge Exp $	*/
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
extern DECLSPEC struct transform *transform_new(enum transform_type, int,
				                Uint32 *);
extern DECLSPEC int		  transform_init(struct transform *,
				                 enum transform_type, int,
					         Uint32 *);

extern DECLSPEC int	transform_load(struct netbuf *, struct transform *);
extern DECLSPEC void	transform_save(struct netbuf *, struct transform *);
extern DECLSPEC void	transform_destroy(struct transform *);
extern __inline__ int	transform_compare(struct transform *,
			                  struct transform *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TRANSFORM_H_ */
