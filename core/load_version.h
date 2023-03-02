/*	Public domain	*/

#ifndef	_AGAR_CORE_LOAD_VERSION_H_
#define	_AGAR_CORE_LOAD_VERSION_H_
#include <agar/core/begin.h>

#if AG_MODEL == AG_SMALL

typedef struct ag_version {
	Uint8 major;              /* Major version number */
	Uint8 minor;              /* Minor version number */
	Uint16 cid;	          /* Class ID */
	Uint16 unicode;           /* Icon (Unicode offset from 0xE000) */
} AG_Version;

#else /* !AG_SMALL */

typedef struct ag_version {
	Uint32 major;              /* Major version number */
	Uint32 minor;              /* Minor version number */
	Uint32 cid;	           /* Class ID */
	Uint32 unicode;            /* Icon (UCS-4 Unicode value) */
} AG_Version;

#endif /* !AG_SMALL */

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
