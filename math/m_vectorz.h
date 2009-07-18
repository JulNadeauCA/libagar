/*	Public domain	*/

typedef struct m_vectorz {
	Uint n;
	int *v;
} M_VectorZ;

__BEGIN_DECLS
M_VectorZ *M_VectorNewZ(Uint);
void       M_VectorSetZ(M_VectorZ *, int);
void       M_VectorCopyZ(const M_VectorZ *, M_VectorZ *);
void	   M_VectorFreeZ(M_VectorZ *);
void	   M_VectorAddZv(M_VectorZ *, const M_VectorZ *);
void	   M_VectorSubZv(M_VectorZ *, const M_VectorZ *);
void	   M_VectorScaleZv(M_VectorZ *, M_Real);
void	   M_VectorResizeV(M_VectorZ *, Uint);
void	   M_VectorPrintZ(const M_VectorZ *);
__END_DECLS
