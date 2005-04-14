/*	$Csoft: mediasel.c,v 1.18 2005/03/11 08:59:33 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/config.h>
#include <engine/view.h>

#include <engine/map/map.h>
#include <engine/map/mapedit.h>

#include <engine/loader/den.h>

#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>

#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static void mediasel_scan_dens(struct combo *, const char *, const char *);
static void mediasel_refresh(struct map *, int, struct combo *);
static void mediasel_init(struct tool *);

const struct tool mediasel_tool = {
	N_("Media selector"),
	N_("Import graphics or audio into the map."),
	MEDIASEL_ICON,
	-1,
	mediasel_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

enum {
	MEDIASEL_GFX,
	MEDIASEL_AUDIO
};

static void
refresh_media(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;
	struct combo *com = argv[2].p;
	int type = argv[3].i;

	mediasel_refresh(m, type, com);
}

/* Select a new graphics package to associate with the object. */
static void
media_selected(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;
	int type = argv[2].i;
	struct tlist_item *it = argv[3].p;

	switch (type) {
	case MEDIASEL_GFX:
		if (OBJECT(m)->gfx != NULL) {
			object_page_out(m, OBJECT_GFX);
		}
		if (it->text[0] == '\0') {
			return;
		}
		OBJECT(m)->gfx_name = Strdup(it->text);
		if (object_page_in(m, OBJECT_GFX) == -1) {
			text_msg(MSG_ERROR, "%s: %s", OBJECT(m)->gfx_name,
			    error_get());
			Free(OBJECT(m)->gfx_name, 0);
			OBJECT(m)->gfx_name = NULL;
		}
		break;
	case MEDIASEL_AUDIO:
		if (OBJECT(m)->audio != NULL) {
			object_page_out(m, OBJECT_AUDIO);
		}
		if (it->text[0] == '\0') {
			return;
		}
		OBJECT(m)->audio_name = Strdup(it->text);
		if (object_page_in(m, OBJECT_AUDIO) == -1) {
			text_msg(MSG_ERROR, "%s: %s", OBJECT(m)->audio_name,
			    error_get());
			Free(OBJECT(m)->audio_name, 0);
			OBJECT(m)->audio_name = NULL;
		}
		break;
	}
}

static void
mediasel_refresh(struct map *m, int type, struct combo *com)
{
	char den_path[MAXPATHLEN], *denp = &den_path[0];
	char *dir;
	const char *hint = NULL;
	
	switch (type) {
	case MEDIASEL_GFX:
		if (OBJECT(m)->gfx_name != NULL) {
			textbox_printf(com->tbox, "%s", OBJECT(m)->gfx_name);
		}
		hint = "gfx";
		break;
	case MEDIASEL_AUDIO:
		if (OBJECT(m)->audio_name != NULL) {
			textbox_printf(com->tbox, "%s", OBJECT(m)->audio_name);
		}
		hint = "audio";
		break;
	}

	tlist_clear_items(com->list);

	prop_copy_string(config, "den-path", den_path, sizeof(den_path));

	for (dir = strsep(&denp, ":");
	     dir != NULL;
	     dir = strsep(&denp, ":")) {
		char cwd[MAXPATHLEN];

		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			text_msg(MSG_ERROR, "getcwd: %s", strerror(errno));
			continue;
		}

		if (chdir(dir) == 0) {
			mediasel_scan_dens(com, hint, "");
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
mediasel_scan_dens(struct combo *com, const char *hint, const char *spath)
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
					tlist_insert_item(com->list, NULL, path,
					    NULL);
				}
				den_close(den);
			}
		} else if (sta.st_mode & S_IFDIR) {
			if (chdir(dent->d_name) == 0) {
				mediasel_scan_dens(com, hint, path);
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
		u_int nw, nh;
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
			r = node_add_anim(m, node, ob, ind, NODEREF_SHD_FRAME);
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
	struct map *m = argv[1].p;

	if (OBJECT(m)->gfx_name != NULL)
		object_page_in(m, OBJECT_GFX);
	if (OBJECT(m)->audio_name != NULL)
		object_page_in(m, OBJECT_AUDIO);
}

static void
hidden_window(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct map *m = argv[1].p;

	if (OBJECT(m)->gfx_name != NULL)
		object_page_out(m, OBJECT_GFX);
	if (OBJECT(m)->audio_name != NULL)
		object_page_out(m, OBJECT_AUDIO);
}

/* Update the graphic import list. */
static void
poll_gfxmedia(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	char label[TLIST_LABEL_MAX];
	Uint32 i;
	
	tlist_clear_items(tl);
	if (ob->gfx == NULL)
		goto out;
	
	for (i = 0; i < ob->gfx->nsubmaps; i++) {
		struct map *sm = ob->gfx->submaps[i];

		snprintf(label, sizeof(label),
		    _("m%u (%ux%u nodes)\n"), i, sm->mapw, sm->maph);
		/* TODO minimap */
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
out:
	tlist_restore_selections(tl);
}

static void
mediasel_init(struct tool *t)
{
	struct map *m = t->p;
	struct mapview *mv = t->mv;
	struct window *win;
	struct box *vb, *hb;
	struct combo *gfx_com, *aud_com;
	struct tlist *tl;
	struct button *bu;
#ifdef DEBUG
	if (!OBJECT_TYPE(m, "map"))
		fatal("type");
#endif

	win = tool_window(t, "mapedit-mediasel");
	window_set_caption(win, _("Load media into `%s'"), OBJECT(m)->name);
	event_new(win, "window-shown", shown_window, "%p", m);
	event_new(win, "window-hidden", hidden_window, "%p", m);

	vb = box_new(win, BOX_VERT, BOX_WFILL);
	box_set_spacing(vb, 0);
	box_set_padding(vb, 2);
	{
		hb = box_new(vb, BOX_VERT, BOX_WFILL);
		box_set_spacing(hb, 1);
		box_set_padding(hb, 2);
		{
			gfx_com = combo_new(hb, 0, "Gfx: ");
			event_new(gfx_com, "combo-selected", media_selected,
			    "%p, %i", m, MEDIASEL_GFX);
			mediasel_refresh(m, MEDIASEL_GFX, gfx_com);
			
			aud_com = combo_new(hb, 0, "Audio: ");
			event_new(aud_com, "combo-selected", media_selected,
			    "%p, %i", m, MEDIASEL_AUDIO);
			mediasel_refresh(m, MEDIASEL_AUDIO, aud_com);
		}
		
		hb = box_new(vb, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
		box_set_spacing(hb, 1);
		box_set_padding(hb, 2);
		{
			bu = button_new(hb, _("Refresh graphics"));
			event_new(bu, "button-pushed", refresh_media,
			    "%p, %p, %i", m, gfx_com, MEDIASEL_GFX);
			
			bu = button_new(hb, _("Refresh audio"));
			event_new(bu, "button-pushed", refresh_media,
			    "%p, %p, %i", m, aud_com, MEDIASEL_AUDIO);
		}

		tl = tlist_new(win, TLIST_POLL|TLIST_MULTI);
		tlist_set_item_height(tl, text_font_height*2);
		tlist_prescale(tl, "XXXXXXXXXXXXXXX", 8);
		event_new(tl, "tlist-poll", poll_gfxmedia, "%p", OBJECT(m));

		hb = box_new(vb, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
		box_set_spacing(hb, 1);
		box_set_padding(hb, 1);
		{
			int icons[] = {
				LEFT_ARROW_ICON,
				RIGHT_ARROW_ICON,
				UP_ARROW_ICON,
				DOWN_ARROW_ICON
			};
			int i;

			/* XXX use toolbar */
			for (i = 0; i < 4; i++) {
				bu = button_new(hb, NULL);
				button_set_label(bu, ICON(icons[i]));
				event_new(bu, "button-pushed", import_media,
				    "%p, %p, %i, %p", tl, mv, i, OBJECT(m));
			}
		}
	}
}

#endif	/* EDITION */
