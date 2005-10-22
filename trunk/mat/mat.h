/*	$Csoft: mat.h,v 1.6 2005/09/12 10:06:42 vedge Exp $	*/
/*	Public domain	*/

#ifndef _MAT_MAT_H_
#define _MAT_MAT_H_

struct mat;
typedef struct mat mat_t;

#include <mat/vec.h>
#include <mat/veci.h>
#include <mat/lu.h>
#include <mat/gaussj.h>

#include "begin_code.h"

#ifndef TINY_VAL
#define TINY_VAL	1.0e-22
#endif

struct mat {
	Uint m, n;		/* Rows (m) and columns (n) */
	double **mat;		/* Matrix elements ([m][n] order) */
};

__BEGIN_DECLS
mat_t		*mat_new(Uint, Uint);
__inline__ void	 mat_free(mat_t *);
void		 mat_alloc_elements(mat_t *, Uint, Uint);
void		 mat_free_elements(mat_t *);
__inline__ void	 mat_resize(mat_t *, Uint, Uint);
void		 mat_set(mat_t *, double);
void		 mat_copy(const mat_t *, mat_t *);
__inline__ mat_t *mat_dup(const mat_t *);
void		 mat_sum(const mat_t *, mat_t *);
mat_t		*mat_dsum(const mat_t *, const mat_t *);
void		 mat_mul(const mat_t *, const mat_t *, mat_t *);
void		 mat_hmul(const mat_t *, const mat_t *, mat_t *);
mat_t		*mat_transpose(const mat_t *, mat_t *);
double		 mat_trace(const mat_t *);

void		 mat_compose21(mat_t *, const mat_t *, const mat_t *);
void		 mat_compose12(mat_t *, const mat_t *, const mat_t *);
void		 mat_compose22(mat_t *, const mat_t *, const mat_t *,
			       const mat_t *, const mat_t *);

void		 mat_set_identity(mat_t *);

__inline__ int	 mat_is_square(const mat_t *);
int		 mat_is_ident(const mat_t *);
int		 mat_is_zero(const mat_t *);
int		 mat_is_L(const mat_t *);
int		 mat_is_L_strict(const mat_t *);
int		 mat_is_L_normed(const mat_t *);
int		 mat_is_U(const mat_t *);
int		 mat_is_U_strict(const mat_t *);
int		 mat_is_U_normed(const mat_t *);
#define		 mat_is_diagonal(M) (mat_is_L(M) && mat_is_U(M))
int		 mat_is_symmetric(const mat_t *);

#ifdef DEBUG
void		 mat_print(const mat_t *);
void		 mat_test(void);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _MAT_MAT_H_ */
