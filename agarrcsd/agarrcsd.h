/*	Public domain	*/

#include <sys/types.h>

#include <agar/core.h>
#include <agar/net.h>

#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
#endif
#define Malloc AG_Malloc
#define Free AG_Free

#include <config/debug.h>
#ifdef DEBUG
# define dprintf(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
#else
# define dprintf(fmt, args...)
#endif

#include "user.h"

extern struct user *user;
