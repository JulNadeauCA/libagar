/*	Public domain	*/

/*
 * Line segment or ray in R^2 and R^3.
 */
typedef struct m_line2 {
	M_Vector2 p;		/* Origin point */
	M_Vector2 d;		/* Direction vector (unit-length) */
	M_Real t;		/* Length of line (or Inf for a halfline) */
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
 * Plane in R^3 and hyperplane in R^n.
 */
typedef struct m_plane3 {
	M_Real a, b, c, d;	/* Coefficients of plane equation */
} M_Plane3;
typedef struct m_plane {
	M_Real *c;		/* Coefficients of plane equation */
	Uint n;
} M_Plane;

#define M_PLANE3_INITIALIZER(a,b,c,d) { a,b,c,d }
#define M_PLANE_INITIALIZER { NULL, 0 }

/*
 * Triangle in R^2 and R^3.
 */
typedef struct m_triangle2 {
	M_Line2 a, b, c;
} M_Triangle2;
typedef struct m_triangle3 {
	M_Line3 a, b, c;
} M_Triangle3;

#define M_TRIANGLE2_INITIALIZER(ax,ay,bx,by,cx,cy) \
	{ M_LineFromPts2(ax,ay), M_LineFromPts2(bx,by), \
	  M_LineFromPts2(cx,cy) }
#define M_TRIANGLE3_INITIALIZER(ax,ay,az,bx,by,bz,cx,cy,cz) \
	{ M_LineFromPts3(ax,ay,az), M_LineFromPts3(bx,by,bz), \
	  M_LineFromPts3(cx,cy,cz) }

/*
 * Rectangle in R^2 and R^3.
 */
typedef struct m_rectangle2 {
	M_Line2 a, b, c, d;
} M_Rectangle2;
typedef struct m_rectangle3 {
	M_Line3 a, b, c, d;
} M_Rectangle3;

#define M_RECTANGLE2_INITIALIZER(ax,ay,bx,by,cx,cy,dx,dy) \
	{ M_LineFromPts2(ax,ay), M_LineFromPts2(bx,by), \
	  M_LineFromPts2(cx,cy), M_LineFromPts2(dx,dy) }
#define M_RECTANGLE_INITIALIZER(ax,ay,az,bx,by,bz,cx,cy,cz,dx,dy,dz) \
	{ M_LineFromPts3(ax,ay,az), M_LineFromPts3(bx,by,bz), \
	  M_LineFromPts3(cx,cy,cz), M_LineFromPts3(dx,dy,dz) }

/*
 * Simple polygon in R^2 and R^3 (no self-intersections, no holes).
 */
typedef struct m_polygon2 {
	M_Line2 *s;		/* Sides (must be closed) */
	Uint n;			/* Number of sides */
	Uint vn;		/* Number of vertices */
} M_Polygon2;
typedef struct m_polygon3 {
	M_Line3 *s;
	Uint n;
	Uint vn;
} M_Polygon3;

#define M_POLYGON2_INITIALIZER { NULL, 0 }
#define M_POLYGON3_INITIALIZER { NULL, 0 }

/*
 * Generic geometrical structure in R^2 and R^3.
 */
typedef enum m_geom_type {
	M_NONE,
	M_POINT,
	M_LINE,
	M_CIRCLE,
	M_SPHERE,
	M_PLANE,
	M_POLYGON,
	M_TRIANGLE,
	M_RECTANGLE
} M_GeomType;

typedef struct m_geom2 {
	M_GeomType type;
	union {
		M_Vector2    point;
		M_Line2      line;
		M_Circle2    circle;
		M_Polygon2   polygon;
		M_Triangle2  triangle;
		M_Rectangle2 rectangle;
	} g;
} M_Geom2;

typedef struct m_geom3 {
	M_GeomType type;
	union {
		M_Vector3   point;
		M_Line3     line;
		M_Circle3   circle;
		M_Sphere    sphere;
		M_Plane3    plane;
		M_Polygon3  polygon;
		M_Triangle3 triangle;
		M_Rectangle3 rectangle;
	} g;
} M_Geom3;

#ifdef _AGAR_INTERNAL
#define g_point     g.point
#define g_line      g.line
#define g_circle    g.circle
#define g_sphere    g.sphere
#define g_plane     g.plane
#define g_polygon   g.polygon
#define g_triangle  g.triangle
#define g_rectangle g.rectangle
#endif

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
#define M_POINT_SET_EMPTY { NULL, 0, 0 }

/*
 * Sets of points in both R^[2..3] and Z^[2..3].
 * This is useful with pixel coordinates in image processing.
 */
typedef struct m_point_set2i {
	M_Vector2 *p;
	int *x, *y;
	Uint n, nMax;
} M_PointSet2i;
typedef struct m_point_set3i {
	M_Vector3 *p;
	int *x, *y, *z;
	Uint n, nMax;
} M_PointSet3i;
#define M_POINT_SET2I_EMPTY { NULL, NULL, NULL, 0, 0 }
#define M_POINT_SET3I_EMPTY { NULL, NULL, NULL, NULL, 0, 0 }

#include <agar/math/m_line.h>
#include <agar/math/m_circle.h>
#include <agar/math/m_sphere.h>
#include <agar/math/m_plane.h>
#include <agar/math/m_triangle.h>
#include <agar/math/m_rectangle.h>
#include <agar/math/m_polygon.h>
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
		if (S->g[i].type == M_POLYGON)
			M_PolygonFree2(&S->g[i].g.polygon);
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
		if (S->g[i].type == M_POLYGON)
			M_PolygonFree3(&S->g[i].g.polygon);
	}
	AG_Free(S->g);
	S->g = NULL;
	S->n = 0;
}
__END_DECLS
