/*	$Csoft: fileops.c,v 1.49 2003/05/26 03:03:30 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc
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
#include <engine/config.h>
#include <engine/view.h>
#include <engine/prop.h>

#include <string.h>

#include "mapedit.h"
#include "mapview.h"

/* Save the map to the default location. */
void
fileops_save_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (object_save(mv->map) == -1)
		text_msg("Error saving", "%s", error_get());
}

/* Revert to the map on disk. */
void
fileops_revert_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;

	if (object_load(mv->map) == -1) {
		text_msg("Error reverting", "%s", error_get());
		return;
	}
	mapview_center(mv, m->origin.x, m->origin.y);
}

/* Clear all nodes on a map. */
void
fileops_clear_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;
	int x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			
			node_destroy(node);
			node_init(node);
		}
	}
}

