/*	Public domain	*/

__BEGIN_DECLS
M_Plane		M_PlaneFromPts(M_Vector3, M_Vector3, M_Vector3);
M_Plane		M_PlaneRead(AG_DataSource *);
void		M_PlaneWrite(AG_DataSource *, M_Plane *);
M_Real		M_PlaneVectorAngle(M_Plane, M_Vector3);
__END_DECLS
