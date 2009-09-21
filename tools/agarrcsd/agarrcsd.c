/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <config/have_gethostname.h>

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

#include "agarrcsd.h"
#include "pathnames.h"
#include "protocol.h"
#include "rcs.h"

#include <agar/gui.h>
#include <agar/dev.h>

volatile int gothup = 0;
AG_Object UserMgr;			/* User manager object */
User *user;				/* Current user (child context) */
NS_Server *serv;			/* Server object (child context) */

static void
sig_hup(int sigraised)
{
	gothup++;
}

static int
version(NS_Server *serv, NS_Command *cmd, void *p)
{
	char hostname[128];

	hostname[0] = '\0';
#ifdef HAVE_GETHOSTNAME
	gethostname(hostname, sizeof(hostname));
#endif
	NS_Message(serv, 0, "agarrcsd:%s:%s", VERSION, hostname);
	return (0);
}

static int
auth_password(NS_Server *serv, void *p)
{
	char buf[65], *pBuf = &buf[0];
	char *name, *pass;
	User *u;
	size_t end;

	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (-1);

	if ((name = Strsep(&pBuf, ":")) == NULL ||
	    (pass = Strsep(&pBuf, ":")) == NULL) {
		return (-1);
	}
	end = strlen(pass)-1;
	if (pass[end] == '\n')
		pass[end] = '\0';

	if ((u = UserLookup(name)) == NULL) {
		NS_Log(NS_INFO, "%s: no such user", name);
		goto fail;
	}
	if (strcmp(pass, u->pass) == 0) {
		user = u;
		NS_Log(NS_INFO, "%s: login successful", name);
		return (1);
	}
	NS_Log(NS_INFO, "%s: password mismatch", name);
fail:
	AG_SetError("Unknown username or password mismatch");
	return (0);
}

static int
auth_pubkey(NS_Server *serv, void *p)
{
	AG_SetError("Public key auth is not implemented");
	return (0);
}

int
main(int argc, char *argv[])
{
	char *host = NULL;
	char *port = "6785";
	char *dir = _PATH_DATA;
	User *u;
	struct sigaction sa;
	int adminflag = 0;
	int i;
	
	if (AG_InitCore("agarrcsd", AG_CREATE_DATADIR) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((i = getopt(argc, argv, "ab:p:d:v?")) != -1) {
		extern char *__progname;

		switch (i) {
		case 'a':
			adminflag = 1;
			break;
		case 'b':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'd':
			dir = optarg;
			break;
		case 'v':
			printf("%s %s\n", __progname, VERSION);
			exit(0);
		case '?':
		default:
			printf("Usage: %s [-av] [-b host] [-p port] [-d dir]\n",
			    __progname);
			exit(0);
		}
	}

	AG_RegisterClass(&UserClass);
	
	AG_ObjectInitStatic(&UserMgr, NULL);

	if (adminflag) {
		if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
			fprintf(stderr, "%s\n", AG_GetError());
			return (-1);
		}
		AG_InitInput(0);
		AG_SetRefreshRate(-1);
		AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
		DEV_InitSubsystem(0);
		DEV_Browser(&UserMgr);
	}

	/* Load previously saved user data. */
	AG_ObjectLoad(&UserMgr);
	AGOBJECT_FOREACH_CLASS(u, &UserMgr, user, "User:*")
		AG_ObjectLoad(u);

	/* Move to the data directory. */
	if (chdir(dir) == -1) {
		fprintf(stderr, "%s: %s\n", dir, strerror(errno));
		exit(1);
	}

	/* Initialize the server. */
	serv = NS_ServerNew(NULL, 0, "_server", _PROTO_NAME, _PROTO_VER, port);
	NS_ServerBind(serv, host, port);

	/* Set up the SIGHUP handler. */
	sa.sa_flags = 0;
	sa.sa_handler = sig_hup;
	sigaction(SIGHUP, &sa, NULL);

	/* Register our auth methods and error functions. */
	NS_RegAuthMode(serv, "password", auth_password, NULL);
	NS_RegAuthMode(serv, "public-key", auth_pubkey, NULL);

	/* Register the daemon functions. */
	NS_RegCmd(serv, "version", version, NULL);
	NS_RegCmd(serv, "rcs-commit", rcs_commit, NULL);
	NS_RegCmd(serv, "rcs-info", rcs_info, NULL);
	NS_RegCmd(serv, "rcs-update", rcs_update, NULL);
	NS_RegCmd(serv, "rcs-list", rcs_list, NULL);
	NS_RegCmd(serv, "rcs-log", rcs_log, NULL);

	if (adminflag) {
		AG_EventLoop();
		goto out;
	}

	/* Server main loop. */
	if (NS_ServerLoop(serv) == -1) {
		fprintf(stderr, "%s.%s: %s\n", host, port, strerror(errno));
		exit(1);
	}
out:
	AG_Destroy();
	return (0);
}
