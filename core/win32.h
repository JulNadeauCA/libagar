/*	Public domain	*/
/*
 * Try to safely include <windows.h>
 */
#include <agar/config/have_cygwin.h>

#ifdef _WIN32
# include <agar/core/queue_close.h>		/* Naming conflicts */
# ifdef HAVE_CYGWIN
#  define USE_SYS_TYPES_FD_SET			/* Warning under Cygwin */
# endif
# include <windows.h>
# include <agar/core/queue_close.h>
# include <agar/core/queue.h>
#endif
