/*	Public domain	*/

#ifndef _AGAR_NET_H_
#define _AGAR_NET_H_

#include <sys/types.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#include "command.h"
#include "error.h"

#ifndef Malloc
#define Malloc(sz) AGN_Malloc(sz)
#endif
#ifndef Realloc
#define Realloc(p,sz) AGN_Realloc((p),(sz))
#endif

#endif /* _AGAR_NET_H_ */
