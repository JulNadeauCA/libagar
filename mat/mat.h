/*	$Csoft: mat.h,v 1.3 2004/08/24 07:44:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_MAT_H_
#define _MAT_MAT_H_

struct mat;
struct vec;
struct veci;

#include <mat/vec.h>
#include <mat/veci.h>
#include <mat/lu.h>
#include <mat/gaussj.h>

#include "begin_code.h"

#ifndef TINY_VAL
#define TINY_VAL	1.0e-22
#endif

struct mat {
	int m, n;			/* Rows and columns */
	double **mat;			/* Matrix elements */
};

__BEGIN_DECLS
struct mat	*mat_new(int, int);
__inline__ void	 mat_free(struct mat *);
void		 mat_alloc_elements(struct mat *, int, int);
void		 mat_free_elements(struct mat *);
__inline__ void	 mat_resize(struct mat *, int, int);
void		 mat_set(struct mat *, double);
void		 mat_copy(const struct mat *, struct mat *);
void		 mat_sum(const struct mat *, struct mat *);
void		 mat_mul(const struct mat *, const struct mat *, struct mat *);
void		 mat_entmul(const struct mat *, const struct mat *,
		            struct mat *);

void		 mat_set_identity(struct mat *);
__inline__ int	 mat_is_square(struct mat *);
int		 mat_is_identity(struct mat *);

#ifdef DEBUG
void		 mat_print(const struct mat *);
void		 mat_test(void);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_MAT_H_ */
