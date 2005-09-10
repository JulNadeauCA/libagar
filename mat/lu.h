/*	$Csoft: lu.h,v 1.1 2004/11/23 02:32:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_LU_H_
#define _MAT_LU_H_
#include "begin_code.h"

__BEGIN_DECLS
int	mat_lu_decompose(mat_t *, veci_t *, double *);
void	mat_lu_backsubst(const mat_t *, const veci_t *, vec_t *);
__END_DECLS

#include "close_code.h"
#endif /* _MAT_LU_H_ */
