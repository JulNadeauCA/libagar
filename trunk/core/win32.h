/*	Public domain	*/
/*
 * Try to safely include <windows.h> on Windows platforms.
 */

#if defined(_WIN32) && !defined(_XBOX)

/*
 * Needed for WM_MOUSEWHEEL to be defined. Idiotic SDK provides us with no way
 * of testing whether WM_MOUSEHWEEL is defined, so we are forced to set this.
 */
#define _WIN32_WINDOWS 0x0401

/* <windows.h> tries to redefine SLIST_ENTRY(), etc. */
#include <agar/core/queue_close.h>

/* Fix some warnings under Cygwin. */
#include <agar/config/have_cygwin.h>
#if defined(HAVE_CYGWIN)
# define USE_SYS_TYPES_FD_SET
#endif

#include <windows.h>

/* Redefine the AG_Queue(3) macros */
#include <agar/core/queue_close.h>
#include <agar/core/queue.h>

#endif /* _WIN32 and !_XBOX */
