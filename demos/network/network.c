/*	Public domain	*/
/*
 * Test the network functions in ag_core.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

char testHost[128], testPort[16];
AG_Console *cons;

static void
TestConnect(AG_Event *event)
{
	AG_Window *win;
	AG_NetSocket *ns = NULL;
	AG_NetAddrList *nal = NULL;

	/* Create a new stream socket, with no defined address family. */
	if ((ns = AG_NetSocketNew(0, AG_NET_STREAM, 0)) == NULL)
		goto fail;

	/* Resolve the hostname and port number (or service name). */
	AG_ConsoleMsg(cons, "Resolving %s:%s...", testHost, testPort);
	if ((nal = AG_NetResolve(testHost, testPort, 0)) == NULL) {
		AG_NetSocketFree(ns);
		goto fail;
	}

	/*
	 * Establish a connection with one of the resolved addresses.
	 * The socket will inherit the address family of the NetAddress.
	 */
	AG_ConsoleMsg(cons, "Connecting...");
	if (AG_NetConnect(ns, nal) == -1) {
		AG_NetAddrListFree(nal);
		AG_NetSocketFree(ns);
		goto fail;
	}

	AG_ConsoleMsg(cons, "Connected to %s [%s]",
	    AG_NetAddrNumerical(ns->addrRemote),
	    agNetAddrFamilyNames[ns->addrRemote->family]);

	AG_NetClose(ns);

	AG_NetAddrListFree(nal);
	AG_NetSocketFree(ns);
	return;
fail:
	AG_ConsoleMsg(cons, "Failed: %s", AG_GetError());
	return;
}

static void
TestServer(AG_Event *event)
{
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
			AG_ConsoleMsg(cons, "%s: %s; skipping",
			    AG_NetAddrNumerical(na), AG_GetError());
			AG_NetSocketFree(ns);
			continue;
		}
		AG_ConsoleMsg(cons, "Bound to %s:%d", AG_NetAddrNumerical(na),
		    na->port);

		/* Add this socket to the set for polling. */
		ns->poll |= AG_NET_POLL_READ;
		ns->poll |= AG_NET_POLL_EXCEPTIONS;
		AG_TAILQ_INSERT_TAIL(&sockSet, ns, sockets);
	}
	AG_NetAddrListFree(ifc);

	for (;;) {
		/* Poll sockSet for read/exception conditions. */
		if (AG_NetPoll(&sockSet, &rdSet, NULL, &exceptSet, 0) < 1) {
			AG_ConsoleMsg(cons, "Poll: %s", AG_GetError());
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
				AG_ConsoleMsg(cons, "Connection on [%s:%d] from [%s]!",
				    AG_NetAddrNumerical(ns->addrLocal),
				    ns->addrLocal->port,
				    AG_NetAddrNumerical(nsNew->addrRemote));

				nsNew->poll |= AG_NET_POLL_READ;
				nsNew->poll |= AG_NET_POLL_WRITE;
				nsNew->poll |= AG_NET_POLL_EXCEPTIONS;
				AG_TAILQ_INSERT_TAIL(&sockSet, nsNew, sockets);
			
				AG_Strlcpy(buf, "Hello\n", sizeof(buf));
				if (AG_NetWrite(nsNew, buf, sizeof(buf), NULL)
				    == -1) {
					goto fail;
				}
			} else {
				size_t nRead;

				if (AG_NetRead(ns, buf, sizeof(buf), &nRead) == -1) {
					goto fail;
				}
				if (nRead == 0) {
					AG_ConsoleMsg(cons,
					    "Closing connection to [%s]",
					    AG_NetAddrNumerical(ns->addrRemote));
					AG_TAILQ_REMOVE(&sockSet, ns, sockets);
					AG_NetSocketFree(ns);
				} else {
					buf[nRead-1] = '\0';
					AG_ConsoleMsg(cons, "Data from [%s]: %s",
					    AG_NetAddrNumerical(ns->addrRemote),
					    buf);
				}
			}
		}
		AG_TAILQ_FOREACH(ns, &exceptSet, except) {
			AG_ConsoleMsg(cons, "Exception on socket [%s]",
			    ns->addrRemote ? AG_NetAddrNumerical(ns->addrRemote) : "");
		}
	}
	AG_NetSocketSetFree(&sockSet);
	return;
fail:
	AG_ConsoleMsg(cons, "Failed: %s", AG_GetError());
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Tlist *tl;
	AG_TlistItem *ti;
	AG_NetAddr *na;
	AG_NetAddrList *ifc;
	AG_Button *btn;
	AG_Event *ev;
	AG_Textbox *tb;
	AG_Box *hBox;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar network interface demo");

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
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(hBox, AG_TEXTBOX_HFILL, "Test Host: ");
		AG_TextboxBindUTF8(tb, testHost, sizeof(testHost));
		tb = AG_TextboxNewS(hBox, 0, " Port: ");
		AG_TextboxSizeHint(tb, "<1234>");
		AG_TextboxBindUTF8(tb, testPort, sizeof(testPort));
	}

	btn = AG_ButtonNew(win, AG_BUTTON_HFILL, "Connect (in separate thread)");
	ev = AG_SetEvent(btn, "button-pushed", TestConnect, NULL);
	ev->flags |= AG_EVENT_ASYNC;
	
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Connect (in main thread)",
	    TestConnect, NULL);

	btn = AG_ButtonNew(win, AG_BUTTON_HFILL, "Start test server");
	ev = AG_SetEvent(btn, "button-pushed", TestServer, NULL);
	ev->flags |= AG_EVENT_ASYNC;

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 380, 500);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *optArg;
	int c;

	AG_Strlcpy(testHost, "libagar.org", sizeof(testHost));
	AG_Strlcpy(testPort, "80", sizeof(testPort));

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: network [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
