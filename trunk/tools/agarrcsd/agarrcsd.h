/*	Public domain	*/

#include <sys/types.h>

#include <agar/core.h>
#include <agar/core/net.h>
#include <agar/core/snprintf.h>

typedef unsigned int Uint;

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
