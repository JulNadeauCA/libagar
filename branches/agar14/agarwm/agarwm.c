/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config/version.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "agarwm.h"
#include "pathnames.h"
#include "protocol.h"

#include <agar/gui.h>
#include <agar/dev.h>

NS_Server wmServer;			/* Network server object */
AG_Thread wmServerThread;		/* Network server thread */
WM_Client wmClientVFS;			/* Client VFS */

#if 0
static volatile int hup_flag = 0;
static void sig_hup(int sigraised) { hup_flag++; }
#endif

static int
version(NS_Server *ns, NS_Command *cmd, void *p)
{
	char hostname[128];

	gethostname(hostname, sizeof(hostname));
	NS_BeginList(ns);
	NS_ListString(ns, "agarwm:%s:%s", VERSION, hostname);
	NS_EndList(ns);
	return (0);
}

static int
WindowNew(NS_Server *ns, NS_Command *cmd, void *p)
{
	static int count = 0;
	AG_Window *win;

	if ((win = AG_WindowNewNamed(0, "win-remote%d", count++)) == NULL) {
		return (-1);
	}
	AG_LabelNewStatic(win, 0, "This is a remote window");
	AG_WindowShow(win);
	
	NS_BeginList(ns);
	NS_ListString(ns, "%s", AGOBJECT(win)->name);
	NS_EndList(ns);
	
	AG_ViewAttach(win);

	AG_TextMsg(AG_MSG_INFO, "Created window %s", AGOBJECT(win)->name);
	
	fprintf(stderr, "Created window: %s\n", AGOBJECT(win)->name);
	fprintf(stderr, "Scanning\n");
	AG_LockVFS(agView);
	TAILQ_FOREACH(win, &agView->windows, windows) {
		printf("Current windows: %s\n", AGOBJECT(win)->name);
	}
	AG_UnlockVFS(agView);
	fprintf(stderr, "Scanning OK\n");
	return (0);
}

static AG_Window *
GetWindow(NS_Command *cmd, const char *key)
{
	char *name = NS_CommandString(cmd, key);
	char sp[32];
	AG_Window *win;

	TAILQ_FOREACH(win, &agView->windows, windows) {
		if (strcmp(name, AGOBJECT(win)->name) == 0)
			break;
	}
	if (win == NULL) {
		AG_SetError("No such window");
	}
	return (win);
}

static int
WindowSetCaption(NS_Server *ns, NS_Command *cmd, void *p)
{
	AG_Window *win = GetWindow(cmd,"win");
	char *cap = NS_CommandString(cmd,"caption");

	fprintf(stderr, "Setting caption: %s (win=%p)\n", cap, win);
	if (win == NULL || cap == NULL) {
		return (-1);
	}
	AG_WindowSetCaption(win, "%s", cap);
	return (0);
}

static int
auth_none(NS_Server *serv, void *p)
{
	char buf[32];
	(void)fgets(buf, sizeof(buf), stdin);
	return (1);
}

static void *
ServerLoop(void *p)
{
	if (NS_Listen(&wmServer) == -1) {
		Fatal("NS_Listen: %s", AG_GetError());
	}
	AG_ThreadExit(NULL);
	return (NULL);
}

int
main(int argc, char *argv[])
{
	char *host = "localhost";
	char *port = _PROTO_PORT;
	char *sockPath = _PATH_SOCKET;
	int w = 640, h = 480;
/*	struct sigaction sa; */
	int rv, i;
	
	if (AG_InitCore("agarwm", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((i = getopt(argc, argv, "w:h:b:p:s:")) != -1) {
		switch (i) {
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'b':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 's':
			sockPath = optarg;
			break;
		case 'v':
			printf("AgarWM %s\n", VERSION);
			exit(0);
		case '?':
		default:
			printf("Usage: agarwm [-v] [-b host] [-p port] "
			       "[-w width] [-h height] [-s socket-path]\n");
			exit(1);
		}
	}
#if 0
	sa.sa_flags = 0;
	sa.sa_handler = sig_hup;
	sigaction(SIGHUP, &sa, NULL);
#endif

	AG_RegisterClass(&wmClientClass);
	AG_ObjectInitStatic(&wmClientVFS, &wmClientClass);
	AG_ObjectSetName(&wmClientVFS, "Debugger");

	if (AG_InitVideo(w, h, 32, 0) == -1) {
		Fatal("Initializing video: %s", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);

	/* Initialize Agar-DEV for debugging help. */
	DEV_InitSubsystem(0);
	DEV_Browser(&wmClientVFS);

	/* Initialize the server. */
	NS_InitSubsystem(0);
	AG_ObjectInitStatic(&wmServer, &nsServerClass);
	AG_ObjectSetName(&wmServer, "_wmServer");
	NS_ServerSetProtocol(&wmServer, _PROTO_NAME, VERSION);
	NS_ServerBind(&wmServer, host, port);
	NS_RegAuthMode(&wmServer, "none", auth_none, NULL);

	/* Register the daemon functions. */
	NS_RegCmd(&wmServer, "version", version, NULL);
	NS_RegCmd(&wmServer, "WindowNew", WindowNew, NULL);
	NS_RegCmd(&wmServer, "WindowSetCaption", WindowSetCaption, NULL);

	/* Create the server thread. */
	fprintf(stderr, "Listening on %s:%s\n", host, port);
	if ((rv = AG_ThreadCreate(&wmServerThread, ServerLoop, NULL)) != 0) {
		Fatal("Spawning server thread: %s", AG_GetError());
	}

	/* Enter the GUI event loop. */
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
