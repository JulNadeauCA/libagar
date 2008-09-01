/*	Public domain	*/

typedef struct m_int_vector {
	Uint n;
	int *v;
} M_IntVector;

__BEGIN_DECLS
M_IntVector *M_IntVectorNew(Uint);
void         M_IntVectorSet(M_IntVector *, int);
void         M_IntVectorCopy(const M_IntVector *, M_IntVector *);
void	     M_IntVectorFree(M_IntVector *);
void	     M_IntVectorAddv(M_IntVector *, const M_IntVector *);
void	     M_IntVectorSubv(M_IntVector *, const M_IntVector *);
void	     M_IntVectorScalev(M_IntVector *, M_Real);
void	     M_IntVectorResize(M_IntVector *, Uint);
void	     M_IntVectorPrint(const M_IntVector *);
__END_DECLS
