/*	$Csoft: fill.c,v 1.27 2003/02/17 02:46:39 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/version.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include <libfobj/fobj.h>

#include "tool.h"
#include "fill.h"

static const struct tool_ops fill_ops = {
	{
		NULL,		/* destroy */
		fill_load,
		fill_save
	},
	fill_window,
	NULL,			/* cursor */
	fill_effect
};

static const struct version fill_ver = {
	"agar fill tool",
	0, 0
};

void
fill_init(void *p)
{
	struct fill *fill = p;

	tool_init(&fill->tool, "fill", &fill_ops);

	fill->mode = FILL_MAP;
}

struct window *
fill_window(void *p)
{
	struct fill *fi = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-fill", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    157, 76,
	    157, 76);
	window_set_caption(win, "Fill");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		struct radio *rad;
		static const char *mode_items[] = {
			"Fill map",
			"Clear map",
			NULL
		};

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &fi->mode);
		win->focus = WIDGET(rad);
	}
	return (win);
}

void
fill_effect(void *p, struct mapview *mv, struct node *dstnode)
{
	struct fill *fi = p;
	struct map *m = mv->map;
	struct node *srcnode = mapedit.src_node;
	struct noderef *nref, *nnref;
	Uint32 x, y;

	if (srcnode == NULL && fi->mode == FILL_MAP) {
		text_msg("Error", "No source node");
		return;
	}

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *dstnode = &m->map[y][x];
			struct noderef *nref;
			
			MAP_CHECK_NODE(dstnode, x, y);

			if (srcnode == dstnode) {
				text_msg("Error",
				    "Source node == destination node");
				continue;
			}

			node_destroy(dstnode);
			node_init(dstnode, x, y);

			switch (fi->mode) {
			case FILL_MAP:
				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
					node_copy_ref(nref, dstnode);
				}
				break;
			}

			dstnode->flags = srcnode->flags & ~NODE_ORIGIN;
		}
	}
}

int
fill_load(void *p, int fd)
{
	struct fill *fill = p;

	if (version_read(fd, &fill_ver) == -1)
		return (-1);
	fill->mode = (int)read_uint32(fd);
	dprintf("mode 0x%x\n", fill->mode);
	return (0);
}

int
fill_save(void *p, int fd)
{
	struct fill *fill = p;

	version_write(fd, &fill_ver);
	write_uint32(fd, (Uint32)fill->mode);
	return (0);
}
