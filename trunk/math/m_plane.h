/*	Public domain	*/

__BEGIN_DECLS
M_Plane3	M_PlaneFromNormal3(M_Vector3, M_Real);
M_Plane3	M_PlaneFromPts3(M_Vector3, M_Vector3, M_Vector3);
M_Plane3	M_PlaneAtDistance3(M_Plane3, M_Real);
M_Plane3	M_PlaneRead3(AG_DataSource *);
void		M_PlaneWrite3(AG_DataSource *, M_Plane3 *);
int	 	M_PlaneIsValid3(M_Plane3);
M_Vector3	M_PlaneNorm3(M_Plane3);
M_Vector3	M_PlaneNorm3p(const M_Plane3 *);
M_Real		M_PlaneVectorAngle3(M_Plane3, M_Vector3);
__END_DECLS
