/*	$Csoft	    */

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

/* XXX use generic input, remapping */

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>

#include "mapedit.h"
#include "command.h"
#include "joy.h"

/*
 * Joystick motion handler.
 * Map editor and map must be locked, called in event context.
 */
void
joy_axismotion(struct mapedit *med, SDL_Event *ev)
{
	static int lastdir = 0;

	switch (ev->jaxis.axis) {
	case 0:	/* X */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_LEFT;
			lastdir &= ~(DIR_RIGHT);
			mapdir_set(&med->cursor_dir, DIR_LEFT, 1);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_RIGHT;
			lastdir &= ~(DIR_LEFT);
			mapdir_set(&med->cursor_dir, DIR_RIGHT, 1);
		} else {
			mapdir_set(&med->cursor_dir, DIR_ALL, 0);
		}
		break;
	case 1:	/* Y */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_UP;
			lastdir &= ~(DIR_DOWN);
			mapdir_set(&med->cursor_dir, DIR_UP, 1);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_DOWN;
			lastdir &= ~(DIR_UP);
			mapdir_set(&med->cursor_dir, DIR_DOWN, 1);
		} else {
			mapdir_set(&med->cursor_dir, DIR_ALL, 0);
		}
		break;
	}
}

/*
 * Joystick button handler.
 * Map editor and map must be locked, called in event context.
 */
void
joy_button(struct mapedit *med, SDL_Event *ev)
{
	struct map *m = med->map;
	struct node *node = &m->map[med->y][med->x];

	/* XXX customize */
	switch (ev->jbutton.button) {
	case 1:	/* Push */
		mapedit_push(med, node, med->curoffs, med->curflags);
		break;
	case 2: /* Pop */
		mapedit_pop(med, node, 0);
		break;
	case 4: /* Tile list up */
		gendir_set(&med->listw_dir, DIR_UP, 1);
		break;
	case 5: /* Tile list down */
		gendir_set(&med->listw_dir, DIR_DOWN, 1);
		break;
	/* XXX ... */
	}
}

