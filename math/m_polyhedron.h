/*	Public domain	*/

__BEGIN_DECLS
void		M_PolyhedronInit(M_Polyhedron *);
void		M_PolyhedronFree(M_Polyhedron *);
int             M_PolyhedronRead(AG_DataSource *, M_Polyhedron *);
void		M_PolyhedronWrite(AG_DataSource *, const M_Polyhedron *);
M_Polyhedron	M_PolyhedronFromPts(Uint, const M_Vector3 *);
Uint		M_PolyhedronAddVertex(M_Polyhedron *, M_Vector3);
void		M_PolyhedronDelVertex(M_Polyhedron *, Uint);
Uint            M_PolyhedronAddEdge(M_Polyhedron *, int, int);
void            M_PolyhedronDelEdge(M_Polyhedron *, Uint);
Uint            M_PolyhedronAddFacet(M_Polyhedron *, Uint, const Uint *);
void            M_PolyhedronDelFacet(M_Polyhedron *, Uint);
__END_DECLS
