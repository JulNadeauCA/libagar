/*	Public domain	*/

#include <agar/config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*
 * AG_Malloc(): Malloc wrapper which raises an exception on failure.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nonnull _Malloc_Like_Attribute
AG_Malloc(AG_Size len)
#else
void *
ag_malloc(AG_Size len)
#endif
{
	void *p;

	if ((p = malloc(len)) == NULL) {
#ifdef __CC65__
		AG_Verbose("Heap avail=%d (max %d)\n", _heapmemavail(),
		                                       _heapmaxavail());
#endif
		AG_FatalError("malloc");
	}
	return (p);
}

/*
 * AG_TryMalloc(): Malloc wrapper which returns NULL and sets an
 * error message on failure.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable _Malloc_Like_Attribute
AG_TryMalloc(AG_Size len)
#else
void *
ag_try_malloc(AG_Size len)
#endif
{
	void *p;

	if ((p = malloc(len)) == NULL) {
		AG_SetErrorV("E0", "Out of memory");
		return (NULL);
	}
	return (p);
}

/*
 * AG_Realloc(): Realloc wrapper which raises an exception on failure.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nonnull
AG_Realloc(void *_Nullable pOld, AG_Size len)
#else
void *
ag_realloc(void *pOld, AG_Size len)
#endif
{
	void *p;
	if ((p = realloc(pOld, len)) == NULL) {
		AG_FatalError("realloc");
	}
	return (p);
}

/*
 * AG_TryRealloc(): Realloc wrapper which returns NULL and sets an
 * error message on failure.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable
AG_TryRealloc(void *_Nullable pOld, AG_Size len)
#else
void *
ag_try_realloc(void *pOld, AG_Size len)
#endif
{
	void *p;

	if ((p = realloc(pOld, len)) == NULL) {
		AG_SetErrorV("E0", "Out of memory");
	}
	return (p);
}

/*
 * AG_Free(): Wrapper around free().
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_Free(void *_Nullable p)
#else
void
ag_free(void *p)
#endif
{
	free(p);
}
