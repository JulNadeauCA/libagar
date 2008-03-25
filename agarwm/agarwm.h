/*	Public domain	*/

#include <sys/types.h>

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
#endif

#define Fatal AG_FatalError

#include "client.h"

__BEGIN_DECLS
extern NS_Server wmServer;		/* Network server object */
extern AG_Thread wmServerThread;	/* Network server thread */
extern WM_Client wmClientVFS;		/* Client VFS */
__END_DECLS
