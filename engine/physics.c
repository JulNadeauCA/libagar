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

int
direction_init(struct direction *dir, int flags, int hiwat)
{
	dir->current = 0;
	dir->clear = 0;
	dir->moved = 0;
	dir->flags = flags;

	dir->tick = 0;
	dir->hiwat = hiwat;

	return (0);
}

/*
 * Set the given direction if set is non-zero, otherwise
 * clear it (asynchronously).
 */
void
direction_set(struct direction *dir, int dirmask, int set)
{
	if (!set) {
		dir->clear |= dirmask;
		return;
	}

	if (dirmask & DIR_UP)
		dir->current &= ~(DIR_DOWN);
	if (dirmask & DIR_DOWN)
		dir->current &= ~(DIR_UP);
	if (dirmask & DIR_LEFT)
		dir->current &= ~(DIR_RIGHT);
	if (dirmask & DIR_RIGHT)
		dir->current &= ~(DIR_LEFT);

	dir->current |= dirmask;
}

/*
 * Update a direction, and return a non-zero value if the map
 * coordinates have changed (so that the caller can move the
 * reference on the map).
 */
int
direction_update(struct direction *dir, struct map *map, int *mapx, int *mapy)
{
	int move = 0;

	if (dir->clear != 0) {
		dir->current &= ~dir->clear;
		dir->clear = 0;
	}

	if (dir->current != 0 &&
	   (dir->moved == 0 || ++dir->tick > dir->hiwat)) {
		dir->tick = 0;

		if (dir->current & DIR_UP) {
			dir->moved |= DIR_UP;
			if (dir->flags & DIR_ONMAP) {
				decrease(mapy, 1, 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapy - *mapy) >= 0) {
				    	scroll(map, DIR_UP);
				}
			}
			move |= DIR_UP;
		}
		if (dir->current & DIR_DOWN) {
			dir->moved |= DIR_DOWN;
			if (dir->flags & DIR_ONMAP) {
				increase(mapy, 1, map->maph - 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapy - *mapy) <=
				     -map->view->maph) {
					scroll(map, DIR_DOWN);
				}
			}
			move |= DIR_DOWN;
		}
		if (dir->current & DIR_LEFT) {
			dir->moved |= DIR_LEFT;
			if (dir->flags & DIR_ONMAP) {
				decrease(mapx, 1, 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapx - *mapx) >= 0) {
					scroll(map, DIR_LEFT);
				}
			}
			move |= DIR_LEFT;
		}
		if (dir->current & DIR_RIGHT) {
			dir->moved |= DIR_RIGHT;
			if (dir->flags & DIR_ONMAP) {
				increase(mapx, 1, map->mapw - 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapx - *mapx) <=
				    -map->view->mapw) {
					scroll(map, DIR_RIGHT);
				}
			}
			move |= DIR_RIGHT;
		}
	}

	return (move);
}


