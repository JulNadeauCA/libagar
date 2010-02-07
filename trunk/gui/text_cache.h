#ifndef _AGAR_GUI_TEXT_CACHE_H_
#define _AGAR_GUI_TEXT_CACHE_H_
#include <agar/gui/begin.h>

typedef struct ag_cached_text {
	char *text;				/* Text string */
	int surface;				/* Surface mapping */
	Uint32 stamp;				/* Access timestamp */
	AG_TextState state;			/* Text rendering state */
	AG_SLIST_ENTRY(ag_cached_text) ents;
} AG_CachedText;

typedef struct ag_text_cache_bucket {
	AG_SLIST_HEAD_(ag_cached_text) ents;
} AG_TextCacheBucket;

typedef struct ag_text_cache {
	AG_Widget          *widget;		/* Widget managing surfaces */
	AG_TextCacheBucket *buckets;		/* Hash table */
	Uint               nBuckets;
	Uint               curEnts;		/* Current entries */
	AG_CachedText    **toExpire;		/* Entries to expire */
	Uint              *toExpireHashes;	/* Entries to expire (hashes) */
	Uint              nToExpire;
} AG_TextCache;

__BEGIN_DECLS
AG_TextCache *AG_TextCacheNew(void *, Uint, Uint);
void          AG_TextCacheDestroy(AG_TextCache *);
int           AG_TextCacheGet(AG_TextCache *, const char *);

static __inline__ Uint
AG_TextCacheHash(AG_TextCache *tc, const char *s)
{
	Uint h;
	Uchar *p;

	for (h = 0, p = (Uchar *)s; *p != '\0'; p++) {
		h = 31*h + *p;
	}
	return (h % tc->nBuckets);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_TEXT_CACHE_H_ */
