/*	$Csoft: mapview.c,v 1.22 2002/09/06 01:26:41 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/physics.h>
#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include "mapedit.h"
#include "mapview.h"

#include "tool/tool.h"

static const struct widget_ops mapview_ops = {
	{
		widget_destroy
		NULL,		/* load */
		NULL		/* save */
	},
	mapview_draw,
	NULL		/* update */
};

enum {
	BORDER_COLOR,
	GRID_COLOR,
	CURSOR_COLOR,
	TILE_SELECTION_COLOR
};

static SDL_TimerID zoomin_timer, zoomout_timer;

static void	mapview_event(int, union evarg *);
static void	mapview_scaled(int, union evarg *);
static void	mapview_lostfocus(int, union evarg *);
static void	mapview_scroll(struct mapview *, int);
static void	mapview_stop_zoom(struct mapview *);

static __inline__ void	draw_node_scaled(struct mapview *, SDL_Surface *,
			    int, int);
static __inline__ void	draw_node_props(struct mapview *, struct node *,
			    int, int);

struct mapview *
mapview_new(struct region *reg, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, m, flags, rw, rh);
	
	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, mv);
	pthread_mutex_unlock(&reg->win->lock);

	return (mv);
}

void
mapview_init(struct mapview *mv, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	widget_init(&mv->wid, "mapview", "widget", &mapview_ops, rw, rh);
	WIDGET(mv)->flags |= WIDGET_MOUSEOUT;

	mv->flags = flags;
	mv->med = med;
	mv->map = m;
	mv->mx = m->defx;
	mv->my = m->defy;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;
	mv->tilew = TILEW;
	mv->tileh = TILEH;
	mv->zoom = 100;
	mv->prop_style = -1; 

	widget_map_color(mv, BORDER_COLOR, "mapview-border",
	    200, 200, 200);
	widget_map_color(mv, GRID_COLOR, "mapview-grid",
	    100, 100, 100);
	widget_map_color(mv, CURSOR_COLOR, "mapview-cursor",
	    100, 100, 100);
	widget_map_color(mv, TILE_SELECTION_COLOR, "mapview-tile-selection",
	    0, 200, 0);

	event_new(mv, "widget-scaled", 0,
	    mapview_scaled, NULL);
	event_new(mv, "widget-lostfocus", 0,
	    mapview_lostfocus, NULL);
	event_new(mv, "window-mousebuttonup", 0,
	    mapview_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(mv, "window-mousebuttondown", 0,
	    mapview_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(mv, "window-keyup", 0,
	    mapview_event, "%i", WINDOW_KEYUP);
	event_new(mv, "window-keydown", 0,
	    mapview_event, "%i", WINDOW_KEYDOWN);
	event_new(mv, "window-mousemotion", 0,
	    mapview_event, "%i", WINDOW_MOUSEMOTION);
	event_new(mv, "window-mouseout", 0,
	    mapview_event, "%i", WINDOW_MOUSEOUT);
}

static __inline__ void
draw_node_scaled(struct mapview *mv, SDL_Surface *s, int rx, int ry)
{
	int x, y;
	Uint32 col = 0;
	Uint8 *src, r1, g1, b1, a1;

	SDL_LockSurface(view->v);
	for (y = 0; y < mv->tileh && ry+y < WIDGET(mv)->h; y++) {
		for (x = 0; x < mv->tilew && rx+x < WIDGET(mv)->w; x++) {
			src = (Uint8 *)s->pixels +
			    (y*TILEH/mv->tileh)*s->pitch +
			    (x*TILEW/mv->tilew)*s->format->BytesPerPixel;

			SDL_GetRGBA(*(Uint32 *)src, s->format,
			    &r1, &g1, &b1, &a1);
			col = SDL_MapRGB(view->v->format, r1, g1, b1);
			if (a1 > 200) {
				WIDGET_PUT_PIXEL(mv, rx+x, ry+y, col);
			}
		}
	}
	SDL_UnlockSurface(view->v);
}

static __inline__ void
draw_node_props(struct mapview *mv, struct node *node, int rx, int ry)
{
	if (mv->prop_style > 0) {
		WIDGET_DRAW(mv, SPRITE(mv->med, mv->prop_style), rx, ry);
	}

	if (node->flags & NODE_BLOCK) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_BLOCK), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_WALK) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_WALK), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_CLIMB) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_CLIMB), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
	
	if (node->flags & NODE_BIO) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_BIO), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_REGEN) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_REGEN), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
	
	if (node->flags & NODE_SLOW) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_SLOW), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_HASTE) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_HASTE), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}

	if (node->flags & NODE_ORIGIN) {
		WIDGET_DRAW(mv, SPRITE(mv->med, MAPEDIT_ORIGIN), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
}

void
mapview_draw(void *p)
{
	struct mapview *mv = p;
	struct map *m = mv->map;
	struct node *node;
	struct noderef *nref;
	struct mapedit *med = mv->med;
	int mx, my, rx, ry;
	int nsprites;
	SDL_Rect clip;

	clip.x = WIDGET_ABSX(mv);
	clip.y = WIDGET_ABSY(mv);
	clip.w = WIDGET(mv)->w;
	clip.h = WIDGET(mv)->h;
	SDL_SetClipRect(view->v, &clip);

	for (my = mv->my, ry = 0;
	     my-mv->my < mv->mh && my < m->maph;
	     my++, ry += mv->tileh) {
		
		for (mx = mv->mx, rx = 0;
	     	     mx-mv->mx < mv->mw && mx < m->mapw;
		     mx++, rx += mv->tilew) {

			node = &m->map[my][mx];
#ifdef DEBUG
			if (node->x != mx || node->y != my) {
				fatal("node at %d,%d should be at %d,%d\n",
				    mx, my, node->x, node->y);
			}
#endif
			nsprites = 0;
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				/* XXX style */
				if (nref->flags & MAPREF_SPRITE) {
					nsprites++;

					if (mv->zoom != 100) {
						draw_node_scaled(mv,
						    SPRITE(nref->pobj,
						    nref->offs), rx, ry);
					} else {
						WIDGET_DRAW(mv,
						    SPRITE(nref->pobj,
						    nref->offs), rx, ry);
					}
				} else if (nref->flags & MAPREF_ANIM) {
					SDL_Surface *src;
					struct anim *anim;
					int i, j, frame;
					Uint32 t;
					
					nsprites++;

					anim = ANIM(nref->pobj, nref->offs);
					if (anim == NULL) {
						fatal("%s:%d invalid\n",
						    nref->pobj->name,
						    nref->offs);
					}
					frame = anim->frame;
					
					if ((nref->flags & MAPREF_ANIM) == 0) {
						continue;
					}

					if (nref->flags & MAPREF_ANIM_DELTA &&
					   (nref->flags & MAPREF_ANIM_STATIC)
					    == 0) {
						t = SDL_GetTicks();
						if ((t - anim->delta) >=
						    anim->delay) {
							anim->delta = t;
							if (++anim->frame >
							    anim->nframes - 1) {
								/* Loop */
								anim->frame = 0;
							}
						}
					} else if (nref->flags &
					    MAPREF_ANIM_INDEPENDENT) {
						frame = nref->frame;

						if ((nref->flags &
						    MAPREF_ANIM_STATIC) == 0) {
							if ((anim->delay < 1) ||
							    (++nref->fdelta >
							       anim->delay+1)) {
								nref->fdelta =
								    0;
								if (
								++nref->frame >
								anim->nframes -
								1) {
									nref->
									frame =
									0;
								}
							}
						}
					}

					for (i = 0, j = anim->nparts - 1;
					    i < anim->nparts; i++, j--) {
						src = anim->frames[j][frame];
						if (mv->zoom != 100) {
							draw_node_scaled(mv,
							    src,
							    rx + nref->xoffs,
							    ry + nref->yoffs -
							    (i * mv->tileh));
						} else {
							WIDGET_DRAW(mv, src,
							    rx + nref->xoffs,
							    ry + nref->yoffs -
							    (i * mv->tileh));
						}
					}
				}
			}
			
			if (nsprites > 0 && mv->flags & MAPVIEW_PROPS &&
			    mv->zoom >= 60) {
				draw_node_props(mv, node, rx, ry);
			}

			if (mv->flags & MAPVIEW_GRID) {
				primitives.line(mv,
				    rx, 0,
				    rx, WIDGET(mv)->h-1,
				    WIDGET_COLOR(mv, GRID_COLOR));
			}
			if (mx-mv->mx == mv->mouse.x &&
			    my-mv->my == mv->mouse.y &&
			    mv->mouse.x < mv->mw &&
			    mv->mouse.y < mv->mh) {
				struct tool *curtool = NULL;

				if (med != NULL) {
					curtool = med->curtool;
				}
				if (curtool != NULL &&
				    TOOL_OPS(curtool)->tool_cursor != NULL) {
					TOOL_OPS(curtool)->tool_cursor(curtool,
					    mv, mx, my);
				} else if (mv->tilew > 6 && mv->tileh > 6) {
					/* XXX cosmetic */
					primitives.square(mv,
					    rx+1, ry+1,
					    mv->tilew-1, mv->tileh-1,
					    WIDGET_COLOR(mv, CURSOR_COLOR));
					primitives.square(mv,
					    rx+2, ry+2,
					    mv->tilew-3, mv->tileh-3,
					    WIDGET_COLOR(mv, CURSOR_COLOR));
				}

			}
				
			if ((mv->flags & MAPVIEW_TILEMAP) &&
			    !TAILQ_EMPTY(&node->nrefsh) &&
			    mv->tilew > 6 && mv->tileh > 6) {
				struct noderef *nref;

				nref = TAILQ_FIRST(&node->nrefsh);
				if ((nref->pobj == med->ref.obj) &&
				    nref->offs == med->ref.offs &&
				    nref->flags == med->ref.flags) {
					/* XXX cosmetic */
					primitives.square(mv,
					    rx+1, ry+1,
					    mv->tilew-1, mv->tileh-1,
					    WIDGET_COLOR(mv,
					        TILE_SELECTION_COLOR));
					primitives.square(mv,
					    rx+2, ry+2,
					    mv->tilew-3, mv->tileh-3,
					    WIDGET_COLOR(mv,
					        TILE_SELECTION_COLOR));
				}
			}
		}
		if (mv->flags & MAPVIEW_GRID) {
			primitives.line(mv, 0, ry, WIDGET(mv)->w-1, ry,
			   WIDGET_COLOR(mv, GRID_COLOR));
		}
	}
	SDL_SetClipRect(view->v, NULL);
	
	if (WIDGET_FOCUSED(mv) && WINDOW_FOCUSED(WIDGET(mv)->win)) {
		primitives.square(mv,
		    0, 0,
		    WIDGET(mv)->w, WIDGET(mv)->h,
		    WIDGET_COLOR(mv, BORDER_COLOR));
	}
}

static void
mapview_scroll(struct mapview *mv, int dir)
{
	switch (dir) {
	case DIR_LEFT:
		if (--mv->mx <= 0) {
			mv->mx = 0;
		}
		break;
	case DIR_UP:
		if (--mv->my <= 0) {
			mv->my = 0;
		}
		break;
	case DIR_RIGHT:
		if (++mv->mx >= (mv->map->mapw - mv->mw)) {
			mv->mx--;
		}
		break;
	case DIR_DOWN:
		if (++mv->my >= (mv->map->maph - mv->mh)) {
			mv->my--;
		}
		break;
	}
}

static void
mapview_caption(struct mapview *mv)
{
	struct window *win = WIDGET(mv)->win;

	if (mv->flags & MAPVIEW_TILEMAP) {
		window_titlebar_printf(win, "%s", OBJECT(mv->map)->name);
	} else {
		if ((mv->flags & MAPVIEW_EDIT) == 0) {
			window_titlebar_printf(win, "@ %s (%d%%)",
			    OBJECT(mv->map)->name, mv->zoom);
		} else {
			window_titlebar_printf(win, "%s (%d%%)",
			    OBJECT(mv->map)->name, mv->zoom);
		}
	}
}

void
mapview_zoom(struct mapview *mv, int fac)
{
	mv->zoom = fac > 4 ? fac : 4;

	mv->tilew = mv->zoom*TILEW / 100;
	mv->tileh = mv->zoom*TILEH / 100;

	mv->mw = WIDGET(mv)->w/mv->tilew + 1;
	mv->mh = WIDGET(mv)->h/mv->tileh + 1;

	mapview_caption(mv);
}

static Uint32
mapview_zoomin(Uint32 ival, void *p)
{
	struct mapview *mv = p;
	
	/* XXX pref */
	mapview_zoom(mv, mv->zoom + 2);
	return (ival);
}

static Uint32
mapview_zoomout(Uint32 ival, void *p)
{
	struct mapview *mv = p;

	/* XXX pref */
	mapview_zoom(mv, mv->zoom - 2);
	return (ival);
}

static void
mapview_event(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	struct mapedit *med = mv->med;
	int type = argv[1].i;
	int button, x, y;

	switch (type) {
	case WINDOW_MOUSEOUT:
		mv->mouse.move = 0;
		if ((mv->flags & MAPVIEW_SHOW_CURSOR) == 0) {
			SDL_ShowCursor(SDL_ENABLE);
		}
		break;
	case WINDOW_MOUSEMOTION:
		x = argv[2].i / mv->tilew;
		y = argv[3].i / mv->tileh;
		
		if ((mv->flags & MAPVIEW_SHOW_CURSOR) == 0) {
			SDL_ShowCursor(SDL_DISABLE);
		}

		/* Scroll */
		if (mv->mouse.move) {
			mapview_scroll(mv,
			    (mv->mouse.x < x) ? DIR_LEFT :
			    (mv->mouse.x > x) ? DIR_RIGHT :
			    (mv->mouse.y < y) ? DIR_UP :
			    (mv->mouse.y > y) ? DIR_DOWN : 0);
		} else if (mv->flags & MAPVIEW_EDIT) {
			if (med->curtool != NULL &&
			    (x != mv->mouse.x || y != mv->mouse.y) &&
			    SDL_GetMouseState(NULL, NULL) &
			    SDL_BUTTON_LMASK &&
			    (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
			    (mv->mx+x < mv->map->mapw) &&
			    (mv->my+y < mv->map->maph) &&
			    TOOL_OPS(med->curtool)->tool_effect != NULL) {
				TOOL_OPS(med->curtool)->tool_effect(
				    med->curtool, mv, mv->mx+x, mv->my+y);
			}
		}
		if (mv->flags & MAPVIEW_TILEMAP) {
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK &&
			    (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
			    (mv->mx+x < mv->map->mapw) &&
			    (mv->my+y < mv->map->maph)) {
				struct node *n;
				struct editobj *eo = NULL;

				n = &mv->map->map[mv->my+y][mv->mx+x];
				if (!TAILQ_EMPTY(&n->nrefsh)) {
					struct noderef *nref;
					
					nref = TAILQ_FIRST(&n->nrefsh);
					TAILQ_FOREACH(eo, &med->eobjsh, eobjs) {
						if (eo->pobj == nref->pobj) {
							break;
						}
					}
					if (eo != NULL) {
						med->curobj = eo;
						med->node.flags = n->flags;
						med->ref.obj = eo->pobj;
						med->ref.offs = nref->offs;
						med->ref.flags = nref->flags;
						dprintf("%s:%d[0x%x]\n",
						    med->ref.obj->name,
						    med->ref.offs,
						    med->ref.flags);
					} else {
						warning("unusable reference\n");
					}
				}
			}
		}
	
		mv->mouse.x = x;
		mv->mouse.y = y;
		break;
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		x = argv[3].i / mv->tilew;
		y = argv[4].i / mv->tileh;
		WIDGET_FOCUS(mv);
		if (button > 1) {
			mv->mouse.move++;
		} else if (mv->flags & MAPVIEW_EDIT) {
			if (med->curtool != NULL &&
			   (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
			   (mv->mx+x < mv->map->mapw) &&
			   (mv->my+y < mv->map->maph) &&
			    TOOL_OPS(med->curtool)->tool_effect != NULL) {
				TOOL_OPS(med->curtool)->tool_effect(
				    med->curtool, mv, mv->mx+x, mv->my+y);
			}
		}
		if (mv->flags & MAPVIEW_TILEMAP) {
			if ((x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
			    (mv->mx+x < mv->map->mapw) &&
			    (mv->my+y < mv->map->maph)) {
				struct node *n;
				struct editobj *eo = NULL;

				n = &mv->map->map[mv->my+y][mv->mx+x];
				if (!TAILQ_EMPTY(&n->nrefsh)) {
					struct noderef *nref;
					
					nref = TAILQ_FIRST(&n->nrefsh);
					TAILQ_FOREACH(eo, &med->eobjsh, eobjs) {
						if (eo->pobj == nref->pobj) {
							break;
						}
					}
					if (eo != NULL) {
						med->curobj = eo;
						med->node.flags = n->flags;
						med->ref.obj = eo->pobj;
						med->ref.offs = nref->offs;
						med->ref.flags = nref->flags;
						dprintf("%s:%d[0x%x]\n",
						    med->ref.obj->name,
						    med->ref.offs,
						    med->ref.flags);
					} else {
						warning("unusable reference\n");
					}
				}
			}
		}
		break;
	case WINDOW_MOUSEBUTTONUP:
		button = argv[2].i;
		if (button > 1) {
			mv->mouse.move = 0;
		}
		break;
	case WINDOW_KEYDOWN:
		if (mv->flags & MAPVIEW_ZOOM) {
			switch (argv[2].i) {
			case SDLK_EQUALS:
				/* XXX logarithmic curve */
				zoomin_timer =
				    SDL_AddTimer(60, mapview_zoomin, mv);
				if (zoomin_timer == NULL) {
					warning("SDL_AddTimer: %s\n",
					    SDL_GetError());
				}
				break;
			case SDLK_MINUS:
				zoomout_timer =
				    SDL_AddTimer(60, mapview_zoomout, mv);
				if (zoomout_timer == NULL) {
					warning("SDL_AddTimer: %s\n",
					    SDL_GetError());
				}
				break;
			case SDLK_0:
				mapview_zoom(mv, 100);
				mapview_stop_zoom(mv);
				break;
			}
		}
		break;
	case WINDOW_KEYUP:
		if (mv->flags & MAPVIEW_ZOOM) {
			switch (argv[2].i) {
			case SDLK_EQUALS:
			case SDLK_MINUS:
				mapview_stop_zoom(mv);
				break;
			}
		}
		break;
	}
}

static void
mapview_scaled(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	WIDGET(mv)->w = argv[1].i;
	WIDGET(mv)->h = argv[2].i;

	mapview_zoom(mv, mv->zoom);
}

static void
mapview_lostfocus(int argc, union evarg *argv)
{
	if (zoomin_timer != NULL) {
		SDL_RemoveTimer(zoomin_timer);
	}
	if (zoomout_timer != NULL) {
		SDL_RemoveTimer(zoomout_timer);
	}
}

void
mapview_center(struct mapview *mv, int x, int y)
{
	struct map *m = mv->map;
	int nx, ny;

	nx = x - (mv->mw / 2);
	ny = y - (mv->mh / 2);
	
	if (nx < 0)
		nx = 0;
	if (ny < 0)
		ny = 0;
	if (nx >= m->mapw-mv->mw)
		nx = m->mapw - mv->mw;
	if (ny >= m->maph-mv->mh)
		ny = m->maph - mv->mh;

	mv->mx = nx;
	mv->my = ny;
}

static void
mapview_stop_zoom(struct mapview *mv)
{
	if (zoomin_timer != NULL) {
		SDL_RemoveTimer(zoomin_timer);
		zoomin_timer = NULL;
	}
	if (zoomout_timer != NULL) {
		SDL_RemoveTimer(zoomout_timer);
		zoomout_timer = NULL;
	}
}

