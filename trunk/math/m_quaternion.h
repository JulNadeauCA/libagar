/*	Public domain	*/

__BEGIN_DECLS
M_Quaternion	M_QuaternionMultIdentity(void);
M_Quaternion	M_QuaternionAddIdentity(void);
void		M_QuaternionpToAxisAngle(const M_Quaternion *, M_Vector3 *,
		                         M_Real *);
void		M_QuaternionpToAxisAngle3(const M_Quaternion *, M_Real *,
		                          M_Real *, M_Real *, M_Real *);
M_Quaternion	M_QuaternionFromAxisAngle(M_Vector3, M_Real);
M_Quaternion	M_QuaternionFromAxisAngle3(M_Real, M_Real, M_Real, M_Real);
void		M_QuaternionpFromAxisAngle(M_Quaternion *, M_Vector3, M_Real);
void		M_QuaternionpFromAxisAngle3(M_Quaternion *, M_Real, M_Real,
		                            M_Real, M_Real);
void		M_QuaternionFromEulv(M_Quaternion *, M_Real, M_Real, M_Real);
M_Quaternion	M_QuaternionFromEul(M_Real, M_Real, M_Real);
void		M_QuaternionToMatrix44(M_Matrix44 *, const M_Quaternion *);
M_Quaternion	M_QuaternionConj(M_Quaternion);
M_Quaternion	M_QuaternionConjp(const M_Quaternion *);
void		M_QuaternionConjv(M_Quaternion *);
M_Quaternion	M_QuaternionScale(M_Quaternion, M_Real);
M_Quaternion	M_QuaternionScalep(const M_Quaternion *, M_Real);
void		M_QuaternionScalev(M_Quaternion *, M_Real);
M_Quaternion	M_QuaternionConcat(const M_Quaternion *, const M_Quaternion *);
M_Quaternion	M_QuaternionMult(M_Quaternion, M_Quaternion);
M_Quaternion	M_QuaternionMultp(const M_Quaternion *, const M_Quaternion *);
void		M_QuaternionMultv(M_Quaternion *, const M_Quaternion *,
		                  const M_Quaternion *);
#define		M_QuaternionMult3(a,b,c) \
		M_QuaternionMult((b),M_QuaternionMult((a),(c)))
#define		M_QuaternionMult3v(r,a,b,c) \
		M_QuaternionMultv((r),M_QuaternionMult((a),(b)),(c))
M_Quaternion	M_QuaternionNorm(M_Quaternion);
M_Quaternion	M_QuaternionNormp(const M_Quaternion *);
void		M_QuaternionNormv(M_Quaternion *);
M_Quaternion	M_QuaternionInverse(M_Quaternion);
M_Quaternion	M_QuaternionInversep(const M_Quaternion *);
void		M_QuaternionInversev(M_Quaternion *);
M_Quaternion	M_QuaternionSLERP(M_Quaternion, M_Quaternion, M_Real);
M_Quaternion	M_QuaternionSLERPp(const M_Quaternion *, const M_Quaternion *,
		                   M_Real);
M_Quaternion	M_ReadQuaternion(AG_DataSource *);
void		M_WriteQuaternion(AG_DataSource *, M_Quaternion *);
__END_DECLS
