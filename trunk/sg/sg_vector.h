/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_vector { SG_Real *v; Uint n; } SG_Vector;
typedef struct sg_vector2 { SG_Real x, y; } SG_Vector2;
typedef struct sg_vector3 { SG_Real x, y, z; } SG_Vector3;
typedef struct sg_vector4 { SG_Real x, y, z, w; } SG_Vector4;

__BEGIN_DECLS
SG_Vector *SG_VectorNew(Uint);
#define	   SG_VectorFree(v) Free((v),M_SG)

__inline__ SG_Real SG_DotProduct(const SG_Vector *, const SG_Vector *);
__inline__ SG_Real SG_DotProduct2(const SG_Vector2 *, const SG_Vector2 *);
__inline__ SG_Real SG_DotProduct3(const SG_Vector3 *, const SG_Vector3 *);
__inline__ SG_Real SG_DotProduct4(const SG_Vector4 *, const SG_Vector4 *);
__inline__ SG_Real SG_Length(const SG_Vector *);
__inline__ SG_Real SG_Length2(const SG_Vector2 *);
__inline__ SG_Real SG_Length3(const SG_Vector3 *);
__inline__ SG_Real SG_Length4(const SG_Vector4 *);
__inline__ void	   SG_Normalize(SG_Vector3 *);
__inline__ void	   SG_CrossProd(SG_Vector3 *, const SG_Vector3 *,
		                const SG_Vector3 *);
__inline__ void	   SG_CrossProdNormed(SG_Vector3 *, const SG_Vector3 *,
		                      const SG_Vector3 *);
__inline__ void	   SG_MulVector3(SG_Vector3 *, SG_Real, const SG_Vector3 *);
__inline__ void	   SG_MulVector4(SG_Vector4 *, SG_Real, const SG_Vector4 *);
__inline__ void	   SG_DivVector3(SG_Vector3 *, SG_Real, const SG_Vector3 *);
__inline__ void	   SG_DivVector4(SG_Vector4 *, SG_Real, const SG_Vector4 *);
__END_DECLS
