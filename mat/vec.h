/*	$Csoft: vec.h,v 1.2 2005/09/10 05:06:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_VEC_H_
#define _MAT_VEC_H_
#include "begin_code.h"

typedef struct mat vec_t;

__BEGIN_DECLS
vec_t		*vec_new(u_int);
#define		 vec_resize(v,m) mat_resize((v),(m),1)
#define		 vec_set(v,val) mat_set((v),(val))
#define		 vec_free(v) mat_free(v)

__inline__ void	 vec_copy(const vec_t *, vec_t *);
double		 vec_len(const vec_t *);

#ifdef DEBUG
void		 vec_print(const vec_t *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_VEC_H_ */
