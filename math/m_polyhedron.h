/*	Public domain	*/

__BEGIN_DECLS
void M_PolyhedronInit(M_Polyhedron *_Nonnull);
void M_PolyhedronFree(M_Polyhedron *_Nonnull);
int  M_PolyhedronRead(AG_DataSource *_Nonnull, M_Polyhedron *_Nonnull);
void M_PolyhedronWrite(AG_DataSource *_Nonnull, const M_Polyhedron *_Nonnull);

Uint M_PolyhedronAddVertex(M_Polyhedron *_Nonnull, M_Vector3);
void M_PolyhedronDelVertex(M_Polyhedron *_Nonnull, Uint);
Uint M_PolyhedronAddEdge(M_Polyhedron *_Nonnull, int, int);
void M_PolyhedronDelEdge(M_Polyhedron *_Nonnull, Uint);
Uint M_PolyhedronAddFacet(M_Polyhedron *_Nonnull, Uint, const Uint *_Nonnull);
void M_PolyhedronDelFacet(M_Polyhedron *_Nonnull, Uint);
__END_DECLS
