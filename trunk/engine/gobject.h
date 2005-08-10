/*	$Csoft: gobject.h,v 1.3 2005/08/04 07:36:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_GOBJECT_H_
#define _AGAR_GOBJECT_H_

#include <engine/map/map.h>

#include "begin_code.h"

struct gobject_ops {
	struct object_ops ops;
	void (*map)(void *, void *);
	void (*unmap)(void *, void *);
	void (*update)(void *, void *);
};

struct gobject {
	struct object obj;
	pthread_mutex_t lock;
	int flags;
#define GOBJECT_SAVED_FLAGS	0x00
	void *parent;			/* Parent object (NULL=unset) */
	enum gobject_type {
		GOBJECT_NONE,
		GOBJECT_MAP,
		GOBJECT_SCENE
	} type;
	union {
		struct {
			int x, y, l0, l1;	/* Destination position */
			int x0, y0, x1, y1;	/* Extent */
		} map;
		struct {
			double x, y, z;		/* Position */
			double dx, dy, dz;	/* Direction vector */
		} scene;
	} data;
#define g_map data.map
#define g_scene data.scene
	TAILQ_ENTRY(gobject) gobjs;
};

#define GOBJECT(ob) ((struct gobject *)(ob))
#define GOBJECT_OPS(ob) ((const struct gobject_ops *)OBJECT(ob)->ops)

__BEGIN_DECLS
void		 gobject_init(void *, const char *, const char *,
		              const struct gobject_ops *);
void		 gobject_reinit(void *);
void		 gobject_destroy(void *);
int		 gobject_load(void *, struct netbuf *);
int		 gobject_save(void *, struct netbuf *);
void		 gobject_update(void *);

struct noderef *go_map_sprite(void *, struct map *, int, int, int, void *,
                              const char *);

#ifdef EDITION
void		 gobject_edit(struct gobject *, void *);
#endif /* EDITION */
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_GOBJECT_H_ */
