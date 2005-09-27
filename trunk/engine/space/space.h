/*	$Csoft: space.h,v 1.4 2005/09/20 13:46:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SPACE_SPACE_H_
#define _AGAR_SPACE_SPACE_H_
#include "begin_code.h"

typedef struct ag_coord {
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
} AG_Coord;

typedef struct ag_quaternion {
	double x, y, z, w;
} AG_Quaternion;

typedef struct ag_space {
	struct ag_object obj;
	pthread_mutex_t	lock;
	TAILQ_HEAD(, ag_actor) actors;
} AG_Space;

#define AG_SPACE(ob) ((AG_Space *)(ob))

__BEGIN_DECLS
AG_Space *AG_SpaceNew(void *, const char *);
void	  AG_SpaceInit(void *, const char *, const char *, const void *);
void	  AG_SpaceReinit(void *);
void	  AG_SpaceDestroy(void *);
int	  AG_SpaceLoad(void *, AG_Netbuf *);
int	  AG_SpaceSave(void *, AG_Netbuf *);
int	  AG_SpaceAttach(void *, void *);
void	  AG_SpaceDetach(void *, void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_SPACE_SPACE_H_ */
