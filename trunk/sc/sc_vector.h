/*	Public domain	*/

typedef struct sc_matrix SC_Vector;

#define vCheckEntry(A,i) mCheckEntry((A),(i),1)
#define vSet(A,i,v) mSet((A),(i),1,(v))
#define vAdd(A,i,v) mAdd((A),(i),1,(v))
#define vSub(A,i,v) mSub((A),(i),1,(v))
#define vMul(A,i,v) mMul((A),(i),1,(v))
#define vDiv(A,i,v) mDiv((A),(i),1,(v))
#define vEnt(A,i) mEnt((A),(i),1)
#define vEntp(A,i) mEntp((A),(i),1)
#define vExists(A,i) ((i) > 0 && (i) <= (A)->m)

__BEGIN_DECLS
SC_Vector	*SC_VectorNew(Uint);
SC_Vector	*SC_VectorNewZero(Uint);
#define		 SC_VectorResize(v,m) SC_MatrixResize((v),(m),1)
#define		 SC_VectorSetZero(v) SC_MatrixSetZero(v)
#define		 SC_VectorFree(v) SC_MatrixFree(v)
#define		 SC_ReadVector(buf) ((SC_Vector *)SC_ReadMatrix(buf))
#define		 SC_WriteVector(v,buf) SC_WriteMatrix(v, buf)

void		 SC_VectorMinimum(SC_Vector *, const SC_Vector *,
		                  const SC_Vector *);
void		 SC_VectorMaximum(SC_Vector *, const SC_Vector *,
		                  const SC_Vector *);

__inline__ void	 SC_VectorCopy(const SC_Vector *, SC_Vector *);
SC_Real		 SC_VectorLength(const SC_Vector *);

#ifdef DEBUG
void		 SC_VectorPrint(const SC_Vector *);
#endif
__END_DECLS
