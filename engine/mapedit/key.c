/*	$Csoft	    */

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
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

#include <engine/engine.h>

#include "mapedit.h"
#include "command.h"
#include "key.h"

/*
 * Process a map editor keystroke.
 * Must be called on a locked map.
 */
void
mapedit_key(struct mapedit *med, SDL_Event *ev)
{
	const int set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		mapdir_set(&med->cursor_dir, DIR_UP, set);
		break;
	case SDLK_DOWN:
		mapdir_set(&med->cursor_dir, DIR_DOWN, set);
		break;
	case SDLK_LEFT:
		mapdir_set(&med->cursor_dir, DIR_LEFT, set);
		break;
	case SDLK_RIGHT:
		mapdir_set(&med->cursor_dir, DIR_RIGHT, set);
		break;
	case SDLK_PAGEUP:
		gendir_set(&med->listw_dir, DIR_UP, set);
		break;
	case SDLK_PAGEDOWN:
		gendir_set(&med->listw_dir, DIR_DOWN, set);
		break;
	case SDLK_DELETE:
		gendir_set(&med->olistw_dir, DIR_LEFT, set);
		break;
	case SDLK_END:
		gendir_set(&med->olistw_dir, DIR_RIGHT, set);
		break;
	default:
		break;
	}

	if (ev->type == SDL_KEYDOWN) {
		struct node *node;
		int mapx, mapy;

		mapx = med->x;
		mapy = med->y;
		node = &med->map->map[mapx][mapy];

		switch (ev->key.keysym.sym) {
		case SDLK_a:
			mapedit_push(med, node, med->curoffs, med->curflags);
			break;
		case SDLK_d:
			mapedit_pop(med, node);
			break;
		case SDLK_b:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_BIO);
			} else {
				mapedit_nodeflags(med, node, NODE_BLOCK);
			}
			break;
		case SDLK_w:
			mapedit_nodeflags(med, node, NODE_WALK);
			break;
		case SDLK_c:
			mapedit_nodeflags(med, node, NODE_CLIMB);
			break;
		case SDLK_p:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_editflags(med, MAPEDIT_DRAWPROPS);
			} else {
				mapedit_nodeflags(med, node, NODE_SLIP);
			}
			break;
		case SDLK_h:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_HASTE);
			}
			break;
		case SDLK_r:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_REGEN);
			}
			break;
		case SDLK_i:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_fillmap(med);
			}
			break;
		case SDLK_n:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_clearmap(med);
			}
			break;
		case SDLK_o:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setorigin(med, &mapx, &mapy);
				mapedit_move(med, mapx, mapy);
			}
			break;
		case SDLK_l:
			mapedit_loadmap(med);
			break;
		case SDLK_s:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_SLOW);
			} else {
				mapedit_savemap(med);
			}
			break;
		case SDLK_g:
			mapedit_editflags(med, MAPEDIT_DRAWGRID);
			break;
		case SDLK_x:
			mapedit_examine(med->map, mapx, mapy);
			break;
		default:
			break;
		}
	}
}

