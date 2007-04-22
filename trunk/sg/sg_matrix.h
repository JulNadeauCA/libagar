/*	$Csoft$	*/
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
SG_Matrix	SG_MatrixTranspose(SG_Matrix);
SG_Matrix	SG_MatrixTransposep(SG_Matrix *);

__inline__ SG_Matrix	SG_MatrixMult(SG_Matrix, SG_Matrix);
__inline__ void		SG_MatrixMultv(SG_Matrix *, const SG_Matrix *);
__inline__ void		SG_MatrixMultpv(SG_Matrix *, const SG_Matrix *,
			                const SG_Matrix *);
int			SG_MatrixInvert(const SG_Matrix *, SG_Matrix *);

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

#define SG_Rotatev(n,a,v) SG_MatrixRotatev(&SGNODE(n)->T,(a),(v))
#define SG_RotateXv(n,a) SG_MatrixRotateXv(&SGNODE(n)->T,(a))
#define SG_RotateYv(n,a) SG_MatrixRotateYv(&SGNODE(n)->T,(a))
#define SG_RotateZv(n,a) SG_MatrixRotateZv(&SGNODE(n)->T,(a))
#define SG_RotateEul(n,p,y,r) SG_MatrixRotateEul(&SGNODE(n)->T,(p),(y),(r))

#define SG_Rotatevd(n,a,v) SG_MatrixRotatev(&SGNODE(n)->T,SG_Radians(a),(v))
#define SG_RotateXvd(n,a) SG_MatrixRotateXv(&SGNODE(n)->T,SG_Radians(a))
#define SG_RotateYvd(n,a) SG_MatrixRotateYv(&SGNODE(n)->T,SG_Radians(a))
#define SG_RotateZvd(n,a) SG_MatrixRotateZv(&SGNODE(n)->T,SG_Radians(a))
#define SG_RotateEuld(n,p,y,r) SG_MatrixRotateEul(&SGNODE(n)->T, \
    SG_Radians(p),SG_Radians(y),SG_Radians(r))

#define SG_Translatev(n,v) SG_MatrixTranslatev(&SGNODE(n)->T,(v))
#define SG_Translate3(n,x,y,z) SG_MatrixTranslate3(&SGNODE(n)->T,(x),(y),(z))
#define SG_TranslateX(n,t) SG_MatrixTranslateX(&SGNODE(n)->T,(t))
#define SG_TranslateY(n,t) SG_MatrixTranslateY(&SGNODE(n)->T,(t))
#define SG_TranslateZ(n,t) SG_MatrixTranslateZ(&SGNODE(n)->T,(t))
#define SG_Scalev(n,v) SG_MatrixScalev(&SGNODE(n)->T,(v))
#define SG_Identity(n) SG_MatrixIdentityv(&SGNODE(n)->T)

__inline__ void		SG_MatrixGetTranslation(const SG_Matrix *, SG_Vector *);
__inline__ void		SG_MatrixGetRotationXYZ(const SG_Matrix *, SG_Real *,
			                        SG_Real *, SG_Real *);

#ifdef SG_DOUBLE_PRECISION
#define SG_MultMatrixGL(A) glMultMatrixd(&(A)->m[0][0])
#define SG_TranslateGL(x,y,z) glTranslated((x),(y),(z))
#define SG_TranslateVecGL(v) glTranslated((v).x,(v).y,(v).z)
#define SG_RotateGL(a,x,y,z) glRotated((a),(x),(y),(z))
#define SG_RotateVecGL(a,v) glRotated((a),(v).x,(v).y,(v).z)
#else
#define SG_MultMatrixGL(A) glMultMatrixf(&(A)->m[0][0])
#define SG_TranslateGL(x,y,z) glTranslatef((x),(y),(z))
#define SG_TranslateVecGL(v) glTranslatef((v).x,(v).y,(v).z)
#define SG_RotateGL(a,x,y,z) glRotatef((a),(x),(y),(z))
#define SG_RotateVecGL(a,v) glRotatef((a),(v).x,(v).y,(v).z)
#endif

__inline__ SG_Matrix	SG_ReadMatrix(AG_Netbuf *);
void			SG_ReadMatrixv(AG_Netbuf *, SG_Matrix *);
void			SG_WriteMatrix(AG_Netbuf *, SG_Matrix *);
__inline__ void		SG_LoadMatrixGL(const SG_Matrix *);
__inline__ void		SG_GetMatrixGL(int, SG_Matrix *);
__END_DECLS
