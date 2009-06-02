/*	Public domain	*/

#include <sys/types.h>

#include <agar/core.h>
#include <agar/core/net.h>
#include <agar/core/snprintf.h>

#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
#endif

#define Malloc AG_Malloc
#define Free AG_Free
#define Strlcpy AG_Strlcpy
#define Strlcat AG_Strlcat
#define Snprintf AG_Snprintf
#define Vsnprintf AG_Vsnprintf
#define Vasprintf AG_Vasprintf
#define Strsep AG_Strsep

#include "user.h"

extern struct user *user;
extern AG_Object UserMgr;
