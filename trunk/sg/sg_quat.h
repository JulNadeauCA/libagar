/*	Public domain	*/

__BEGIN_DECLS
SG_Quat	SG_QuatMultIdentity(void);
SG_Quat	SG_QuatAddIdentity(void);

void	SG_QuatpToAxisAngle(const SG_Quat *, SG_Vector *, SG_Real *);
void	SG_QuatpToAxisAngle3(const SG_Quat *, SG_Real *, SG_Real *, SG_Real *,
	                     SG_Real *);

SG_Quat	SG_QuatFromAxisAngle(SG_Vector, SG_Real);
SG_Quat	SG_QuatFromAxisAngle3(SG_Real, SG_Real, SG_Real, SG_Real);
void	SG_QuatpFromAxisAngle(SG_Quat *, SG_Vector, SG_Real);
void	SG_QuatpFromAxisAngle3(SG_Quat *, SG_Real, SG_Real, SG_Real, SG_Real);
void	SG_QuatFromEulv(SG_Quat *, SG_Real, SG_Real, SG_Real);
SG_Quat	SG_QuatFromEul(SG_Real, SG_Real, SG_Real);

void	SG_QuatToMatrix(SG_Matrix *, const SG_Quat *);

__inline__ SG_Quat SG_QuatConj(SG_Quat);
__inline__ SG_Quat SG_QuatConjp(const SG_Quat *);
__inline__ void	   SG_QuatConjv(SG_Quat *);

__inline__ SG_Quat SG_QuatScale(SG_Quat, SG_Real);
__inline__ SG_Quat SG_QuatScalep(const SG_Quat *, SG_Real);
__inline__ void	   SG_QuatScalev(SG_Quat *, SG_Real);

SG_Quat		   SG_QuatConcat(const SG_Quat *, const SG_Quat *);
__inline__ SG_Quat SG_QuatMult(SG_Quat, SG_Quat);
SG_Quat		   SG_QuatMultp(const SG_Quat *, const SG_Quat *);
void		   SG_QuatMultv(SG_Quat *, const SG_Quat *, const SG_Quat *);
#define		   SG_QuatMult3(a,b,c) SG_QuatMult((b),SG_QuatMult((a),(c)))
#define		   SG_QuatMult3v(r,a,b,c) SG_QuatMultv((r),SG_QuatMult((a),(b)),(c))

__inline__ SG_Quat SG_QuatNorm(SG_Quat);
__inline__ SG_Quat SG_QuatNormp(const SG_Quat *);
__inline__ void	   SG_QuatNormv(SG_Quat *);

__inline__ SG_Quat SG_QuatInverse(SG_Quat);
__inline__ SG_Quat SG_QuatInversep(const SG_Quat *);
__inline__ void	   SG_QuatInversev(SG_Quat *);

__inline__ SG_Quat SG_QuatSLERP(SG_Quat, SG_Quat, SG_Real);
SG_Quat		   SG_QuatSLERPp(const SG_Quat *, const SG_Quat *, SG_Real);

__inline__ SG_Quat	SG_ReadQuat(AG_Netbuf *);
__inline__ void		SG_ReadQuatv(AG_Netbuf *, SG_Quat *);
__inline__ void		SG_WriteQuat(AG_Netbuf *, SG_Quat *);
__END_DECLS
