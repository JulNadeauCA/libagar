/*	$Csoft: lu.h,v 1.2 2004/08/26 06:28:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_LU_H_
#define _MAT_LU_H_
#include "begin_code.h"

__BEGIN_DECLS
int	mat_lu_decompose(struct mat *, struct veci *, double *);
void	mat_lu_backsubst(const struct mat *, const struct veci *,
	                 struct vec *);
__END_DECLS

#include "close_code.h"
#endif /* _MAT_LU_H_ */
