/*	Public domain	*/

typedef struct sg_vertex {
	SG_Real s, t;			/* Texture coordinates (T2F) */
	SG_Color c;			/* Vertex color (C4F) */
	SG_Vector n;			/* Normal vector (N3F) */
	SG_Vector v;			/* Vertex position (V3F) */
	Uint flags;
#define SG_VERTEX_SELECTED	0x01
#define SG_VERTEX_HIGHLIGHTED	0x02
#define SG_VERTEX_STRIDE (sizeof(Uint)+sizeof(Uint))
} SG_Vertex;

typedef struct sg_edge {
	int v;				/* Incident vertex */
	struct sg_facet *f;		/* Incident facet */
	struct sg_edge *oe;		/* Opposite halfedge */
	SLIST_ENTRY(sg_edge) edges;
} SG_Edge;

typedef struct sg_edge_ent {
	SLIST_HEAD(,sg_edge) edges;	/* Edges in bucket */
} SG_EdgeEnt;

typedef struct sg_facet {
	SG_Edge *e[4];			/* Polygon edges */
	Uint16 n;			/* Number of edges (3 or 4) */
	Uint16 flags;
#define SG_FACET_SELECTED	0x01
#define SG_FACET_HIGHLIGHTED	0x02
	struct sg_object *obj;		/* Back pointer to object */
	SLIST_ENTRY(sg_facet) facets;
} SG_Facet;

typedef struct sg_object {
	struct sg_node node;
	Uint flags;
#define SG_OBJECT_STATIC	0x01	/* Geometry is unchanging */
	SG_Vertex *vtx;			/* Vertex array */
	Uint      nvtx;
	SG_EdgeEnt *edgetbl;		/* Edge table */
	Uint	   nedgetbl;
	SLIST_HEAD(,sg_facet) facets;	/* Facets (quads or triangles) */
	SG_Material *mat;		/* Optional material object */
} SG_Object;

typedef enum sg_extrude_mode {
	SG_EXTRUDE_REGION,	/* Create 2n edges and n+1 faces */
	SG_EXTRUDE_EDGES,	/* Create 2n edges and n faces */
	SG_EXTRUDE_VERTICES	/* Create n edges and no faces */
} SG_ExtrudeMode;

#define SGOBJECT(so) ((SG_Object *)(so))

#ifdef _AGAR_INTERNAL
#define OBJ_V(so,vnum) (SGOBJECT(so)->vtx[vnum].v)
#define OBJ_N(so,vnum) (SGOBJECT(so)->vtx[vnum].n)
#define OBJ_S(so,vnum) (SGOBJECT(so)->vtx[vnum].s)
#define OBJ_T(so,vnum) (SGOBJECT(so)->vtx[vnum].t)

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
#endif /* _AGAR_INTERNAL */

#define SG_FOREACH_EDGE(edge, i, so) \
	for ((i) = 0; (i) < (so)->nedgetbl; (i)++) \
		SLIST_FOREACH(edge, &(so)->edgetbl[i].edges, edges)
#define SG_FOREACH_FACET(fct, so) \
	SLIST_FOREACH((fct), &(so)->facets, facets)

__BEGIN_DECLS
extern SG_NodeOps sgObjectOps;

SG_Object *SG_ObjectNew(void *, const char *);
void	   SG_ObjectInit(void *, const char *);
void	   SG_ObjectDestroy(void *);
int	   SG_ObjectLoad(void *, AG_DataSource *);
int	   SG_ObjectSave(void *, AG_DataSource *);
void	   SG_ObjectDraw(void *, SG_View *);
void	   SG_ObjectMenuInstance(void *, AG_MenuItem *, SG_View *);
void	   SG_ObjectMenuClass(SG *, AG_MenuItem *, SG_View *);

void	   SG_VertexInit(SG_Vertex *);
int	   SG_VertexNew(void *, const SG_Vector);
int	   SG_VertexNewv(void *, const SG_Vector *);
int	   SG_VertexNewvn(void *, const SG_Vector *, const SG_Vector *);
int	   SG_VertexNew2(void *, SG_Real, SG_Real);
int	   SG_VertexNew3(void *, SG_Real, SG_Real, SG_Real);
int	   SG_VertexNewCopy(void *, const SG_Vertex *);

void	   SG_EdgeRehash(void *, Uint);
SG_Edge	  *SG_EdgeFindByVtx(void *, int, int);
void	   SG_EdgeInsert(SG_Object *, int, int);
SG_Edge	  *SG_Edge2(void *, int, int);

SG_Facet  *SG_FacetNew(void *, int);
void       SG_FacetDelete(SG_Facet *);
#define    SG_Facet3(so) SG_FacetNew((so),3)
#define    SG_Facet4(so) SG_FacetNew((so),4)
SG_Facet  *SG_FacetFromTri3(void *, int, int, int);
SG_Facet  *SG_FacetFromQuad4(void *, int, int, int, int);
int	   SG_FacetExtrude(void *, SG_Facet *, SG_Vector, SG_ExtrudeMode);

SG_Vector SG_FacetNormal(SG_Object *, SG_Facet *);
SG_Real   SG_FacetArea(SG_Object *, SG_Facet *);
SG_Real   SG_FacetAreaSigned(SG_Object *, SG_Facet *);
SG_Vector SG_FacetCentroid(SG_Object *, SG_Facet *);

Uint8	 *SG_ObjectEdgeMatrix(SG_Object *, Uint *);
Uint8	 *SG_ObjectFacetMatrix(SG_Object *, Uint *);
int	  SG_ObjectCheckConnectivity(void *);
int	  SG_ObjectNormalize(void *);
Uint	  SG_ObjectConvQuadsToTriangles(void *);
void	  SG_ObjectFreeGeometry(void *);

void	  SG_ObjectEdit(void *, struct ag_widget *, SG_View *);
__END_DECLS
