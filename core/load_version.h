/*	Public domain	*/

#ifndef	_AGAR_CORE_LOAD_VERSION_H_
#define	_AGAR_CORE_LOAD_VERSION_H_
#include <agar/core/begin.h>

typedef struct ag_version {
	Uint32 major;		/* TODO 2.0: 16-bit */
	Uint32 minor;
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
} AG_Version;

/* TODO 2.0: shorten this */
#define AG_VERSION_NAME_MAX	48
#define AG_VERSION_MAX		(AG_VERSION_NAME_MAX+8)

__BEGIN_DECLS

#ifdef AG_SERIALIZATION

int AG_ReadVersion(AG_DataSource *_Nonnull, const char *_Nonnull,
                   const AG_Version *_Nonnull, AG_Version *_Nullable);
int AG_WriteVersion(AG_DataSource *_Nonnull, const char *_Nonnull,
                    const AG_Version *_Nonnull);

int  AG_ReadObjectVersion(AG_DataSource *_Nonnull, void *_Nonnull,
                          AG_Version *_Nullable);
void AG_WriteObjectVersion(AG_DataSource *_Nonnull, void *_Nonnull);

#endif /* AG_SERIALIZATION */

__END_DECLS

#include <agar/core/close.h>
#endif	/* _AGAR_CORE_LOAD_VERSION_H_ */
