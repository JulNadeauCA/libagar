/*	$Csoft: vec.h,v 1.1 2004/11/23 02:32:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_VEC_H_
#define _MAT_VEC_H_
#include "begin_code.h"

typedef struct mat vec_t;

__BEGIN_DECLS
vec_t		*vec_new(int);
__inline__ void	 vec_resize(vec_t *, int);
#define		 vec_resize(v,m) mat_resize((v),(m),1)
#define		 vec_set(v,val) mat_set((v),(val))
__inline__ void	 vec_copy(const vec_t *, vec_t *);
#define		 vec_free(v) mat_free(v)
__inline__ void	 vec_add(const vec_t *, vec_t *);
__inline__ void	 vec_mul(const vec_t *, vec_t *);
#ifdef DEBUG
void		 vec_print(const vec_t *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_VEC_H_ */
