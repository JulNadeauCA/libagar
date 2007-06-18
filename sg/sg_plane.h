/*	Public domain	*/

typedef struct sg_plane {
	SG_Real a, b, c, d;
} SG_Plane;

__BEGIN_DECLS
SG_Plane	 SG_PlaneFromNormal(SG_Vector, SG_Real);
SG_Plane	 SG_PlaneFrom3Pts(SG_Vector, SG_Vector, SG_Vector);
SG_Plane	 SG_PlaneAtDistance(SG_Plane, SG_Real);

SG_Plane	 SG_ReadPlane(AG_Netbuf *);
void		 SG_WritePlane(AG_Netbuf *, SG_Plane *);
int		 SG_ValidPlane(SG_Plane);

__inline__ SG_Vector	SG_PlaneNorm(SG_Plane);
__inline__ SG_Vector	SG_PlaneNormp(const SG_Plane *);
__inline__ SG_Real	SG_VectorPlaneAngle(SG_Vector, SG_Plane);
__END_DECLS
