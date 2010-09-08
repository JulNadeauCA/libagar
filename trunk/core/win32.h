/*	Public domain	*/
/*
 * Try to safely include <windows.h>
 */
#include <agar/config/have_cygwin.h>

#if defined(HAVE_CYGWIN)
# include <agar/core/queue_close.h>		/* Naming conflicts */
# define USE_SYS_TYPES_FD_SET			/* Warning under Cygwin */
# include <windows.h>
# include <agar/core/queue_close.h>		/* Naming conflicts */
# include <agar/core/queue.h>
#elif defined(_WIN32) && !defined(_XBOX)
# include <agar/core/queue_close.h>		/* Naming conflicts */
# include <windows.h>
# include <agar/core/queue_close.h>		/* Naming conflicts */
# include <agar/core/queue.h>
#endif
