/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>

#include "tileset.h"

void
feature_init(struct feature *ft, const char *type, const char *name,
    const void *ops)
{
	strlcpy(ft->name, name, sizeof(ft->name));
	ft->type = type;
	ft->ops = ops;
	ft->ops->init(ft);
	TAILQ_INIT(&ft->sketches);
}

/* Associate a sketch with the feature. */
struct feature_sketch *
feature_insert_sketch(struct feature *ft, struct sketch *sk)
{
	struct feature_sketch *fsk;

	fsk = Malloc(sizeof(struct feature_sketch), M_OBJECT);
	fsk->x = 0;
	fsk->y = 0;
	fsk->visible = 1;
	fsk->suppressed = 0;
	TAILQ_INSERT_TAIL(&ft->sketches, fsk, sketches);
	return (fsk);
}

void
feature_remove_sketch(struct feature *ft, struct sketch *sk)
{
	struct feature_sketch *fsk;

	TAILQ_FOREACH(fsk, &ft->sketches, sketches) {
		if (fsk->sk == sk)
			break;
	}
	if (fsk != NULL) {
		TAILQ_REMOVE(&ft->sketches, fsk, sketches);
		Free(fsk, M_RG);
	}
}

void
feature_destroy(struct feature *ft)
{

}

