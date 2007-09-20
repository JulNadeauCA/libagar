/*	Public domain	*/

typedef struct sg_matrix3 { SG_Real m[3][3]; } SG_Matrix3;
typedef struct sg_matrix4 { SG_Real m[4][4]; } SG_Matrix4, SG_Matrix;

#define SG_MATRIX3_ZERO		{ {0,0,0}, {0,0,0}, {0,0,0} }
#define SG_MATRIX3_IDENTITY	{ {1,0,0}, {0,1,0}, {0,0,1} }
#define SG_MATRIX_ZERO		{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} }
#define SG_MATRIX_IDENTITY	{ {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} }

__BEGIN_DECLS
void		SG_MatrixPrint(SG_Matrix *);
SG_Matrix	SG_MatrixIdentity(void);
void		SG_MatrixIdentityv(SG_Matrix *);
void		SG_MatrixCopy(SG_Matrix *, const SG_Matrix *);
void		SG_MatrixToFloats(float *, const SG_Matrix *);
void		SG_MatrixToDoubles(double *, const SG_Matrix *);
void		SG_MatrixDirection(const SG_Matrix *, SG_Vector *, SG_Vector *,
		                   SG_Vector *);
__inline__ SG_Matrix	SG_MatrixTranspose(SG_Matrix);
__inline__ SG_Matrix	SG_MatrixTransposep(const SG_Matrix *);
__inline__ void		SG_MatrixTransposev(SG_Matrix *);
__inline__ void		SG_MatrixDiagonalSwap(SG_Matrix *);

__inline__ SG_Matrix	SG_MatrixMult(SG_Matrix, SG_Matrix);
__inline__ void		SG_MatrixMultv(SG_Matrix *, const SG_Matrix *);
__inline__ void		SG_MatrixMultpv(SG_Matrix *, const SG_Matrix *,
			                const SG_Matrix *);
int			SG_MatrixInvert(const SG_Matrix *, SG_Matrix *);
SG_Matrix		SG_MatrixInvertCramerp(const SG_Matrix *);

__inline__ SG_Vector	SG_MatrixMultVector(SG_Matrix, SG_Vector);
__inline__ SG_Vector	SG_MatrixMultVectorp(const SG_Matrix *,
			                     const SG_Vector *);
__inline__ void		SG_MatrixMultVectorv(SG_Vector *, const SG_Matrix *);

__inline__ SG_Vector4	SG_MatrixMultVector4(SG_Matrix, SG_Vector4);
__inline__ SG_Vector4	SG_MatrixMultVector4p(const SG_Matrix *,
			                     const SG_Vector4 *);
__inline__ void		SG_MatrixMultVector4v(SG_Vector4 *, const SG_Matrix *);
__inline__ void		SG_MatrixMultVector4fv(float *, const SG_Matrix *);

void			SG_MatrixRotatev(SG_Matrix *, SG_Real, SG_Vector);
void			SG_MatrixOrbitv(SG_Matrix *, SG_Vector, SG_Vector,
			                SG_Real);
void			SG_MatrixRotateXv(SG_Matrix *, SG_Real);
void			SG_MatrixRotateYv(SG_Matrix *, SG_Real);
void			SG_MatrixRotateZv(SG_Matrix *, SG_Real);
void			SG_MatrixRotateEul(SG_Matrix *, SG_Real, SG_Real,
			                   SG_Real);
void			SG_MatrixTranslatev(SG_Matrix *, SG_Vector);
void			SG_MatrixTranslate2(SG_Matrix *, SG_Real, SG_Real);
void			SG_MatrixTranslate3(SG_Matrix *, SG_Real, SG_Real,
			                    SG_Real);
void			SG_MatrixTranslateX(SG_Matrix *, SG_Real);
void			SG_MatrixTranslateY(SG_Matrix *, SG_Real);
void			SG_MatrixTranslateZ(SG_Matrix *, SG_Real);
void			SG_MatrixScalev(SG_Matrix *, SG_Vector);
void			SG_MatrixScale2(SG_Matrix *, SG_Real, SG_Real);
void			SG_MatrixUniScale(SG_Matrix *, SG_Real);

__inline__ void		SG_MatrixGetTranslation(const SG_Matrix *, SG_Vector *);
__inline__ void		SG_MatrixGetRotationXYZ(const SG_Matrix *, SG_Real *,
			                        SG_Real *, SG_Real *);

__inline__ SG_Matrix	SG_ReadMatrix(AG_Netbuf *);
void			SG_ReadMatrixv(AG_Netbuf *, SG_Matrix *);
void			SG_WriteMatrix(AG_Netbuf *, SG_Matrix *);
__inline__ void		SG_LoadMatrixGL(const SG_Matrix *);
__inline__ void		SG_GetMatrixGL(int, SG_Matrix *);
__END_DECLS
