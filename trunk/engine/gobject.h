/*	$Csoft: gobject.h,v 1.2 2005/05/01 00:46:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_GOBJECT_H_
#define _AGAR_GOBJECT_H_

#include <engine/map/map.h>

#include "begin_code.h"

struct gobject_ops {
	struct object_ops ops;
	void (*specify)(void *, void *, ...);
	void (*update)(void *, void *);
};

struct gobject {
	struct object obj;
	pthread_mutex_t lock;
	enum gobject_type {
		GOBJECT_NONE,
		GOBJECT_MAP,
		GOBJECT_SCENE
	} type;
	union {
		struct {
			struct map *dm;		/* Destination map */
			int x, y, layer;	/* Destination position */
			struct map fm;		/* Map of fragments */
		} map;
		struct {
			double x, y, z;		/* Position */
			double dx, dy, dz;	/* Direction vector */
		} scene;
	} data;
#define g_map data.map
#define g_scene data.scene
};

#define GOBJECT(ob) ((struct gobject *)(ob))

__BEGIN_DECLS
void		 gobject_init(void *, const char *, const char *,
		              const struct gobject_ops *);
void		 gobject_reinit(void *);
void		 gobject_destroy(void *);
struct window	*gobject_edit(void *);
int		 gobject_load(void *, struct netbuf *);
int		 gobject_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_GOBJECT_H_ */
