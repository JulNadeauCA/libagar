/*	Public domain	*/
/*
 * Test the AG_Net(3) network interface.
 */

#include "agartest.h"

#ifdef AG_NETWORK

#include <string.h>

typedef struct {
	AG_TestInstance _inherit;
	char testHost[128];
	char testPort[16];
	AG_Console *cons;
} MyTestInstance;

static void
TestConnect(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_NetSocket *ns = NULL;
	AG_NetAddrList *nal = NULL;

	/* Create a new stream socket, with no defined address family. */
	if ((ns = AG_NetSocketNew(0, AG_NET_STREAM, 0)) == NULL)
		goto fail;

	/* Resolve the hostname and port number (or service name). */
	AG_ConsoleMsg(ti->cons, "Resolving %s:%s...", ti->testHost, ti->testPort);
	if ((nal = AG_NetResolve(ti->testHost, ti->testPort, 0)) == NULL) {
		AG_NetSocketFree(ns);
		goto fail;
	}

	/*
	 * Establish a connection with one of the resolved addresses.
	 * The socket will inherit the address family of the NetAddress.
	 */
	AG_ConsoleMsg(ti->cons, "Connecting...");
	if (AG_NetConnect(ns, nal) == -1) {
		AG_NetAddrListFree(nal);
		AG_NetSocketFree(ns);
		goto fail;
	}

	AG_ConsoleMsg(ti->cons, "Connected to %s [%s]",
	    AG_NetAddrNumerical(ns->addrRemote),
	    agNetAddrFamilyNames[ns->addrRemote->family]);

	AG_NetClose(ns);

	AG_NetAddrListFree(nal);
	AG_NetSocketFree(ns);
	return;
fail:
	AG_ConsoleMsg(ti->cons, "Failed: %s", AG_GetError());
	return;
}

static void
TestServer(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	char buf[128];
	AG_NetAddrList *ifc;
	AG_NetAddr *na;
	AG_NetSocket *ns;
	AG_NetSocketSet sockSet, rdSet, exceptSet;

	AG_NetSocketSetInit(&sockSet);
	AG_NetSocketSetInit(&rdSet);
	AG_NetSocketSetInit(&exceptSet);

	/* Retrieve the locally configured network addresses. */
	if ((ifc = AG_NetGetIfConfig()) == NULL) {
		goto fail;
	}
	AG_TAILQ_FOREACH(na, ifc, addrs) {
		/* Skip any non-internet address. */
		if (na->family != AG_NET_INET4 &&
		    na->family != AG_NET_INET6)
			continue;

		/* Create a new stream socket bound to port 3456. */
		if ((ns = AG_NetSocketNew(na->family, AG_NET_STREAM, 0))
		    == NULL) {
			goto fail;
		}
		AG_NetSetOptionInt(ns, AG_NET_REUSEADDR, 1);
		na->port = 3456;
		if (AG_NetBind(ns, na) == -1) {
			AG_ConsoleMsg(ti->cons, "bind(%s): %s; skipping",
			    AG_NetAddrNumerical(na), AG_GetError());
			AG_NetSocketFree(ns);
			continue;
		}
		AG_ConsoleMsg(ti->cons, "Bound to %s:%d",
		    AG_NetAddrNumerical(na), na->port);

		/* Add this socket to the set for polling. */
		ns->poll |= AG_NET_POLL_READ;
		ns->poll |= AG_NET_POLL_EXCEPTIONS;
		AG_TAILQ_INSERT_TAIL(&sockSet, ns, sockets);
	}
	AG_NetAddrListFree(ifc);

	for (;;) {
		/* Poll sockSet for read/exception conditions. */
		if (AG_NetPoll(&sockSet, &rdSet, NULL, &exceptSet, 0) < 1) {
			AG_ConsoleMsg(ti->cons, "Poll: %s", AG_GetError());
			break;
		}
		AG_TAILQ_FOREACH(ns, &rdSet, read) {
			/*
			 * Read condition on a bound socket indicates
			 * an incoming connection that we must accept.
			 */
			if (ns->flags & AG_NET_SOCKET_BOUND) {
				AG_NetSocket *nsNew;

				if ((nsNew = AG_NetAccept(ns)) == NULL) {
					goto fail;
				}
				AG_ConsoleMsg(ti->cons,
				    "Connection on [%s:%d] from [%s]!",
				    AG_NetAddrNumerical(ns->addrLocal),
				    ns->addrLocal->port,
				    AG_NetAddrNumerical(nsNew->addrRemote));

				nsNew->poll |= AG_NET_POLL_READ;
				nsNew->poll |= AG_NET_POLL_WRITE;
				nsNew->poll |= AG_NET_POLL_EXCEPTIONS;
				AG_TAILQ_INSERT_TAIL(&sockSet, nsNew, sockets);
			
				AG_Strlcpy(buf, "Hello\n", sizeof(buf));
				if (AG_NetWrite(nsNew, buf, strlen(buf), NULL)
				    == -1) {
					goto fail;
				}
			} else {
				AG_Size nRead;

				if (AG_NetRead(ns, buf, sizeof(buf), &nRead) == -1) {
					goto fail;
				}
				if (nRead == 0) {
					AG_ConsoleMsg(ti->cons,
					    "Closing connection to [%s]",
					    AG_NetAddrNumerical(ns->addrRemote));
					AG_TAILQ_REMOVE(&sockSet, ns, sockets);
					AG_NetSocketFree(ns);
				} else {
					buf[nRead-1] = '\0';
					AG_ConsoleMsg(ti->cons,
					    "Data from [%s]: %s",
					    AG_NetAddrNumerical(ns->addrRemote),
					    buf);
				}
			}
		}
		AG_TAILQ_FOREACH(ns, &exceptSet, except) {
			AG_ConsoleMsg(ti->cons, "Exception on socket [%s]",
			    ns->addrRemote ? AG_NetAddrNumerical(ns->addrRemote) : "");
		}
	}
	AG_NetSocketSetFree(&sockSet);
	return;
fail:
	AG_ConsoleMsg(ti->cons, "Failed: %s", AG_GetError());
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	AG_Strlcpy(ti->testHost, "libagar.org", sizeof(ti->testHost));
	AG_Strlcpy(ti->testPort, "80", sizeof(ti->testPort));
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Tlist *tl;
	AG_NetAddr *na;
	AG_NetAddrList *ifc;
	AG_Button *btn;
	AG_Event *ev;
	AG_Textbox *tb;
	AG_Box *hBox;

	if ((ifc = AG_NetGetIfConfig()) != NULL) {
		AG_LabelNewS(win, 0, "Local addresses:");
		tl = AG_TlistNew(win, AG_TLIST_HFILL);
		AG_TlistSizeHint(tl, "[inet4] XXX.XXX.XXX.XXX", 4);

		AG_TAILQ_FOREACH(na, ifc, addrs) {
			AG_TlistAdd(tl, NULL, "[%s] %s",
			    agNetAddrFamilyNames[na->family],
			    AG_NetAddrNumerical(na));
		}
		AG_NetAddrListFree(ifc);
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
	}
		
	AG_LabelNewS(win, 0, "Status:");
	ti->cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(hBox, AG_TEXTBOX_HFILL, "Test Host: ");
		AG_TextboxBindUTF8(tb, ti->testHost, sizeof(ti->testHost));
		tb = AG_TextboxNewS(hBox, 0, " Port: ");
		AG_TextboxSizeHint(tb, "<1234>");
		AG_TextboxBindUTF8(tb, ti->testPort, sizeof(ti->testPort));
	}

	btn = AG_ButtonNew(win, AG_BUTTON_HFILL, "Connect (in separate thread)");
	ev = AG_SetEvent(btn, "button-pushed", TestConnect, "%p", ti);
	ev->flags |= AG_EVENT_ASYNC;
	
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Connect (in main thread)",
	    TestConnect, "%p", ti);

	btn = AG_ButtonNew(win, AG_BUTTON_HFILL, "Start test server");
	ev = AG_SetEvent(btn, "button-pushed", TestServer, "%p", ti);
	ev->flags |= AG_EVENT_ASYNC;

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 380, 500);
	return (0);
}

const AG_TestCase networkTest = {
	"network",
	N_("Test the AG_Net(3) interface"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

#endif /* AG_NETWORK */
