/*	Public domain	*/

typedef struct sc_matrix {
	Uint m, n;
	SC_Real **mat;
} SC_Matrix;

#ifdef DEBUG
#define SC_AssertEntry(A,i,j) \
	if ((i) < 1 || (i) > (A)->m || (j) < 1 || (j) > (A)->n) \
	    AG_FatalError("A(%d,%d) >< A(%dx%d)",(i),(j),(A)->m,(A)->n)
#define mSet(A,i,j,v) do { \
	SC_AssertEntry((A),(i),(j)); \
	(A)->mat[i][j] = (v); \
} while(0)
#define mAdd(A,i,j,v) do {\
	SC_AssertEntry((A),(i),(j)); \
	(A)->mat[i][j] += (v); \
} while(0)
#define mSub(A,i,j,v) do {\
	SC_AssertEntry((A),(i),(j)); \
	(A)->mat[i][j] -= (v); \
} while(0)
#define mMul(A,i,j,v) do {\
	SC_AssertEntry((A),(i),(j)); \
	(A)->mat[i][j] *= (v); \
} while(0)
#define mDiv(A,i,j,v) do {\
	SC_AssertEntry((A),(i),(j)); \
	(A)->mat[i][j] /= (v); \
} while(0)
#define mEnt(A,i,j) SC_MatrixGetEntry((A),(i),(j))
#define mEntp(A,i,j) SC_MatrixGetEntryp((A),(i),(j))
#else /* !DEBUG */
#define SC_AssertEntry(A,i,j) /* nothing */
#define mSet(A,i,j,v) (A)->mat[i][j] = (v)
#define mAdd(A,i,j,v) (A)->mat[i][j] += (v)
#define mSub(A,i,j,v) (A)->mat[i][j] -= (v)
#define mMul(A,i,j,v) (A)->mat[i][j] *= (v)
#define mDiv(A,i,j,v) (A)->mat[i][j] /= (v)
#define mEnt(A,i,j) ((A)->mat[i][j])
#define mEntp(A,i,j) (&(A)->mat[i][j])
#endif /* DEBUG */

#define mExists(A,i,j) SC_MatrixEntryExists((A),(i),(j))

__BEGIN_DECLS
SC_Matrix	*SC_MatrixNew(Uint, Uint);
__inline__ void	 SC_MatrixFree(SC_Matrix *);
void		 SC_MatrixAlloc(SC_Matrix *, Uint, Uint);
void		 SC_MatrixFreeElements(SC_Matrix *);
__inline__ void	 SC_MatrixResize(SC_Matrix *, Uint, Uint);
void		 SC_WriteMatrix(SC_Matrix *, AG_Netbuf *);
SC_Matrix	*SC_ReadMatrix(AG_Netbuf *);

__inline__ SC_Real	 SC_MatrixGetEntry(const SC_Matrix *, Uint, Uint);
__inline__ SC_Real	*SC_MatrixGetEntryp(const SC_Matrix *, Uint, Uint);
__inline__ int		 SC_MatrixEntryExists(const SC_Matrix *, Uint, Uint);
__inline__ SC_Matrix	*SC_MatrixDup(const SC_Matrix *);
__inline__ int		 SC_MatrixCompare(const SC_Matrix *, const SC_Matrix *);

void		 SC_MatrixSetIdentity(SC_Matrix *);
void	 	 SC_MatrixSetZero(SC_Matrix *);

void	 	 SC_MatrixCopy(const SC_Matrix *, SC_Matrix *);
void		 SC_MatrixSum(const SC_Matrix *, SC_Matrix *);
SC_Matrix	*SC_MatrixDirectSum(const SC_Matrix *, const SC_Matrix *);
void		 SC_MatrixMulv(const SC_Matrix *, const SC_Matrix *,
		               SC_Matrix *);
void		 SC_MatrixEntrywiseMul(const SC_Matrix *, const SC_Matrix *,
		                       SC_Matrix *);
SC_Matrix	*SC_MatrixTranspose(const SC_Matrix *, SC_Matrix *);
SC_Real		 SC_MatrixTrace(const SC_Matrix *);

void		 SC_MatrixCompose21(SC_Matrix *, const SC_Matrix *,
		                    const SC_Matrix *);
void		 SC_MatrixCompose12(SC_Matrix *, const SC_Matrix *,
		                    const SC_Matrix *);
void		 SC_MatrixCompose22(SC_Matrix *, const SC_Matrix *,
		                    const SC_Matrix *, const SC_Matrix *,
				    const SC_Matrix *);

__inline__ int	 SC_MatrixIsSquare(const SC_Matrix *);
int		 SC_MatrixIsIdentity(const SC_Matrix *);
int		 SC_MatrixIsZero(const SC_Matrix *);
int		 SC_MatrixIsLowTri(const SC_Matrix *);
int		 SC_MatrixIsLowTriStrict(const SC_Matrix *);
int		 SC_MatrixIsLowTriNormed(const SC_Matrix *);
int		 SC_MatrixIsUpTri(const SC_Matrix *);
int		 SC_MatrixIsUpTriStrict(const SC_Matrix *);
int		 SC_MatrixIsUpTriNormed(const SC_Matrix *);
#define		 SC_MatrixIsDiagonal(M) (SC_MatrixIsLowTri(M) || \
		                         SC_MatrixIsUpTri(M))
int		 SC_MatrixIsSymmetric(const SC_Matrix *);

#ifdef DEBUG
void		 SC_MatrixPrint(const SC_Matrix *);
void		 SC_MatrixTest(void);
#endif
__END_DECLS
