/*	Public domain	*/

typedef struct sg_vector3 { SG_Real x, y, z; } SG_Vector3;
typedef struct sg_vector3 SG_Vector;
typedef struct sg_vector4 { SG_Real x, y, z, w; } SG_Vector4;

#define SG_VECTOR(x,y,z) SG_GetVector((x),(y),(z))
#define SG_VECTOR3(x,y,z) SG_GetVector((x),(y),(z))
#define SG_VECTOR2(x,y) SG_GetVector((x),(y),0)
#define SG_INV(v) SG_VectorMirror((v),1,1,1);

#define SG_0 SG_GetZeroVector()
#define SG_I (SG_VECTOR(1.0,0.0,0.0))
#define SG_J (SG_VECTOR(0.0,1.0,0.0))
#define SG_K (SG_VECTOR(0.0,0.0,1.0))
#define SG_X (SG_VECTOR(1.0,0.0,0.0))
#define SG_Y (SG_VECTOR(0.0,1.0,0.0))
#define SG_Z (SG_VECTOR(0.0,0.0,1.0))

#define SG_U(theta,phi) \
    (SG_VECTOR(SG_Cos(phi)*SG_Cos(theta),SG_Cos(phi)*SG_Sin(theta),SG_Sin(phi)))

#define SG_VECTOR_INITIALIZER(x,y,z) { (x), (y), (z) }
#define SG_ZERO_VECTOR_INITIALIZER() { 0.0, 0.0, 0.0 }

#ifdef SG_DOUBLE_PRECISION
#define SGVEC(v) ((double *)&(v))
#else
#define SGVEC(v) ((float *)&(v))
#endif

__BEGIN_DECLS
__inline__ SG_Vector	SG_GetVector(SG_Real, SG_Real, SG_Real);
__inline__ SG_Vector	SG_GetZeroVector(void);
__inline__ void	  	SG_SetVector(SG_Vector *, SG_Real, SG_Real, SG_Real);
__inline__ void	  	SG_CopyVector(SG_Vector *, const SG_Vector *);
__inline__ SG_Real	SG_VectorDot(SG_Vector, SG_Vector);
__inline__ SG_Real	SG_VectorDotp(const SG_Vector *, const SG_Vector *);
__inline__ SG_Real	SG_VectorLen(SG_Vector);
__inline__ SG_Real	SG_VectorLen2(SG_Vector);
__inline__ SG_Real	SG_VectorLenp(const SG_Vector *);
__inline__ SG_Real	SG_VectorLen2p(const SG_Vector *);
__inline__ SG_Real	SG_VectorDistance(SG_Vector, SG_Vector);
__inline__ SG_Real	SG_VectorDistance2(SG_Vector, SG_Vector);
__inline__ SG_Real	SG_VectorDistancep(const SG_Vector *,
			                   const SG_Vector *);
__inline__ SG_Real	SG_VectorDistance2p(const SG_Vector *,
			                    const SG_Vector *);
__inline__ SG_Vector	SG_VectorNorm(SG_Vector);
__inline__ SG_Vector	SG_VectorNormp(const SG_Vector *);
__inline__ void		SG_VectorNormv(SG_Vector *);
__inline__ SG_Vector	SG_VectorCross(SG_Vector, SG_Vector);
__inline__ SG_Vector	SG_VectorCrossp(const SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorCrossv(SG_Vector *, const SG_Vector *,
			                const SG_Vector *);
__inline__ SG_Vector	SG_VectorNCross(SG_Vector, SG_Vector);
__inline__ SG_Vector	SG_VectorNCrossp(const SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorNCrossv(SG_Vector *, const SG_Vector *,
			                 const SG_Vector *);
__inline__ SG_Vector	SG_VectorScale(SG_Vector, SG_Real);
__inline__ SG_Vector	SG_VectorScalep(const SG_Vector *, SG_Real);
__inline__ void		SG_VectorScalev(SG_Vector *, SG_Real);
__inline__ SG_Vector	SG_VectorAdd(SG_Vector, SG_Vector);
__inline__ SG_Vector	SG_VectorAddp(const SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorAddv(SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorAddv3(SG_Vector *, SG_Real, SG_Real, SG_Real);
	   SG_Vector	SG_VectorAddn(int, ...);
__inline__ SG_Vector	SG_VectorSub(SG_Vector, SG_Vector);
__inline__ SG_Vector	SG_VectorSubp(const SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorSubv(SG_Vector *, const SG_Vector *,
			              const SG_Vector *);
SG_Vector		SG_VectorSubn(int, ...);

__inline__ SG_Vector	SG_VectorAvg2(SG_Vector, SG_Vector);
__inline__ SG_Vector	SG_VectorAvg2p(const SG_Vector *, const SG_Vector *);
__inline__ void		SG_VectorAvg2v(SG_Vector *, const SG_Vector *,
			               const SG_Vector *);

__inline__ SG_Vector	SG_VectorTranslate(SG_Vector, SG_Real, SG_Real,
			                   SG_Real);
__inline__ SG_Vector	SG_VectorTranslatep(const SG_Vector *, SG_Real, SG_Real,
			                    SG_Real);

__inline__ SG_Vector	SG_VectorMirror(SG_Vector, int, int, int);
__inline__ SG_Vector	SG_VectorMirrorp(const SG_Vector *, int, int, int);
__inline__ SG_Vector	SG_VectorRotateX(SG_Vector, SG_Real);
__inline__ SG_Vector	SG_VectorRotateY(SG_Vector, SG_Real);
__inline__ SG_Vector	SG_VectorRotateZ(SG_Vector, SG_Real);
__inline__ SG_Vector	SG_VectorRotateEul(SG_Vector, SG_Real, SG_Real,
			                   SG_Real);
SG_Vector		SG_VectorRotate(SG_Vector, SG_Real, SG_Vector);
void			SG_VectorRotatev(SG_Vector *, SG_Real, SG_Vector);

SG_Vector	SG_ReadVector(AG_Netbuf *);
void		SG_ReadVectorv(AG_Netbuf *, SG_Vector *);
void		SG_WriteVector(AG_Netbuf *, SG_Vector *);
SG_Vector	SG_ReadVectorf(AG_Netbuf *);
void		SG_ReadVectorfv(AG_Netbuf *, SG_Vector *);
void		SG_WriteVectorf(AG_Netbuf *, SG_Vector *);

__inline__ SG_Real	SG_VectorVectorAngle(SG_Vector, SG_Vector);
__END_DECLS

