/*	$Csoft: gobject.h,v 1.5 2005/08/14 01:03:14 vedge Exp $	*/
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
	int (*keydown)(void *, int ks, int km);
	int (*keyup)(void *, int ks, int km);
	int (*mousemotion)(void *, int x, int y, int xrel, int yrel, int btn);
	int (*mousebuttondown)(void *, int x, int y, int btn);
	int (*mousebuttonup)(void *, int x, int y, int btn);
	void (*joyaxis)(void *, int dev, int axis, int value);
	void (*joyball)(void *, int dev, int ball, int xrel, int yrel);
	void (*joyhat)(void *, int dev, int hat, int value);
	void (*joybutton)(void *, int dev, int button, int state);
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
			int xmot, ymot;		/* Motion offset */
			int da, dv;		/* Direction (CW degs from W) */
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

int go_map_sprite(void *, int, int, int, void *, const char *);
void go_unmap_sprite(void *);
__inline__ int go_set_sprite(void *, int, int, int, void *, const char *);
void go_move_sprite(void *, int, int);
int go_walkable(void *, int, int);

#ifdef EDITION
void gobject_edit(struct gobject *, void *);
#endif /* EDITION */
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_GOBJECT_H_ */
