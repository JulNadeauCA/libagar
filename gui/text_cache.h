/*	Public domain	*/

#ifndef _AGAR_GUI_TEXT_CACHE_H_
#define _AGAR_GUI_TEXT_CACHE_H_

#include <agar/gui/text.h>
#include <agar/gui/begin.h>

typedef struct ag_cached_text {
	char *_Nonnull text;			/* Text string */
	int surface;				/* Surface mapping */
	Uint32 _pad;
	AG_TextState state;			/* Text rendering state */
	AG_TAILQ_ENTRY(ag_cached_text) ents;
} AG_CachedText;

typedef struct ag_text_cache_bucket {
	AG_TAILQ_HEAD(ag_cached_textq,ag_cached_text) ents;
	Uint nEnts;
	Uint32 _pad;
} AG_TextCacheBucket;

struct ag_widget;

typedef struct ag_text_cache {
	struct ag_widget *_Nonnull widget;	/* Widget managing surfaces */
	AG_TextCacheBucket *_Nonnull buckets;	/* Hash table */
	Uint                        nBuckets;
	Uint curEnts;				/* Current entries */
	Uint nBucketEnts;			/* Target bucket utilization */
	Uint32 _pad;
} AG_TextCache;

__BEGIN_DECLS
AG_TextCache *_Nonnull AG_TextCacheNew(void *_Nonnull, Uint, Uint);

void AG_TextCacheClear(AG_TextCache *_Nonnull);
void AG_TextCacheDestroy(AG_TextCache *_Nonnull);
int  AG_TextCacheGet(AG_TextCache *_Nonnull, const char *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_TEXT_CACHE_H_ */
