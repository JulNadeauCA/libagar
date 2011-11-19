/*	Public domain	*/

__BEGIN_DECLS
M_Plane		M_PlaneFromNormal(M_Vector3, M_Real);
M_Plane		M_PlaneFromPts(M_Vector3, M_Vector3, M_Vector3);
M_Plane		M_PlaneAtDistance(M_Plane, M_Real);
M_Plane		M_PlaneRead(AG_DataSource *);
void		M_PlaneWrite(AG_DataSource *, M_Plane *);
int	 	M_PlaneIsValid(M_Plane);
M_Real		M_PlaneVectorAngle(M_Plane, M_Vector3);

/* Return normal vector of a plane */
static __inline__ M_Vector3
M_PlaneNorm(M_Plane P)
{
	M_Vector3 n;
	n.x = P.a;
	n.y = P.b;
	n.z = P.c;
	return (n);
}
static __inline__ M_Vector3
M_PlaneNormp(const M_Plane *P)
{
	M_Vector3 n;
	n.x = P->a;
	n.y = P->b;
	n.z = P->c;
	return (n);
}

__END_DECLS
