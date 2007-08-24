/*	Public domain	*/

__BEGIN_DECLS
SG_Line2	SG_Line2From2Pts(SG_Vector, SG_Vector);
SG_Line		SG_LineFrom2Pts(SG_Vector, SG_Vector);
SG_Real		SG_DistanceFromLine2Pts(SG_Vector, SG_Vector, SG_Vector);

SG_Plane		SG_PlaneFromNormal(SG_Vector, SG_Real);
SG_Plane		SG_PlaneFrom3Pts(SG_Vector, SG_Vector, SG_Vector);
SG_Plane		SG_PlaneAtDistance(SG_Plane, SG_Real);
SG_Plane		SG_PlaneRead(AG_Netbuf *);
void			SG_PlaneWrite(AG_Netbuf *, SG_Plane *);
int	 		SG_PlaneIsValid(SG_Plane);
__inline__ SG_Vector	SG_PlaneNorm(SG_Plane);
__inline__ SG_Vector	SG_PlaneNormp(const SG_Plane *);
__inline__ SG_Real	SG_VectorPlaneAngle(SG_Vector, SG_Plane);
__END_DECLS
