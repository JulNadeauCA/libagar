/*	Public domain	*/

/*
 * Line segment or ray in R^2 and R^3.
 */
typedef struct m_line2 {
	M_Vector2 p;		/* Origin point */
	M_Vector2 d;		/* Direction vector (unit-length) */
	M_Real t;		/* Length of line (or Inf) */
} M_Line2;
typedef struct m_line3 {
	M_Vector3 p;
	M_Vector3 d;
	M_Real t;
} M_Line3;

#define M_LINE2_INITIALIZER(px,py,nx,ny,t) { {px,py}, {nx,ny}, t }
#define M_LINE3_INITIALIZER(px,py,pz,nx,ny,nz,t) { {px,py,pz}, {nx,ny,nz}, t }

/*
 * Circle in R^2 and R^3.
 */
typedef struct m_circle2 {
	M_Vector2 p;		/* Origin point */
	M_Real r;		/* Radius */
} M_Circle2;
typedef struct m_circle3 {
	M_Vector3 p;
	M_Real r;
} M_Circle3;

#define M_CIRCLE2_INITIALIZER(px,py,r) { { px,py }, r }
#define M_CIRCLE3_INITIALIZER(px,py,pz,r) { { px,py,pz }, r }

/*
 * Sphere in R^3.
 */
typedef struct m_sphere {
	M_Vector3 p;		/* Origin point */
	M_Real r;		/* Radius */
} M_Sphere;

#define M_SPHERE_INITIALIZER(px,py,pz,r) { { px,py,pz }, r }

/*
 * Plane in R^3.
 */
typedef struct m_plane {
	M_Vector3 n;		/* Normal vector */
	M_Real d;		/* Distance to origin */
} M_Plane;

#define M_PLANE_INITIALIZER(a,b,c,d) { {a,b,c},d }

/*
 * Triangle in R^2 and R^3.
 */
typedef struct m_triangle2 {
	M_Vector2 a, b, c;
} M_Triangle2;
typedef struct m_triangle3 {
	M_Vector3 a, b, c;
} M_Triangle3;

#define M_TRIANGLE2_INITIALIZER(a,b,c) { a,b,c }
#define M_TRIANGLE3_INITIALIZER(a,b,c) { a,b,c }

/*
 * Rectangle in R^2 and R^3.
 */
typedef struct m_rectangle2 {
	M_Vector2 a, b, c, d;
} M_Rectangle2;
typedef struct m_rectangle3 {
	M_Vector3 a, b, c, d;
} M_Rectangle3;

#define M_RECTANGLE2_INITIALIZER(a,b,c) { a,b,c }
#define M_RECTANGLE3_INITIALIZER(a,b,c) { a,b,c }

/*
 * Polygon specified as ordered set of vertices. Polygon may be convex
 * or concave, but holes and self-intersections are not allowed.
 */
typedef struct m_polygon {
	M_Vector2 *v;		/* Vertices */
	Uint n;			/* Number of vertices */
} M_Polygon;

#define M_POLYGON_INITIALIZER { NULL, 0 }

/*
 * Polyhedron specified as boundary representation.
 * Holes and self-intersections are not allowed.
 */
typedef struct m_halfedge {
	Uint v;			/* Incident vertex index */
	Uint f;			/* Incident facet index */
	Uint oe;		/* Opposite halfedge index */
} M_Halfedge;
typedef struct m_facet {
	Uint *e;		/* Incident edge indices */
	Uint n;			/* Number of edges (>=3) */
} M_Facet;
typedef struct m_polyhedron {
	M_Vector3  *v;		/* Vertices */
	Uint       nv;
	M_Halfedge *e;		/* Edges */
	Uint       ne;
	M_Facet    *f;		/* Facets */
	Uint       nf;
} M_Polyhedron;

#define M_FACET_EDGES_MAX 255
#define M_HALFEDGE_INITIALIZER { 0, 0, 0 }
#define M_FACET_INITIALIZER { NULL, 0 }
#define M_POLYHEDRON_INITIALIZER { NULL, 0, NULL, 0, NULL, 0 }

/*
 * Generic geometrical structure in R^2 and R^3.
 */
typedef enum m_geom_type {
	/* In R^2 and R^3 */
	M_NONE,
	M_POINT,
	M_LINE,
	M_CIRCLE,
	M_TRIANGLE,
	M_RECTANGLE,
	M_POLYGON,
	/* In R^3 */
	M_PLANE,
	M_SPHERE,
	M_POLYHEDRON
} M_GeomType;

typedef struct m_geom2 {
	M_GeomType type;
	union {
		M_Vector2	point;
		M_Line2		line;
		M_Circle2	circle;
		M_Triangle2	triangle;
		M_Rectangle2	rectangle;
		M_Polygon	polygon;
	} g;
} M_Geom2;

typedef struct m_geom3 {
	M_GeomType type;
	union {
		M_Vector3	point;
		M_Line3		line;
		M_Circle3	circle;
		M_Triangle3	triangle;
		M_Rectangle3	rectangle;
		M_Polygon	polygon;
		M_Sphere	sphere;
		M_Plane		plane;
		M_Polyhedron	polyhedron;
	} g;
} M_Geom3;

/* Sets of generic geometric entities. */
typedef struct m_geom_set2 {
	M_Geom2 *g;
	Uint n;
} M_GeomSet2;
typedef struct m_geom_set3 {
	M_Geom3 *g;
	Uint n;
} M_GeomSet3;
#define M_GEOM_SET_EMPTY { NULL, 0 }

/* Sets of points in R^[2..3]. */
typedef struct m_point_set2 {
	M_Vector2 *p;
	Uint n, nMax;
} M_PointSet2;
typedef struct m_point_set3 {
	M_Vector3 *p;
	Uint n, nMax;
} M_PointSet3;
#define M_POINT_SET2_EMPTY { NULL, 0, 0 }
#define M_POINT_SET3_EMPTY { NULL, 0, 0 }
#define M_POINT_SET_EMPTY { NULL, 0, 0 }

/*
 * Sets of points in both R^[2..3] and Z^[2..3].
 * This is useful with pixel coordinates in image processing.
 */
typedef struct m_point_set2i {
	M_Real w, h;
	int *x, *y;
	Uint n, nMax;
} M_PointSet2i;
typedef struct m_point_set3i {
	M_Real w, h, d;
	int *x, *y, *z;
	Uint n, nMax;
} M_PointSet3i;
#define M_POINT_SET2I_EMPTY { 1.0, 1.0, NULL, NULL, 0, 0 }
#define M_POINT_SET3I_EMPTY { 1.0, 1.0, 1.0, NULL, NULL, NULL, 0, 0 }

#include <agar/math/m_line.h>
#include <agar/math/m_circle.h>
#include <agar/math/m_sphere.h>
#include <agar/math/m_plane.h>
#include <agar/math/m_triangle.h>
#include <agar/math/m_rectangle.h>
#include <agar/math/m_polygon.h>
#include <agar/math/m_polyhedron.h>
#include <agar/math/m_point_set.h>

__BEGIN_DECLS

/*
 * Interface to M_GeomSet2 and M_GeomSet3.
 */
static __inline__ void
M_GeomSetAdd2(M_GeomSet2 *S, const M_Geom2 *g)
{
	S->g = (M_Geom2 *)AG_Realloc(S->g, (S->n+1)*sizeof(M_Geom2));
	S->g[S->n++] = *g;
}
static __inline__ void
M_GeomSetAdd3(M_GeomSet3 *S, const M_Geom3 *g)
{
	S->g = (M_Geom3 *)AG_Realloc(S->g, (S->n+1)*sizeof(M_Geom3));
	S->g[S->n++] = *g;
}

static __inline__ void
M_GeomSetFree2(M_GeomSet2 *S)
{
	Uint i;
	for (i = 0; i < S->n; i++) {
		switch (S->g[i].type) {
		case M_POLYGON:
			M_PolygonFree(&S->g[i].g.polygon);
			break;
		default:
			break;
		}
	}
	AG_Free(S->g);
	S->g = NULL;
	S->n = 0;
}
static __inline__ void
M_GeomSetFree3(M_GeomSet3 *S)
{
	Uint i;
	for (i = 0; i < S->n; i++) {
		switch (S->g[i].type) {
		case M_POLYGON:
			M_PolygonFree(&S->g[i].g.polygon);
			break;
		case M_POLYHEDRON:
			M_PolyhedronFree(&S->g[i].g.polyhedron);
			break;
		default:
			break;
		}
	}
	AG_Free(S->g);
	S->g = NULL;
	S->n = 0;
}
__END_DECLS
