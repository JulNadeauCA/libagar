/*	$Csoft: space.h,v 1.1 2004/11/21 11:15:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PHYS_SPACE_H_
#define _AGAR_PHYS_SPACE_H_
#include "begin_code.h"

struct coords {
	enum {
		COORDS_CARTESIAN,
		COORDS_SPHERICAL,
		COORDS_CYLINDRICAL,
		COORDS_PARABOLIC,
		COORDS_MAP
	} space;
	union {
		struct {
			double x, y, z;
		} car;
		struct {
			double rho;		/* Radius */
			double phi;		/* Colatitude */
			double theta;		/* Azimuth */
		} sph;
		struct {
			double r;		/* Radius */
			double theta;		/* Azimuth */
			double h;		/* Height */
		} cyl;
		struct {
			double eta;		/* -z+sqrt(x^2+y^2+z^2) */
			double xi;		/* z+sqrt(x^2+y^2+z^2) */
			double phi;		/* arctan(y/x) */
		} par;
		struct {
			int x, y;		/* Map coordinates */
			int xoffs, yoffs;	/* Node coordinates */
			int layer;		/* Layer# */
		} map;
	} coords;
#define c_car coords.car
#define c_sph coords.sph
#define c_cyl coords.cyl
#define c_par coords.par
#define c_map coords.map
};

struct quaternion {
	double x, y, z, w;
};

struct space_entity {
	void *obj;			/* Pointer to object instance */
	struct coords coords;		/* Coordinates of entity */
	struct quaternion dir;		/* Orientation of entity */
};

struct space {
	struct object obj;
	pthread_mutex_t	lock;
};

#define SPACE(ob) ((struct space *)(ob))

__BEGIN_DECLS
struct space	 *space_new(void *, const char *);
void		  space_init(void *, const char *, const char *, const void *);
void		  space_reinit(void *);
void		  space_destroy(void *);
struct window	 *space_edit(void *);
int		  space_load(void *, struct netbuf *);
int		  space_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PHYS_SPACE_H_ */
