/*	$Csoft: sketch.h,v 1.3 2005/03/05 12:13:49 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_SKETCH_H_
#define _AGAR_RG_SKETCH_H_
#include "begin_code.h"

#define SKETCH_NAME_MAX	32

enum sketch_mod_type {
	SKETCH_VERTEX_DISPLACEMENT
};
struct sketch_mod {
	enum sketch_mod_type type;
	struct vg_vertex *vtx;		/* Modified vertex */
	struct vg_vertex pvtx;		/* Previous value */
};
struct sketch_undoblk {
	struct sketch_mod *mods;	/* Undoable modifications */
	u_int		  nmods;
};

struct sketch {
	char name[SKETCH_NAME_MAX];
	int flags;
	struct tileset *ts;
	struct vg *vg;
	u_int nrefs;
	float h, s, v, a;
	struct sketch_undoblk *ublks;
	u_int nublks, curblk;
	TAILQ_ENTRY(sketch) sketches;
};

__BEGIN_DECLS
void sketch_init(struct sketch *, struct tileset *, int);
void sketch_destroy(struct sketch *);
int  sketch_load(struct sketch *, struct netbuf *);
void sketch_save(struct sketch *, struct netbuf *);
void sketch_scale(struct sketch *, int, int, float);
struct window *sketch_edit(struct tileview *, struct tile_element *);

void sketch_keydown(struct tileview *, struct tile_element *, int, int);
void sketch_keyup(struct tileview *, struct tile_element *, int, int);
int  sketch_mousewheel(struct tileview *, struct tile_element *, int);
void sketch_mousebuttondown(struct tileview *, struct tile_element *, double,
	                    double, int);
void sketch_mousebuttonup(struct tileview *, struct tile_element *, double,
		          double, int);
void sketch_mousemotion(struct tileview *, struct tile_element *, double,
	                double, double, double, int);

void sketch_begin_undoblk(struct sketch *);
void sketch_undo(struct tileview *, struct tile_element *);
void sketch_redo(struct tileview *, struct tile_element *);

void sketch_open_menu(struct tileview *, int, int);
void sketch_close_menu(struct tileview *);
struct toolbar *sketch_toolbar(struct tileview *, struct tile_element *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_SKETCH_H_ */
