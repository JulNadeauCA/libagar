/*	$Csoft: veci.h,v 1.1 2004/06/26 03:42:19 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_VECI_H_
#define _MAT_VECI_H_
#include "begin_code.h"

struct veci {
	int n;				/* Length */
	int *vec;			/* Vector data */
};

__BEGIN_DECLS
struct veci	*veci_new(int);
__inline__ void	 veci_set(struct veci *, int);
__inline__ void	 veci_copy(const struct veci *, struct veci *);
__inline__ void	 veci_free(struct veci *);
__inline__ void	 veci_add(const struct veci *, struct veci *);
#ifdef DEBUG
void		 veci_print(const struct veci *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_VECI_H_ */
