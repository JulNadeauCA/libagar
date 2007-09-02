/*	Public domain	*/

#define SG_VECTOR2(x,y) SG_GetVector2((x),(y))
#define SG_VECTOR(x,y,z) SG_GetVector((x),(y),(z))
#define SG_VECTOR4(x,y) SG_GetVector4((x),(y),(z),(w))
#define SG_MIRROR2(v) SG_VectorMirror2((v),1,1)
#define SG_MIRROR(v) SG_VectorMirror((v),1,1,1)
#define SG_MIRROR4(v) SG_VectorMirror4((v),1,1,1,1)

#define SG_Vec0 SG_GetZeroVector()
#define SG_VecI (SG_VECTOR(1.0,0.0,0.0))
#define SG_VecJ (SG_VECTOR(0.0,1.0,0.0))
#define SG_VecK (SG_VECTOR(0.0,0.0,1.0))
#define SG_Vec20 SG_GetZeroVector2()
#define SG_Vec2I (SG_VECTOR2(1.0,0.0))
#define SG_Vec2J (SG_VECTOR2(0.0,1.0))
#define SG_Vec40 SG_GetZeroVector4()
#define SG_Vec4I (SG_VECTOR4(1.0,0.0,0.0,0.0))
#define SG_Vec4J (SG_VECTOR4(0.0,1.0,0.0,0.0))
#define SG_Vec4K (SG_VECTOR4(0.0,0.0,1.0,0.0))
#define SG_Vec4L (SG_VECTOR4(0.0,0.0,0.0,0.0))

#ifdef _AGAR_INTERNAL
#define Vec0 SG_Vec0
#define VecI SG_VecI
#define VecJ SG_VecJ
#define VecK SG_VecK
#endif

#define SG_VECTOR2_INITIALIZER(x,y)	{ (x),(y) }
#define SG_VECTOR_INITIALIZER(x,y,z)	{ (x),(y),(z) }
#define SG_VECTOR4_INITIALIZER(x,y,z,w)	{ (x),(y),(z),(w) }
#define SG_ZERO_VECTOR2_INITIALIZER()	{ 0.0,0.0 }
#define SG_ZERO_VECTOR_INITIALIZER()	{ 0.0,0.0,0.0 }
#define SG_ZERO_VECTOR4_INITIALIZER()	{ 0.0,0.0,0.0,0.0 }

__BEGIN_DECLS
__inline__ SG_Vector2 SG_GetVector2(SG_Real, SG_Real);
__inline__ SG_Vector  SG_GetVector (SG_Real, SG_Real, SG_Real);
__inline__ SG_Vector4 SG_GetVector4(SG_Real, SG_Real, SG_Real, SG_Real);

__inline__ SG_Vector2 SG_GetZeroVector2(void);
__inline__ SG_Vector  SG_GetZeroVector (void);
__inline__ SG_Vector4 SG_GetZeroVector4(void);

__inline__ void	SG_SetVector2(SG_Vector2 *, SG_Real, SG_Real);
__inline__ void	SG_SetVector (SG_Vector  *, SG_Real, SG_Real, SG_Real);
__inline__ void	SG_SetVector4(SG_Vector4 *, SG_Real, SG_Real, SG_Real, SG_Real);

__inline__ void	SG_VectorCopy2(SG_Vector2 *, const SG_Vector2 *);
__inline__ void	SG_VectorCopy (SG_Vector  *, const SG_Vector  *);
__inline__ void	SG_VectorCopy4(SG_Vector4 *, const SG_Vector4 *);

__inline__ SG_Vector2 SG_VectorMirror2(SG_Vector2, int, int);
__inline__ SG_Vector  SG_VectorMirror (SG_Vector,  int, int, int);
__inline__ SG_Vector4 SG_VectorMirror4(SG_Vector4, int, int, int, int);
__inline__ SG_Vector2 SG_VectorMirror2p(const SG_Vector2 *, int, int);
__inline__ SG_Vector  SG_VectorMirrorp (const SG_Vector  *, int, int, int);
__inline__ SG_Vector4 SG_VectorMirror4p(const SG_Vector4 *, int, int, int, int);

__inline__ SG_Real SG_VectorLen2(SG_Vector2);
__inline__ SG_Real SG_VectorLen (SG_Vector );
__inline__ SG_Real SG_VectorLen4(SG_Vector4);
__inline__ SG_Real SG_VectorLen2p(const SG_Vector2 *);
__inline__ SG_Real SG_VectorLenp (const SG_Vector  *);
__inline__ SG_Real SG_VectorLen4p(const SG_Vector4 *);

__inline__ SG_Real SG_VectorDot2(SG_Vector2, SG_Vector2);
__inline__ SG_Real SG_VectorDot (SG_Vector , SG_Vector );
__inline__ SG_Real SG_VectorDot4(SG_Vector4, SG_Vector4);
__inline__ SG_Real SG_VectorDot2p(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Real SG_VectorDotp (const SG_Vector  *, const SG_Vector  *);
__inline__ SG_Real SG_VectorDot4p(const SG_Vector4 *, const SG_Vector4 *);

__inline__ SG_Real SG_VectorDistance2(SG_Vector2, SG_Vector2);
__inline__ SG_Real SG_VectorDistance (SG_Vector,  SG_Vector );
__inline__ SG_Real SG_VectorDistance4(SG_Vector4, SG_Vector4);
__inline__ SG_Real SG_VectorDistance2p(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Real SG_VectorDistancep (const SG_Vector  *, const SG_Vector  *);
__inline__ SG_Real SG_VectorDistance4p(const SG_Vector4 *, const SG_Vector4 *);

__inline__ SG_Vector2 SG_VectorNorm2(SG_Vector2);
__inline__ SG_Vector  SG_VectorNorm (SG_Vector );
__inline__ SG_Vector4 SG_VectorNorm4(SG_Vector4);
__inline__ SG_Vector2 SG_VectorNorm2p(const SG_Vector2 *);
__inline__ SG_Vector  SG_VectorNormp (const SG_Vector  *);
__inline__ SG_Vector4 SG_VectorNorm4p(const SG_Vector4 *);
__inline__ void	      SG_VectorNorm2v(SG_Vector2 *);
__inline__ void	      SG_VectorNormv (SG_Vector  *);
__inline__ void	      SG_VectorNorm4v(SG_Vector4 *);

__inline__ SG_Vector  SG_VectorCross(SG_Vector, SG_Vector);
__inline__ SG_Vector4 SG_VectorCross4(SG_Vector4, SG_Vector4, SG_Vector4);
__inline__ SG_Vector  SG_VectorCrossp(const SG_Vector *, const SG_Vector *);
__inline__ SG_Vector4 SG_VectorCross4p(const SG_Vector4 *, const SG_Vector4 *,
                                       const SG_Vector4 *);
__inline__ SG_Vector  SG_VectorNormCross(SG_Vector, SG_Vector);
__inline__ SG_Vector4 SG_VectorNormCross4(SG_Vector4, SG_Vector4, SG_Vector4);
__inline__ SG_Vector  SG_VectorNormCrossp(const SG_Vector *, const SG_Vector *);
__inline__ SG_Vector4 SG_VectorNormCross4p(const SG_Vector4 *,
                                           const SG_Vector4 *,
                                           const SG_Vector4 *);

__inline__ SG_Vector2 SG_VectorScale2(SG_Vector2, SG_Real);
__inline__ SG_Vector  SG_VectorScale (SG_Vector,  SG_Real);
__inline__ SG_Vector4 SG_VectorScale4(SG_Vector4, SG_Real);
__inline__ SG_Vector2 SG_VectorScale2p(const SG_Vector2 *, SG_Real);
__inline__ SG_Vector  SG_VectorScalep (const SG_Vector  *, SG_Real);
__inline__ SG_Vector4 SG_VectorScale4p(const SG_Vector4 *, SG_Real);
__inline__ void       SG_VectorScale2v(SG_Vector2 *, SG_Real);
__inline__ void       SG_VectorScalev (SG_Vector  *, SG_Real);
__inline__ void       SG_VectorScale4v(SG_Vector4 *, SG_Real);

__inline__ SG_Vector2 SG_VectorAdd2(SG_Vector2, SG_Vector2);
__inline__ SG_Vector  SG_VectorAdd (SG_Vector,  SG_Vector );
__inline__ SG_Vector4 SG_VectorAdd4(SG_Vector4, SG_Vector4);
__inline__ SG_Vector2 SG_VectorAdd2p(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Vector  SG_VectorAddp (const SG_Vector  *, const SG_Vector  *);
__inline__ SG_Vector4 SG_VectorAdd4p(const SG_Vector4 *, const SG_Vector4 *);
__inline__ void	      SG_VectorAdd2v(SG_Vector2 *, const SG_Vector2 *);
__inline__ void	      SG_VectorAddv (SG_Vector  *, const SG_Vector  *);
__inline__ void	      SG_VectorAdd4v(SG_Vector4 *, const SG_Vector4 *);
SG_Vector2	      SG_VectorAdd2n(int, ...);
SG_Vector	      SG_VectorAddn (int, ...);
SG_Vector4	      SG_VectorAdd4n(int, ...);

__inline__ SG_Vector2 SG_VectorSub2(SG_Vector2, SG_Vector2);
__inline__ SG_Vector  SG_VectorSub (SG_Vector,  SG_Vector);
__inline__ SG_Vector4 SG_VectorSub4(SG_Vector4, SG_Vector4);
__inline__ SG_Vector2 SG_VectorSub2p(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Vector  SG_VectorSubp (const SG_Vector  *, const SG_Vector  *);
__inline__ SG_Vector4 SG_VectorSub4p(const SG_Vector4 *, const SG_Vector4 *);
__inline__ void	      SG_VectorSub2v(SG_Vector2 *, const SG_Vector2 *);
__inline__ void	      SG_VectorSubv (SG_Vector  *, const SG_Vector  *);
__inline__ void	      SG_VectorSub4v(SG_Vector4 *, const SG_Vector4 *);
SG_Vector2	      SG_VectorSub2n(int, ...);
SG_Vector	      SG_VectorSubn (int, ...);
SG_Vector4	      SG_VectorSub4n(int, ...);

__inline__ SG_Vector2 SG_VectorAvg2(SG_Vector2, SG_Vector2);
__inline__ SG_Vector  SG_VectorAvg (SG_Vector,  SG_Vector );
__inline__ SG_Vector4 SG_VectorAvg4(SG_Vector4, SG_Vector4);
__inline__ SG_Vector2 SG_VectorAvg2p(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Vector  SG_VectorAvgp (const SG_Vector  *, const SG_Vector  *);
__inline__ SG_Vector4 SG_VectorAvg4p(const SG_Vector4 *, const SG_Vector4 *);

__inline__ SG_Real SG_VectorVectorAngle2(SG_Vector2, SG_Vector2);
__inline__ void	   SG_VectorVectorAngle(SG_Vector, SG_Vector, SG_Real *,
                                        SG_Real *);
__inline__ void	   SG_VectorVectorAngle4(SG_Vector4, SG_Vector4, SG_Real *,
                                         SG_Real *, SG_Real *);

__inline__ SG_Vector2 SG_VectorRotate2(SG_Vector2, SG_Real);
__inline__ SG_Vector  SG_VectorRotate (SG_Vector,  SG_Real, SG_Vector );
__inline__ SG_Vector4 SG_VectorRotate4(SG_Vector4, SG_Real, SG_Vector4);
__inline__ void	      SG_VectorRotate2v(SG_Vector2 *, SG_Real);
void		      SG_VectorRotatev (SG_Vector  *, SG_Real, SG_Vector);
__inline__ void	      SG_VectorRotate4v(SG_Vector4 *, SG_Real, SG_Vector4);
__inline__ SG_Vector  SG_VectorRotateQuat(SG_Vector, SG_Quat);
__inline__ SG_Vector  SG_VectorRotateI(SG_Vector, SG_Real);
__inline__ SG_Vector  SG_VectorRotateJ(SG_Vector, SG_Real);
__inline__ SG_Vector  SG_VectorRotateK(SG_Vector, SG_Real);

__inline__ SG_Vector2 SG_VectorLERP2(SG_Vector2, SG_Vector2, SG_Real);
__inline__ SG_Vector  SG_VectorLERP (SG_Vector,  SG_Vector,  SG_Real);
__inline__ SG_Vector4 SG_VectorLERP4(SG_Vector4, SG_Vector4, SG_Real);
__inline__ SG_Vector2 SG_VectorLERP2p(SG_Vector2 *, SG_Vector2 *, SG_Real);
__inline__ SG_Vector  SG_VectorLERPp (SG_Vector  *, SG_Vector  *, SG_Real);
__inline__ SG_Vector4 SG_VectorLERP4p(SG_Vector4 *, SG_Vector4 *, SG_Real);

SG_Vector  SG_VectorElemPow(SG_Vector, SG_Real);

SG_Vector2 SG_ReadVector2(AG_Netbuf *);
SG_Vector  SG_ReadVector (AG_Netbuf *);
SG_Vector4 SG_ReadVector4(AG_Netbuf *);
void	   SG_ReadVector2v(AG_Netbuf *, SG_Vector2 *);
void	   SG_ReadVectorv (AG_Netbuf *, SG_Vector  *);
void	   SG_ReadVector4v(AG_Netbuf *, SG_Vector4 *);
void	   SG_WriteVector2(AG_Netbuf *, SG_Vector2 *);
void	   SG_WriteVector (AG_Netbuf *, SG_Vector  *);
void	   SG_WriteVector4(AG_Netbuf *, SG_Vector4 *);
SG_Vector2 SG_ReadVectorf2(AG_Netbuf *);
SG_Vector  SG_ReadVectorf (AG_Netbuf *);
SG_Vector4 SG_ReadVectorf4(AG_Netbuf *);
void	   SG_ReadVectorf2v(AG_Netbuf *, SG_Vector2 *);
void	   SG_ReadVectorfv (AG_Netbuf *, SG_Vector  *);
void	   SG_ReadVectorf4v(AG_Netbuf *, SG_Vector4 *);
void	   SG_WriteVectorf2(AG_Netbuf *, SG_Vector2 *);
void	   SG_WriteVectorf (AG_Netbuf *, SG_Vector *);
void	   SG_WriteVectorf4(AG_Netbuf *, SG_Vector4 *);
__END_DECLS

