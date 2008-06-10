/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include "view.h"
#include "text.h"
#include "widget.h"
#include "text_cache.h"

AG_TextCache *
AG_TextCacheNew(void *widget, Uint nBuckets, Uint nToExpire)
{
	AG_TextCache *tc;
	Uint i;

	fprintf(stderr, "TextCacheNew(): %d buckets, %d expire queue\n",
	    nBuckets, nToExpire);
	tc = Malloc(sizeof(AG_TextCache));
	tc->widget = widget;
	tc->curEnts = 0;
	tc->buckets = Malloc(nBuckets*sizeof(AG_TextCacheBucket));
	tc->nBuckets = nBuckets;
	tc->toExpire = Malloc(nBuckets*sizeof(AG_CachedText *));
	tc->toExpireHashes = Malloc(nBuckets*sizeof(Uint));
	tc->nToExpire = nToExpire;

	for (i = 0; i < nBuckets; i++) {
		SLIST_INIT(&tc->buckets[i].ents);
	}
	return (tc);
}

void
AG_TextCacheDestroy(AG_TextCache *tc)
{
	Uint i;
	AG_CachedText *ct;

	fprintf(stderr, "TextCacheDestroy: freeing %d buckets\n", tc->nBuckets);
	for (i = 0; i < tc->nBuckets; i++) {
		SLIST_FOREACH(ct, &tc->buckets[i].ents, ents) {
			AG_WidgetUnmapSurface(tc->widget, ct->surface);
			Free(ct->text);
			free(ct);
		}
	}
	Free(tc->buckets);
	Free(tc->toExpire);
	Free(tc->toExpireHashes);
}

static __inline__ void
DeleteEntry(AG_TextCache *tc, Uint h, AG_CachedText *ct)
{
	fprintf(stderr, "TextCache: removing entry #%u (\"%s\")\n", h,
	    ct->text);
	SLIST_REMOVE(&tc->buckets[h].ents, ct, ag_cached_text, ents);
	AG_WidgetUnmapSurface(tc->widget, ct->surface);
	free(ct->text);
	free(ct);
	tc->curEnts--;
}

static void
ExpireEntries(AG_TextCache *tc)
{
	AG_CachedText *ct;
	Uint i, j;

	for (i = 0; i < tc->nToExpire; i++) {
		tc->toExpire[i] = NULL;
	}
	for (i = 0; i < tc->nBuckets; i++) {
		SLIST_FOREACH(ct, &tc->buckets[i].ents, ents) {
			for (j = 0; j < tc->nToExpire; j++) {
				if (tc->toExpire[j] == NULL ||
				    ct->stamp < tc->toExpire[j]->stamp) {
				    	tc->toExpire[j] = ct;
				    	tc->toExpireHashes[j] = i;
					break;
				}
			}
		}
	}
	for (i = 0; i < tc->nToExpire; i++) {
		if (tc->toExpire[i] == NULL) {
			continue;
		}
		DeleteEntry(tc, tc->toExpireHashes[i], tc->toExpire[i]);
	}
}

int
AG_TextCacheInsLookup(AG_TextCache *tc, const char *text)
{
	AG_TextCacheBucket *bucket;
	AG_CachedText *ct;
	Uint h;

	h = AG_TextCacheHash(tc, text);
	fprintf(stderr, "TextCacheLookup: string \"%s\" = %u...", text, h);
	bucket = &tc->buckets[h];
	SLIST_FOREACH(ct, &bucket->ents, ents) {
		if (strcmp(ct->text, text) == 0 &&
		    AG_TextStateCompare(&ct->state, agTextState) == 0)
			break;
	}
	if (ct == NULL) {
		fprintf(stderr, "MISS (ent %u)\n", tc->curEnts+1);
		ct = Malloc(sizeof(AG_TextCache));
		ct->text = Strdup(text);
		ct->surface = AG_WidgetMapSurface(tc->widget,
		    AG_TextRender(text));
		ct->stamp = SDL_GetTicks();
		ct->state = *agTextState;
		tc->curEnts++;
		SLIST_INSERT_HEAD(&bucket->ents, ct, ents);

		if (tc->curEnts > tc->nBuckets) {
			fprintf(stderr, "TextCache: Expiring entries "
			    "(%u > %u)...\n", tc->curEnts, tc->nBuckets);
			ExpireEntries(tc);
		}
	} else {
		ct->stamp = SDL_GetTicks();
		fprintf(stderr, "HIT (%u)\n", ct->stamp);
	}
	return (ct->surface);
}
