/*	$Csoft: mediasel.c,v 1.5 2004/03/17 12:42:06 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include <config/edition.h>
#ifdef EDITION

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/mapedit/mapedit.h>
#include <engine/loader/den.h>
#include <engine/widget/tlist.h>

#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "mediasel.h"

static void mediasel_scan_dens(struct mediasel *, const char *, const char *);
static void mediasel_refresh(struct mediasel *);

static void
refresh_media(int argc, union evarg *argv)
{
	struct mediasel *msel = argv[1].p;

	mediasel_refresh(msel);
}

/* Select a new graphics package to associate with the object. */
static void
media_selected(int argc, union evarg *argv)
{
	struct mediasel *msel = argv[1].p;
	struct tlist_item *it = argv[2].p;

	switch (msel->type) {
	case MEDIASEL_GFX:
		if (msel->obj->gfx != NULL) {
			object_page_out(msel->obj, OBJECT_GFX);
		}
		if (it->text[0] == '\0') {
			return;
		}
		msel->obj->gfx_name = Strdup(it->text);
		if (object_page_in(msel->obj, OBJECT_GFX) == -1) {
			text_msg(MSG_ERROR, "%s: %s", msel->obj->gfx_name,
			    error_get());
			free(msel->obj->gfx_name);
			msel->obj->gfx_name = NULL;
		}
		break;
	case MEDIASEL_AUDIO:
		if (msel->obj->gfx != NULL) {
			object_page_out(msel->obj, OBJECT_AUDIO);
		}
		if (it->text[0] == '\0') {
			return;
		}
		msel->obj->audio_name = Strdup(it->text);
		if (object_page_in(msel->obj, OBJECT_AUDIO) == -1) {
			text_msg(MSG_ERROR, "%s: %s", msel->obj->audio_name,
			    error_get());
			free(msel->obj->audio_name);
			msel->obj->audio_name = NULL;
		}
		break;
	}
}

static void
mediasel_refresh(struct mediasel *msel)
{
	char den_path[MAXPATHLEN];
	char *dir, *last;
	const char *hint = NULL;
	
	switch (msel->type) {
	case MEDIASEL_GFX:
		if (msel->obj->gfx_name != NULL) {
			textbox_printf(msel->com->tbox, "%s",
			    msel->obj->gfx_name);
		}
		hint = "gfx";
		break;
	case MEDIASEL_AUDIO:
		if (msel->obj->audio_name != NULL) {
			textbox_printf(msel->com->tbox, "%s",
			    msel->obj->audio_name);
		}
		hint = "audio";
		break;
	}

	tlist_clear_items(msel->com->list);

	prop_copy_string(config, "den-path", den_path, sizeof(den_path));

	for (dir = strtok_r(den_path, ":", &last);
	     dir != NULL;
	     dir = strtok_r(NULL, ":", &last)) {
		char cwd[MAXPATHLEN];

		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			text_msg(MSG_ERROR, "getcwd: %s", strerror(errno));
			continue;
		}

		if (chdir(dir) == 0) {
			mediasel_scan_dens(msel, hint, "");
			if (chdir(cwd) == -1) {
				text_msg(MSG_ERROR, "chdir %s: %s", cwd,
				    strerror(errno));
				continue;
			}
		}
	}
}

/* Search subdirectories for .den files containing a given hint. */
static void
mediasel_scan_dens(struct mediasel *msel, const char *hint, const char *spath)
{
	DIR *di;
	struct dirent *dent;

	if ((di = opendir(".")) == NULL) {
		text_msg(MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	while ((dent = readdir(di)) != NULL) {
		char path[MAXPATHLEN];
		struct stat sta;
		char *ext;

		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		if (stat(dent->d_name, &sta) == -1) {
			dprintf("%s: %s\n", dent->d_name, strerror(errno));
			continue;
		}

		strlcpy(path, spath, sizeof(path));
		strlcat(path, "/", sizeof(path));
		strlcat(path, dent->d_name, sizeof(path));

		if ((sta.st_mode & S_IFREG) &&
		    (ext = strrchr(dent->d_name, '.')) != NULL &&
		    strcasecmp(ext, ".den") == 0) {
			struct den *den;

			if ((den = den_open(dent->d_name, DEN_READ)) != NULL) {
				if (strcmp(den->hint, hint) == 0 &&
				    (ext = strrchr(path, '.')) != NULL) {
					*ext = '\0';
					tlist_insert_item(msel->com->list, NULL,
					    path, NULL);
					/* TODO icon */
				}
				den_close(den);
			}
		} else if (sta.st_mode & S_IFDIR) {
			if (chdir(dent->d_name) == 0) {
				mediasel_scan_dens(msel, hint, path);
				if (chdir("..") == -1) {
					text_msg(MSG_ERROR, "..: %s",
					    strerror(errno));
					closedir(di);
					return;
				}
			}
		}
	}
	closedir(di);
}

enum {
	INSERT_LEFT,
	INSERT_RIGHT,
	INSERT_UP,
	INSERT_DOWN
};

/*
 * Generate node references from media into a map.
 * The map is resized as necessary.
 */
static void
import_media(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct mapview *mv = argv[2].p;
	int mode = argv[3].i;
	struct object *ob = argv[4].p;
	struct tlist_item *it;
	struct map *m = mv->map;
	int sx, sy, dx, dy;
	struct noderef *r;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no active selection."));
		return;
	}

	TAILQ_FOREACH(it, &tl->items, items) {
		char ind_str[32];
		struct node *node;
		int t, xinc, yinc;
		Uint32 ind;
		unsigned int nw, nh;
		struct map *submap = it->p1;
		SDL_Surface *srcsu = it->p1;
		struct gfx_anim *anim = it->p1;

		if (!it->selected)
			continue;

		strlcpy(ind_str, it->text+1, sizeof(ind_str));
		*(strchr(ind_str, ' ')) = '\0';
		ind = (Uint32)atoi(ind_str);

		switch (it->text[0]) {
		case 's':
			t = NODEREF_SPRITE;
			xinc = srcsu->w/TILESZ;
			yinc = srcsu->h/TILESZ;
			break;
		case 'a':
			{
				SDL_Surface *frame = anim->frames[0];

				t = NODEREF_ANIM;
				/* XXX */
				xinc = frame->w/TILESZ;
				yinc = frame->h/TILESZ;
			}
			break;
		case 'm':
			t = -1;
			xinc = submap->mapw;
			yinc = submap->maph;
			break;
		default:
			fatal("bad ref");
		}

		nw = mv->esel.x + xinc + 1;
		nh = mv->esel.y + yinc + 1;
		dprintf("inc %d,%d, ngeo=%ux%u\n", xinc, yinc, nw, nh);

		if (map_resize(m,
		    nw > m->mapw ? nw : m->mapw,
		    nh > m->maph ? nh : m->maph) == -1) {
			text_msg(MSG_ERROR, "%s", error_get());
			continue;
		}
		
		switch (t) {
		case NODEREF_SPRITE:
			node = &m->map[mv->esel.y][mv->esel.x];
			node_clear(m, node, m->cur_layer);
			r = node_add_sprite(m, node, ob, ind);
			r->layer = m->cur_layer;
			break;
		case NODEREF_ANIM:
			node = &m->map[mv->esel.y][mv->esel.x];
			node_clear(m, node, m->cur_layer);
			r = node_add_anim(m, node, ob, ind, NODEREF_ANIM_AUTO);
			r->layer = m->cur_layer;
			break;
		case -1:					/* Submap */
			/*
			 * Copy the elements of a fragment submap. The NULL
			 * references are translated to the object itself,
			 * to avoid complications with dependencies.
			 */
			for (sy = 0, dy = mv->esel.y;
			     sy < submap->maph && dy < m->maph;
			     sy++, dy++) {
				for (sx = 0, dx = mv->esel.x;
				     sx < submap->mapw && dx < m->mapw;
				     sx++, dx++) {
					struct node *sn = &submap->map[sy][sx];
					struct node *dn = &m->map[dy][dx];
					struct noderef *r;

					node_clear(m, dn, m->cur_layer);
					node_copy(submap, sn, -1, m, dn,
					    m->cur_layer);

					TAILQ_FOREACH(r, &dn->nrefs, nrefs) {
						if (r->layer != m->cur_layer) {
							continue;
						}
						switch (r->type) {
						case NODEREF_SPRITE:
							if (r->r_sprite.obj
							    != NULL) {
								break;
							}
							r->r_sprite.obj =
							    OBJECT(m);
							object_add_dep(m, m);
							if (object_page_in(m,
							    OBJECT_GFX) == -1) {
								fatal("page");
							}
							break;
						case NODEREF_ANIM:
							if (r->r_anim.obj
							    != NULL) {
								break;
							}
							r->r_anim.obj =
							    OBJECT(m);
							object_add_dep(m, m);
							if (object_page_in(m,
							    OBJECT_GFX) == -1) {
								fatal("page");
							}
							break;
						default:
							break;
						}
						r->layer = m->cur_layer;
					}
				}
			}
			break;
		default:
			fatal("bad nref");
		}

		switch (mode) {
		case INSERT_LEFT:
			if ((mv->esel.x -= xinc) < 0)
				mv->esel.x = 0;
			break;
		case INSERT_RIGHT:
			mv->esel.x += xinc;
			break;
		case INSERT_UP:
			if ((mv->esel.y -= yinc) < 0)
				mv->esel.y = 0;
			break;
		case INSERT_DOWN:
			mv->esel.y += yinc;
			break;
		}
		mv->esel.w = 1;
		mv->esel.h = 1;
	}
}

static void
shown_window(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	if (OBJECT(mv->map)->gfx_name != NULL)
		object_page_in(mv->map, OBJECT_GFX);
	if (OBJECT(mv->map)->audio_name != NULL)
		object_page_in(mv->map, OBJECT_AUDIO);
}

static void
hidden_window(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	if (OBJECT(mv->map)->gfx_name != NULL)
		object_page_out(mv->map, OBJECT_GFX);
	if (OBJECT(mv->map)->audio_name != NULL)
		object_page_out(mv->map, OBJECT_AUDIO);
}

/* Update the graphic import list. */
static void
poll_media(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	char label[TLIST_LABEL_MAX];
	Uint32 i;
	
	tlist_clear_items(tl);

	if (ob->gfx == NULL)
		return;
	
	for (i = 0; i < ob->gfx->nsubmaps; i++) {
		struct map *sm = ob->gfx->submaps[i];

		snprintf(label, sizeof(label),
		    _("m%u (%ux%u nodes)\n"), i, sm->mapw, sm->maph);
		tlist_insert_item(tl, NULL, label, sm);
	}
	for (i = 0; i < ob->gfx->nsprites; i++) {
		SDL_Surface *sp = ob->gfx->sprites[i];
	
		snprintf(label, sizeof(label),
		    _("s%u (%ux%u pixels, %ubpp)\n"), i, sp->w, sp->h,
		    sp->format->BitsPerPixel);
		tlist_insert_item(tl, sp, label, sp);
	}
	for (i = 0; i < ob->gfx->nanims; i++) {
		struct gfx_anim *an = ob->gfx->anims[i];

		snprintf(label, sizeof(label),
		    _("a%u (%u frames)\n"), i, an->nframes);
		tlist_insert_item(tl, (an->nframes > 0) ?
		    an->frames[0] : NULL, label, an);
	}

	tlist_restore_selections(tl);
}

static void
close_window(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->mediasel.trigger, "state", 0);
	window_hide(win);
}

void
mediasel_init(struct mapview *mv, struct window *pwin)
{
	struct object *ob = OBJECT(mv->map);
	struct window *win;
	struct box *vb, *hb;
	struct combo *gfx_com, *aud_com;
	struct tlist *tl;
	struct mediasel *msel;

	win = window_new(NULL);
	window_set_caption(win, _("Load media into `%s'"), ob->name);
	event_new(win, "window-shown", shown_window, "%p", mv);
	event_new(win, "window-hidden", hidden_window, "%p", mv);
	event_new(win, "window-close", close_window, "%p", mv);

	vb = box_new(win, BOX_VERT, BOX_WFILL);
	box_set_spacing(vb, 0);
	box_set_padding(vb, 2);

	hb = box_new(vb, BOX_HORIZ, BOX_WFILL);
	box_set_spacing(hb, 1);
	box_set_padding(hb, 2);
	mv->mediasel.gfx = msel = Malloc(sizeof(struct mediasel));
	msel->type = MEDIASEL_GFX;
	msel->obj = ob;
	msel->com = combo_new(hb, 0, _("Graphics: "));
	textbox_prescale(msel->com->tbox, "XXXXXXXXXXXXXXXXX");
	event_new(msel->com, "combo-selected", media_selected, "%p", msel);
	msel->rfbu = button_new(hb, _("Refresh"));
	event_new(msel->rfbu, "button-pushed", refresh_media, "%p", msel);
	mediasel_refresh(msel);
	
	hb = box_new(vb, BOX_HORIZ, BOX_WFILL);
	box_set_spacing(hb, 1);
	box_set_padding(hb, 2);
	mv->mediasel.audio = msel = Malloc(sizeof(struct mediasel));
	msel->type = MEDIASEL_AUDIO;
	msel->obj = ob;
	msel->com = combo_new(hb, 0, _("Audio: "));
	textbox_prescale(msel->com->tbox, "XXXXXXXXXXXXXXXXX");
	event_new(msel->com, "combo-selected", media_selected, "%p", msel);
	msel->rfbu = button_new(hb, _("Refresh"));
	event_new(msel->rfbu, "button-pushed", refresh_media, "%p", msel);
	mediasel_refresh(msel);

	tl = tlist_new(win, TLIST_POLL|TLIST_MULTI);
	tlist_set_item_height(tl, ttf_font_height(font)*2);
	event_new(tl, "tlist-poll", poll_media, "%p", ob);

	hb = box_new(vb, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	box_set_spacing(hb, 1);
	box_set_padding(hb, 1);
	{
		int i;
		int icons[] = {
			MAPEDIT_TOOL_LEFT,
			MAPEDIT_TOOL_RIGHT,
			MAPEDIT_TOOL_UP,
			MAPEDIT_TOOL_DOWN
		};
		struct button *bu;

		for (i = 0; i < 4; i++) {
			bu = button_new(hb, NULL);
			button_set_label(bu, SPRITE(&mapedit, icons[i]));
			event_new(bu, "button-pushed", import_media,
			    "%p, %p, %i, %p", tl, mv, i, ob);
		}
	}
	mv->mediasel.win = win;
	window_attach(pwin, win);
}

void
mediasel_destroy(struct mapview *mv)
{
	free(mv->mediasel.gfx);
	free(mv->mediasel.audio);
}

#endif	/* EDITION */
