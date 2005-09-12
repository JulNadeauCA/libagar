/*	$Csoft: veci.h,v 1.2 2005/09/10 05:06:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_VECI_H_
#define _MAT_VECI_H_
#include "begin_code.h"

struct veci {
	int n;				/* Length */
	int *vec;			/* Vector data */
};

typedef struct veci veci_t;

__BEGIN_DECLS
veci_t		*veci_new(u_int);
__inline__ void	 veci_set(veci_t *, int);
__inline__ void	 veci_copy(const veci_t *, veci_t *);
__inline__ void	 veci_free(veci_t *);
__inline__ void	 veci_add(const veci_t *, veci_t *);
#ifdef DEBUG
void		 veci_print(const veci_t *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_VECI_H_ */
