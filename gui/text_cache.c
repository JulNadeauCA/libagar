/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * A cache of rendered text surfaces tagged with rendering attributes.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/text_cache.h>
#include <agar/gui/widget.h>

/* #define TEXTCACHE_DEBUG */

static void FreeCachedText(AG_TextCache *_Nonnull, AG_CachedText *_Nonnull);

static __inline__ Uint _Pure_Attribute
Hash_Text(AG_TextCache *_Nonnull tc, const char *_Nonnull s)
{
	Uint h;
	Uchar *p;

	for (h = 0, p = (Uchar *)s; *p != '\0'; p++) {
		h = 31*h + *p;
	}
	return (h % tc->nBuckets);
}

AG_TextCache *
AG_TextCacheNew(void *widget, Uint nBuckets, Uint nBucketEnts)
{
	AG_TextCache *tc;
	Uint i;

#ifdef TEXTCACHE_DEBUG
	Debug(widget, "TextCacheNew(): %d buckets\n", nBuckets);
#endif
	tc = Malloc(sizeof(AG_TextCache));
	tc->widget = widget;
	tc->curEnts = 0;
	tc->buckets = Malloc(nBuckets*sizeof(AG_TextCacheBucket));
	tc->nBuckets = nBuckets;
	tc->nBucketEnts = nBucketEnts;

	for (i = 0; i < nBuckets; i++) {
		AG_TextCacheBucket *buck = &tc->buckets[i];

		TAILQ_INIT(&buck->ents);
		buck->nEnts = 0;
	}
	return (tc);
}

void
AG_TextCacheClear(AG_TextCache *tc)
{
	Uint i;
	AG_CachedText *ct, *ctNext;

#ifdef TEXTCACHE_DEBUG
	Debug(NULL, "TextCacheDestroy: freeing %d buckets\n", tc->nBuckets);
#endif
	for (i = 0; i < tc->nBuckets; i++) {
		AG_TextCacheBucket *buck = &tc->buckets[i];

		for (ct = TAILQ_FIRST(&buck->ents);
		     ct != TAILQ_END(&buck->ents);
		     ct = ctNext) {
			ctNext = TAILQ_NEXT(ct, ents);
			FreeCachedText(tc, ct);
		}
		TAILQ_INIT(&buck->ents);
		buck->nEnts = 0;
	}
	tc->curEnts = 0;
}

void
AG_TextCacheDestroy(AG_TextCache *tc)
{
	AG_TextCacheClear(tc);
	Free(tc->buckets);
	free(tc);
}

/* Expire some of the oldest entries from the cache. */
static void
ExpireEntries(AG_TextCache *_Nonnull tc)
{
	AG_CachedText *ct;
	Uint i;

	for (i = 0; i < tc->nBuckets; i++) {
		AG_TextCacheBucket *buck = &tc->buckets[i];

		if (buck->nEnts > tc->nBucketEnts) {
			ct = TAILQ_LAST(&buck->ents, ag_cached_textq);
#ifdef TEXTCACHE_DEBUG
			Debug(NULL,
			    "TextCache: expiring entry #%u (\"%s\"), %u ents\n",
			    i, ct->text, buck->nEnts);
#endif
			TAILQ_REMOVE(&buck->ents, ct, ents);
			FreeCachedText(tc, ct);
			buck->nEnts--;
			tc->curEnts--;
		}
	}
}

static void
FreeCachedText(AG_TextCache *_Nonnull tc, AG_CachedText *_Nonnull ct)
{
	AG_WidgetUnmapSurface(tc->widget, ct->surface);
	Free(ct->text);
	free(ct);
}

/* Compare two text states. */
static __inline__ int
CompareTextStates(const AG_TextState *_Nonnull a, const AG_TextState *_Nonnull b)
{
	if (a->font == b->font &&
	    AG_ColorCompare(&a->color, &b->color) == 0 &&
	    AG_ColorCompare(&a->colorBG, &b->colorBG) == 0 &&
	    a->justify == b->justify &&
	    a->valign == b->valign &&
	    a->tabWd == b->tabWd) {
		return (0);
	}
	return (1);
}

int
AG_TextCacheGet(AG_TextCache *tc, const char *text)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_TextCacheBucket *buck;
	AG_CachedText *ct;
	AG_Surface *S;
	Uint h;

	h = Hash_Text(tc, text);
#ifdef TEXTCACHE_DEBUG
	Debug(NULL, "TextCacheLookup: string \"%s\" = %u...", text, h);
#endif
	buck = &tc->buckets[h];
	TAILQ_FOREACH(ct, &buck->ents, ents) {
		if (strcmp(ct->text, text) == 0 &&
		    CompareTextStates(&ct->state, ts) == 0)
			break;
	}
	if (ct == NULL) {
#ifdef TEXTCACHE_DEBUG
		Debug(NULL, "MISS (ent %u)\n", tc->curEnts+1);
#endif
		if ((S = AG_TextRender(text)) == NULL) {
			return (-1);
		}
		if ((ct = TryMalloc(sizeof(AG_CachedText))) == NULL) {
			goto fail;
		}
		if ((ct->text = strdup(text)) == NULL) {
			free(ct);
			goto fail;
		}
		ct->surface = AG_WidgetMapSurface(tc->widget, S);
		memcpy(&ct->state, ts, sizeof(AG_TextState));
		tc->curEnts++;
		TAILQ_INSERT_HEAD(&buck->ents, ct, ents);
		buck->nEnts++;

		if (tc->curEnts > tc->nBuckets*tc->nBucketEnts) {
#ifdef TEXTCACHE_DEBUG
			Debug(NULL, "TextCache: Expiring entries "
			    "(%u > %u)...\n", tc->curEnts,
			    tc->nBuckets*tc->nBucketEnts);
#endif
			ExpireEntries(tc);
		}
	} else {
		TAILQ_REMOVE(&buck->ents, ct, ents);
		TAILQ_INSERT_HEAD(&buck->ents, ct, ents);
#ifdef TEXTCACHE_DEBUG
		Debug(NULL, "HIT\n");
#endif
	}
	return (ct->surface);
fail:
	AG_SurfaceFree(S);
	return (-1);
}
