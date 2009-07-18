/*	Public domain	*/

__BEGIN_DECLS
M_Sphere	M_SphereRead(AG_DataSource *);
void		M_SphereWrite(AG_DataSource *, M_Sphere *);

M_Sphere	M_SphereFromPt(M_Vector3, M_Real);
M_Real		M_SpherePointDistance(M_Sphere, M_Vector3);
M_Real		M_SphereSurfaceArea(M_Sphere);
M_Real		M_SphereVolume(M_Sphere);
__END_DECLS
