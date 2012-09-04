/*	Public domain	*/

#ifndef _AGAR_CORE_VERSION_H_
#define _AGAR_CORE_VERSION_H_
#include <agar/core/begin.h>

#define AGAR_MAJOR_VERSION	1
#define AGAR_MINOR_VERSION	5
#define AGAR_PATCHLEVEL		0

typedef struct ag_agar_version {
	int major;
	int minor;
	int patch;
	const char *release;
} AG_AgarVersion;

#define AG_VERSION_NUM(X,Y,Z) ((X)*1000 + (Y)*100 + (Z))
#define AG_COMPILED_VERSION \
	AG_VERSION_NUM(AGAR_MAJOR_VERSION, AGAR_MINOR_VERSION, AGAR_PATCHLEVEL)
#define AG_VERSION_ATLEAST(X,Y,Z) \
	(AG_COMPILED_VERSION >= AG_VERSION_NUM(X, Y, Z))

__BEGIN_DECLS
void  AG_GetVersion(AG_AgarVersion *);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_VERSION_H_ */
