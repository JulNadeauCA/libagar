/*	$Csoft: vec.h,v 1.3 2004/08/24 07:44:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_VEC_H_
#define _MAT_VEC_H_
#include "begin_code.h"

struct vec {
	int n;				/* Vector size */
	double *vec;			/* Vector data */
};

__BEGIN_DECLS
struct vec	*vec_new(int);
__inline__ void	 vec_resize(struct vec *, int);
__inline__ void	 vec_set(struct vec *, double);
__inline__ void	 vec_copy(const struct vec *, struct vec *);
__inline__ void	 vec_free(struct vec *);
__inline__ void	 vec_add(const struct vec *, struct vec *);
__inline__ void	 vec_mul(const struct vec *, struct vec *);
#ifdef DEBUG
void		 vec_print(const struct vec *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_VEC_H_ */
