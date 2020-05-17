/*	Public domain	*/

struct sg_view;

#define SG_FACET_NAME_MAX	64
#define SG_EDGE_NAME_MAX	32

typedef struct sg_vertex {
	M_Vector2 st;			/* Texture coordinates (T2F) */
	M_Color c;			/* Vertex color (C4F) */
	M_Vector3 n;			/* Normal vector (N3F) */
	M_Vector3 v;			/* Vertex position (V3F) */
	Uint flags;
#define SG_VERTEX_SELECTED	0x01
#define SG_VERTEX_HIGHLIGHTED	0x02
#define SG_VERTEX_STRIDE (sizeof(Uint)+sizeof(Uint))
	Uint8 _pad[12];
} SG_Vertex;

typedef struct sg_edge {
	Uint flags;
#define SG_EDGE_SAVED		0x01	/* For save operation */
#define SG_EDGE_SELECTED	0x02
#define SG_EDGE_HIGHLIGHTED	0x04
	int v;				/* Incident vertex */
	struct sg_facet *_Nullable f;	/* Incident facet */
	struct sg_edge *_Nonnull oe;	/* Opposite halfedge */
	AG_SLIST_ENTRY(sg_edge) edges;
} SG_Edge;
typedef struct sg_edge_ent {
	AG_SLIST_HEAD_(sg_edge) edges;	/* Edges in bucket */
} SG_EdgeEnt;

typedef struct sg_facet {
	SG_Edge *_Nullable e[4];	/* Polygon edges */
	Uint n;				/* Number of edges (3 or 4) */
	Uint flags;
#define SG_FACET_SELECTED	0x01
#define SG_FACET_HIGHLIGHTED	0x02
	struct sg_facet *_Nonnull of;		/* Opposite facet */
	struct sg_object *_Nonnull obj;		/* Back pointer to object */
	AG_SLIST_ENTRY(sg_facet) facets;	/* In facet list */
	AG_TAILQ_ENTRY(sg_facet) bsp;		/* In BSP tree */
} SG_Facet;
typedef struct sg_facet_ent {
	AG_SLIST_HEAD_(sg_facet) facets; /* Facets in bucket */
} SG_FacetEnt;

typedef struct sg_bsp_node {
	M_Plane P;				/* Plane */
	SG_Facet *_Nullable facets;		/* Facets */
	Uint               nFacets;
	int                 tag;		/* User tag */
	AG_TAILQ_HEAD_(sg_bsp_node) front;	/* Front subnodes */
	AG_TAILQ_HEAD_(sg_bsp_node) back;	/* Back subnodes */
	AG_TAILQ_ENTRY(sg_bsp_node) bsp;	/* In parent node */
} SG_BSPNode;

typedef struct sg_object {
	struct sg_node _inherit;	/* SG_Object -> SG_Node */

	Uint flags;
#define SG_OBJECT_STATIC	0x01	/* Geometry is unchanging */
#define SG_OBJECT_NODUPVERTEX	0x02	/* Check for duplicate vertices in
					   SG_VertexNew*() */
	Uint               nVtx;
	SG_Vertex *_Nonnull vtx;	/* Vertex array */
	SG_EdgeEnt *_Nonnull edgeTbl;	/* Edge table */
	Uint	            nEdgeTbl;
	Uint                 nFacetTbl;
	SG_FacetEnt *_Nonnull facetTbl;	/* Facet table */
	SG_Texture *_Nullable tex;	/* Associated texture */
	SG_BSPNode *_Nullable bsp;	/* Root BSP node */
	Uint8 _pad[8];
} SG_Object;

typedef enum sg_extrude_mode {
	SG_EXTRUDE_REGION,	/* Create 2n edges and n+1 faces */
	SG_EXTRUDE_EDGES,	/* Create 2n edges and n faces */
	SG_EXTRUDE_VERTICES	/* Create n edges and no faces */
} SG_ExtrudeMode;

#define SGOBJECT(obj)          ((SG_Object *)(obj))
#define SGCOBJECT(obj)         ((const SG_Object *)(obj))
#define SG_OBJECT_SELF()          SGOBJECT( AG_OBJECT(0,"SG_Node:SG_Object:*") )
#define SG_OBJECT_PTR(n)          SGOBJECT( AG_OBJECT((n),"SG_Node:SG_Object:*") )
#define SG_OBJECT_NAMED(n)        SGOBJECT( AG_OBJECT_NAMED((n),"SG_Node:SG_Object:*") )
#define SG_CONST_OBJECT_SELF()   SGCOBJECT( AG_CONST_OBJECT(0,"SG_Node:SG_Object:*") )
#define SG_CONST_OBJECT_PTR(n)   SGCOBJECT( AG_CONST_OBJECT((n),"SG_Node:SG_Object:*") )
#define SG_CONST_OBJECT_NAMED(n) SGCOBJECT( AG_CONST_OBJECT_NAMED((n),"SG_Node:SG_Object:*") )

#ifdef _AGAR_SG_INTERNAL
#define OBJ_V(so,vnum) (SGOBJECT(so)->vtx[vnum].v)
#define OBJ_N(so,vnum) (SGOBJECT(so)->vtx[vnum].n)
#define OBJ_ST(so,vnum) (SGOBJECT(so)->vtx[vnum].st)

#define FACET_V(so,fct,edge) (SGOBJECT(so)->vtx[(fct)->e[edge]->v].v)
#define FACET_N(so,fct,edge) (SGOBJECT(so)->vtx[(fct)->e[edge]->v].n)
#define FACET_S(so,fct,edge) (SGOBJECT(so)->vtx[(fct)->e[edge]->v].s)
#define FACET_T(so,fct,edge) (SGOBJECT(so)->vtx[(fct)->e[edge]->v].t)

#define EDGE_V(so,edge) (SGOBJECT(so)->vtx[(edge)->v].v)
#define EDGE_N(so,edge) (SGOBJECT(so)->vtx[(edge)->v].n)
#define EDGE_S(so,edge) (SGOBJECT(so)->vtx[(edge)->v].s)
#define EDGE_T(so,edge) (SGOBJECT(so)->vtx[(edge)->v].t)

/* Macros for making things explicit during forward halfedge traversals */
#define LFACE(e) ((e)->f)
#define RFACE(e) ((e)->oe->f)
#define HVTX(e) ((e)->v)
#define TVTX(e) ((e)->oe->v)
#define FV1(fct) ((fct)->e[0]->v)
#define FV2(fct) ((fct)->e[1]->v)
#define FV3(fct) ((fct)->e[2]->v)
#define FV4(fct) ((fct)->e[3]->v)
#endif /* _AGAR_SG_INTERNAL */

#define SG_FOREACH_EDGE(edge, i, so) \
	for ((i) = 0, (edge) = NULL; (i) < (so)->nEdgeTbl; (i)++) \
		AG_SLIST_FOREACH(edge, &(so)->edgeTbl[(i)].edges, edges)
#define SG_FOREACH_FACET(facet, i, so) \
	for ((i) = 0, (facet) = NULL; (i) < (so)->nFacetTbl; (i)++) \
		AG_SLIST_FOREACH(facet, &(so)->facetTbl[(i)].facets, facets)

__BEGIN_DECLS
extern SG_NodeClass sgObjectClass;

struct ag_console;
struct ag_menu_item;

SG_Object *_Nonnull SG_ObjectNew(void *_Nullable, const char *_Nullable);

void                  SG_ObjectSetTexture(void *_Nonnull, SG_Texture *_Nullable);
SG_Texture *_Nullable SG_ObjectGetTexture(void *_Nonnull);

Uint8 *_Nullable SG_ObjectEdgeMatrix(SG_Object *_Nonnull, Uint *_Nonnull);
Uint8 *_Nullable SG_ObjectFacetMatrix(SG_Object *_Nonnull, Uint *_Nonnull);

int  SG_ObjectCheckConnectivity(void *_Nonnull, struct ag_console *_Nonnull);
int  SG_ObjectNormalize(void *_Nonnull);
Uint SG_ObjectConvQuadsToTriangles(void *_Nonnull);
void SG_ObjectFreeGeometry(void *_Nonnull);
void SG_ObjectMenuInstance(void *_Nonnull, struct ag_menu_item *_Nonnull,
                           struct sg_view *_Nonnull);

void SG_VertexInit(SG_Vertex *_Nonnull);
int  SG_VertexNew(void *_Nonnull, const M_Vector3);
int  SG_VertexNewv(void *_Nonnull, const M_Vector3 *_Nonnull);
int  SG_VertexNewvn(void *_Nonnull, const M_Vector3 *_Nonnull,
                    const M_Vector3 *_Nonnull);
int  SG_VertexNew2(void *_Nonnull, M_Real,M_Real);
int  SG_VertexNew3(void *_Nonnull, M_Real,M_Real,M_Real);
int  SG_VertexNewCopy(void *_Nonnull, const SG_Vertex *_Nonnull);

int  SG_EdgeRehash(void *_Nonnull, Uint);
int  SG_FacetRehash(void *_Nonnull, Uint);

SG_Edge	*_Nonnull SG_Edge2(void *_Nonnull, int,int);
void              SG_EdgeGetName(SG_Edge *_Nonnull, char *_Nonnull, AG_Size);

SG_Facet *_Nonnull SG_FacetFromTri3(void *_Nonnull, int,int,int);
SG_Facet *_Nonnull SG_FacetFromQuad4(void *_Nonnull, int,int,int,int);

int  SG_FacetExtrude(void *_Nonnull, SG_Facet *_Nonnull, M_Vector3, SG_ExtrudeMode);
void SG_FacetDelete(SG_Facet *_Nonnull);

void      SG_FacetGetName(SG_Facet *_Nonnull, char *_Nonnull, AG_Size);
M_Real    SG_FacetArea(SG_Object *_Nonnull, SG_Facet *_Nonnull);
M_Real    SG_FacetAreaSigned(SG_Object *_Nonnull, SG_Facet *_Nonnull);
M_Vector3 SG_FacetCentroid(SG_Object *_Nonnull, SG_Facet *_Nonnull);

/* Edge hash function. Halfedge pairs share the same bucket. */
static __inline__ Uint
SG_HashEdge(SG_Object *_Nonnull so, int v1, int v2)
{
	return ((31*v1 + 31*v2) % so->nEdgeTbl);
}

/* Facet hash function. Double-sided facets share the same bucket. */
static __inline__ Uint
SG_HashTriangle(SG_Object *_Nonnull so, int v1, int v2, int v3)
{
	return (31*v1 + 31*v2 + 31*v3) % so->nFacetTbl;
}
static __inline__ Uint
SG_HashQuad(SG_Object *_Nonnull so, int v1, int v2, int v3, int v4)
{
	return (31*v1 + 31*v2 + 31*v3 + 31*v4) % so->nFacetTbl;
}
static __inline__ Uint
SG_HashFacet(SG_Object *_Nonnull so, const SG_Facet *_Nonnull f)
{
	if (f->n == 3) {
		return SG_HashTriangle(so,
		    f->e[0]->v,
		    f->e[1]->v,
		    f->e[2]->v);
	} else {
		return SG_HashQuad(so,
		    f->e[0]->v,
		    f->e[1]->v,
		    f->e[2]->v,
		    f->e[3]->v);
	}
}

/* Return the edge v1->v2, if any. */
static __inline__ SG_Edge *_Nonnull 
SG_FindEdge(void *_Nonnull obj, int v1, int v2)
{
	SG_Object *so = obj;
	SG_EdgeEnt *ent = &so->edgeTbl[SG_HashEdge(so, v1,v2)];
	SG_Edge *e;

	AG_SLIST_FOREACH(e, &ent->edges, edges) {
		if (e->v == v1 && e->oe->v == v2)
			break;
	}
	return (e);
}

/* Return the triangular facet v1,v2,v3 if any. */
static __inline__ SG_Facet *_Nonnull 
SG_FindTriangle(void *_Nonnull obj, int v1, int v2, int v3)
{
	SG_Object *so = obj;
	SG_FacetEnt *fe = &so->facetTbl[SG_HashTriangle(so,v1,v2,v3)];
	SG_Facet *f;

	AG_SLIST_FOREACH(f, &fe->facets, facets) {
		if (f->e[0]->v == v1 &&
		    f->e[1]->v == v2 &&
		    f->e[2]->v == v3)
			break;
	}
	return (f);
}

/* Return the quad facet v1,v2,v3,v4 if any. */
static __inline__ SG_Facet *_Nonnull 
SG_FindQuad(void *_Nonnull obj, int v1, int v2, int v3, int v4)
{
	SG_Object *so = obj;
	SG_FacetEnt *fe = &so->facetTbl[SG_HashQuad(so,v1,v2,v3,v4)];
	SG_Facet *f;

	AG_SLIST_FOREACH(f, &fe->facets, facets) {
		if (f->e[0]->v == v1 &&
		    f->e[1]->v == v2 &&
		    f->e[2]->v == v3 &&
		    f->e[3]->v == v4)
			break;
	}
	return (f);
}

/*
 * Compute the normal for a given polygon facet.
 * The object must be locked.
 */
static __inline__ M_Vector3
SG_FacetNormal(SG_Object *_Nonnull so, const SG_Facet *_Nonnull f)
{
	M_Vector3 v0, v1, v2;

	v1 = so->vtx[f->e[2]->v].v;		/* Right-hand rule */
	v0 = so->vtx[f->e[1]->v].v;
	v2 = so->vtx[f->e[0]->v].v;
	return M_VecNormCross3(M_VecSub3(v0,v1),
	                       M_VecSub3(v0,v2));
}

#ifdef _AGAR_SG_INTERNAL
/* Specify a vertex in immediate mode. */
static __inline__ void
GL_VertexTN(const SG_Vertex *_Nonnull vtx)
{
	GL_TexCoord2(vtx->st.x, vtx->st.y);
	GL_Normal3v(&vtx->n);
	GL_Color4v(&vtx->c);
	GL_Vertex3v(&vtx->v);
}
#endif /* _AGAR_SG_INTERNAL */
__END_DECLS
