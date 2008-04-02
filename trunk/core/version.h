/*	Public domain	*/

#ifndef _AGAR_CORE_VERSION_H_
#define _AGAR_CORE_VERSION_H_

#include "begin_code.h"

#define AGAR_MAJOR_VERSION	1
#define AGAR_MINOR_VERSION	3
#define AGAR_PATCHLEVEL		3

typedef struct ag_agar_version {
	int major;
	int minor;
	int patch;
	const char *release;
} AG_AgarVersion;

#define AG_VERSION_NUM(X,Y,Z) ((X)*1000 + (Y)*100 + (Z))
#define AG_COMPILED_VERSION \
	AG_VERSION_NUM(AG_MAJOR_VERSION, AG_MINOR_VERSION, AG_PATCHLEVEL)
#define AG_VERSION_ATLEAST(X,Y,Z) \
	(AG_COMPILED_VERSION >= AG_VERSION_NUM(X, Y, Z))

__BEGIN_DECLS
void  AG_GetVersion(AG_AgarVersion *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_CORE_VERSION_H_ */
