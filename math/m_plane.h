/*	Public domain	*/

__BEGIN_DECLS
M_Plane M_PlaneFromPts(M_Vector3, M_Vector3, M_Vector3);
M_Plane	M_PlaneRead(AG_DataSource *_Nonnull);
void	M_PlaneWrite(AG_DataSource *_Nonnull, M_Plane *_Nonnull);
M_Real	M_PlaneVectorAngle(M_Plane, M_Vector3);
__END_DECLS
