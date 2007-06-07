/*	$Csoft: server.c,v 1.6 2005/09/04 01:57:04 vedge Exp $	*/

/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://www.hypertriton.com/>
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

#include <agar/config/server.h>
#ifdef SERVER

#include <core/core.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "net.h"
#include "sockunion.h"
#include "fgetln.h"

#define MAX_SERVER_SOCKS 64

const char *nsServerName;		/* Server protocol string */
const char *nsServerVer;		/* Server version string */
const char *nsServerHost = NULL;	/* Bind hostname */
const char *nsServerPort = "6571";	/* Port number/name */
pid_t nsPID = 0;			/* PID of listening process */

NS_Cmd       *server_cmds = NULL;	/* Implemented commands */
unsigned int nserver_cmds = 0;
NS_Auth      *server_auths = NULL;	/* Authentication methods */
unsigned int nserver_auths = 0;

static void (*errhandler)(void) = NULL;
static void (*cmdcallback)(void) = NULL;
int nsCallbackSecs = 1;
int nsCallbackUsecs = 0;

static void       **nsListItems;
static size_t      *nsListItemSize;
static unsigned int nsListItemCount;

#if defined(HAVE_SYSLOG) || defined(HAVE_VSYSLOG)
const int nsSyslogLevels[] = {
	LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING,
	LOG_ERR, LOG_CRIT, LOG_ALERT, LOG_EMERG
};
#else
const char *nsLogLevelNames[] = {
	"DEBUG", "INFO", "NOTICE", "WARNING",
	"ERR", "CRIT", "ALERT", "EMERG"
};
#endif

void
NS_Log(enum ns_log_lvl loglvl, const char *fmt, ...)
{
	char msg[128];
	va_list ap;

	va_start(ap, fmt);
#ifdef HAVE_VSYSLOG
	vsyslog(nsSyslogLevels[loglvl], fmt, ap)
#else
# ifdef HAVE_SYSLOG
	syslog(nsSyslogLevels[loglvl], "%s", msg);
# else
	fprintf(stderr, "[%s] ", nsLogLevelNames[loglvl]);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
# endif
#endif
	va_end(ap);
}

void
NS_BeginList(void)
{
	nsListItems = NULL;
	nsListItemSize = NULL;
	nsListItemCount = 0;
}

void
NS_EndList(void)
{
	char ack[2];
	int i;
	
	printf("0 %010u:", nsListItemCount);
	for (i = 0; i < nsListItemCount; i++) {
		printf("%012lu:", (unsigned long)nsListItemSize[i]);
	}
	fputc('\n', stdout);
	fgets(ack, 2, stdin);
	
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	for (i = 0; i < nsListItemCount; i++) {
		void *itembuf = nsListItems[i];
		size_t itemsz = nsListItemSize[i];

		if (fwrite(itembuf, 1, itemsz, stdout) < itemsz) {
			NS_Log(NS_ERR, "EndList: Write error");
			exit(1);
		}
	}
	fflush(stdout);
	setvbuf(stdout, NULL, _IOLBF, 0);
	fgets(ack, 2, stdin);

	if (nsListItemCount > 0) {
		Free(nsListItems, M_NETBUF);
		Free(nsListItemSize, M_NETBUF);
		nsListItems = NULL;
		nsListItemSize = NULL;
		nsListItemCount = 0;
	}
}

void
NS_ListItem(void *buf, size_t len)
{
	if (nsListItemCount == 0) {
		nsListItems = Malloc(sizeof(void *), M_NETBUF);
		nsListItemSize = Malloc(sizeof(size_t), M_NETBUF);
	} else {
		nsListItems = Realloc(nsListItems,
		    (nsListItemCount+1)*sizeof(void *));
		nsListItemSize = Realloc(nsListItemSize,
		    (nsListItemCount+1)*sizeof(size_t));
	}
	nsListItems[nsListItemCount] = buf;
	nsListItemSize[nsListItemCount] = len;
	nsListItemCount++;
}

void
NS_ListString(const char *fmt, ...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	if (vasprintf(&buf, fmt, ap) == -1) {
		AG_FatalError("vasprintf: Out of memory");
	}
	va_end(ap);

	NS_ListItem(buf, strlen(buf)+1);
}

/*
 * Register a generic error handler function to invoke upon any failed
 * request.
 */
void
NS_SetErrorFn(void (*errh)(void))
{
	errhandler = errh;
}

/* Register a function to execute at every ival secs. */
void
NS_RegCallback(void (*cb)(void), int secs, int usecs)
{
	cmdcallback = cb;
	nsCallbackSecs = secs;
	nsCallbackUsecs= usecs;
}

/* Register a server command. */
void
NS_RegCmd(const char *name, int (*func)(NS_Command *, void *),
    void *arg)
{
	server_cmds = Realloc(server_cmds, (nserver_cmds+1) *
					   sizeof(NS_Cmd));
	server_cmds[nserver_cmds].name = name;
	server_cmds[nserver_cmds].func = func;
	server_cmds[nserver_cmds].arg = arg;
	nserver_cmds++;
}

/* Register an authentication method. */
void
NS_RegAuth(const char *name, int (*func)(void *), void *arg)
{
	server_auths = Realloc(server_auths, (nserver_auths+1) *
					     sizeof(NS_Auth));
	server_auths[nserver_auths].name = name;
	server_auths[nserver_auths].func = func;
	server_auths[nserver_auths].arg = arg;
	nserver_auths++;
}

static void
NS_ProcessCommand(NS_Command *ncmd)
{
	extern char *__progname;
	int i;

	for (i = 0; i < nserver_cmds; i++) {
		if (strcmp(server_cmds[i].name, ncmd->name) == 0) {
			if (server_cmds[i].func(ncmd, server_cmds[i].arg)
			    == -1) {
				if (errhandler != NULL) {
					errhandler();
				} else {
					printf("1 %s\n", AG_GetError());
				}
			}
			fflush(stdout);
			return;
		}
	}
	NS_Die(1, "unimplemented command");
}

/*
 * Negotiate the version, authenticate and loop processing requests from
 * the client.
 */
static void
NS_QueryLoop(void)
{
	char tmp[BUFSIZ];
	char servproto[32];
	char *buf, *lbuf = NULL, *value, *p;
	NS_Command ncmd;
	size_t len;
	ssize_t rv;
	int seq = 0;
	int i;

	snprintf(servproto, sizeof(servproto), "%s %s\n",
	    nsServerName, nsServerVer);
	fputs(servproto, stdout);
	fflush(stdout);
	
	if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
		goto read_failure;
	}
	if (strcmp(tmp, servproto) != 0) {
		NS_Log(NS_ERR, "incompatible version: `%s'!=`%s'", tmp,
		    servproto);
		fputs("Incompatible client version\n", stdout);
		exit(1);
	}

	strlcpy(tmp, "auth:", sizeof(tmp));
	for (i = 0; i < nserver_auths; i++) {
		strlcat(tmp, server_auths[i].name, sizeof(tmp));
		if (i < nserver_auths)
			strlcat(tmp, ",", sizeof(tmp));
	}
	strlcat(tmp, "\n", sizeof(tmp));
	fputs(tmp, stdout);

	if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
		goto read_failure;
	}
	for (i = 0; i < nserver_auths; i++) {
		if (strncmp(tmp, server_auths[i].name,
		    strlen(server_auths[i].name)-1) == 0)
			break;
	}
	if (i == nserver_auths) {
		NS_Log(NS_NOTICE, "Bad auth mode: `%s'", tmp);
		exit(1);
	}
	fputs("ok-send-auth\n", stdout);
	fflush(stdout);

	if (server_auths[i].func(server_auths[i].arg) == 0) {
		NS_Log(NS_ERR, "Auth failed: %s", AG_GetError());
		exit(1);
	}
	fputs("ok\n", stdout);
	fflush(stdout);

	while ((buf = NS_Fgetln(stdin, &len)) != NULL) {
		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
		} else {
			if ((lbuf = malloc(len+1)) == NULL) {
				NS_Die(1, "malloc line");
			}
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}

		if (seq++ == 0) {
			/* Initiate a new request. */
			NS_InitCommand(&ncmd);
			if (strlcpy(ncmd.name, buf, sizeof(ncmd.name)) >=
			    sizeof(ncmd.name)) {
				NS_DestroyCommand(&ncmd);
				Free(lbuf, M_NETBUF);
				NS_Die(1, "command name too big");
			}
		} else if ((value = strchr(buf, '=')) != NULL) {
			/*
			 * Append an argument to the current request.
			 */
			NS_CommandArg *narg;

			ncmd.args = Realloc(ncmd.args,
			    ++ncmd.nargs * sizeof(NS_CommandArg));
			narg = &ncmd.args[ncmd.nargs-1];
			narg->value = Strdup(value+1);
			narg->size = sizeof(value);

			*value = '\0';
			if (strlcpy(narg->key, buf, sizeof(narg->key)) >=
			    sizeof(narg->key)) {
				NS_DestroyCommand(&ncmd);
				Free(lbuf, M_NETBUF);
				NS_Die(1, "command key is too big");
			}
		} else {
			NS_ProcessCommand(&ncmd);
			NS_DestroyCommand(&ncmd);
			seq = 0;
		}

		if (lbuf != NULL) {
			Free(lbuf, M_NETBUF);
			lbuf = NULL;
		}
	}
read_failure:
	if (ferror(stdin)) {
		NS_Log(NS_ERR, "Client read: %s", strerror(errno));
		exit(1);
	} else if (feof(stdin)) {
		NS_Log(NS_ERR, "EOF from client");
		exit(1);
	}
	exit(0);
}

void
NS_Die(int rv, const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printf("! %s\n", buf);
	fflush(stdout);
	exit(rv);
}

/* Initiate a binary transfer. */
void
NS_BinaryMode(size_t nbytes)
{
	if (setvbuf(stdout, NULL, _IONBF, 0) == EOF) {
		NS_Log(NS_ERR, "_IONBF stdout: %s", strerror(errno));
		exit(1);
	}
	printf("0 %012lu\n", (unsigned long)nbytes);
	fflush(stdout);
}

/* Switch back to command mode. */
void
NS_CommandMode(void)
{
	if (setvbuf(stdout, NULL, _IOLBF, 0) == EOF) {
		NS_Log(NS_ERR, "_IOLBF stdout: %s", strerror(errno));
		exit(1);
	}
}

static void
sig_chld(int sigraised)
{
	int save_errno = errno;
	int rv;

	do {
		rv = waitpid(-1, NULL, WNOHANG);
	} while (rv > 0 || (rv == -1 && errno == EINTR));

	errno = save_errno;
}

static void
sig_urg(int sigraised)
{
	fprintf(stderr, "urgent condition on socket\n");
}

static void
sig_quit(int sigraised)
{
#ifdef HAVE_SYSLOG
	struct syslog_data sdata = SYSLOG_DATA_INIT;

	syslog_r(LOG_ERR, &sdata, "client got signal %s",
	    sys_signame[sigraised]);
#endif
	_exit(0);
}

static void
sig_pipe(int sigraised)
{
#ifdef HAVE_SYSLOG
	struct syslog_data sdata = SYSLOG_DATA_INIT;

	syslog_r(LOG_DEBUG, &sdata, "client lost connection");
#endif
	_exit(0);
}

static void
sig_die(int sigraised)
{
#ifdef HAVE_SYSLOG
	struct syslog_data sdata = SYSLOG_DATA_INIT;

	syslog_r(LOG_DEBUG, &sdata, "server got signal %s",
	    sys_signame[sigraised]);
#endif
	kill(0, SIGUSR1);
	_exit(0);
}

static int
cmd_quit(NS_Command *cmd, void *arg)
{
	NS_Die(0, "Kongen leve!\n");
	return (0);
}

/* Main loop of the listening process. */
int
NS_Listen(const char *srvname, const char *srvver, const char *srvhost,
    const char *srvport)
{
	struct addrinfo hints, *res, *res0;
	extern char *__progname;
	const char *cause = NULL;
	int rv, nservsocks, maxfd = 0;
	int i, servsocks[MAX_SERVER_SOCKS];
	fd_set servfds;
	struct sigaction sa;

	NS_RegCmd("quit", cmd_quit, NULL);

	nsPID = getpid();
	nsServerName = srvname;
	nsServerVer = srvver;
	nsServerHost = srvhost;
	nsServerPort = srvport;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(nsServerHost, nsServerPort, &hints, &res0))
	    != 0) {
		AG_SetError("%s", gai_strerror(rv));
		return (-1);
	}

	FD_ZERO(&servfds);
	for (nservsocks = 0, res = res0;
	     res != NULL && nservsocks < MAX_SERVER_SOCKS;
	     res = res->ai_next) {
		rv = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (rv == -1) {
			cause = "socket";
			continue;
		}

		i = 1;
		if (setsockopt(rv, SOL_SOCKET, SO_REUSEADDR, &i,
		    (socklen_t)sizeof(i)) == -1) {
			NS_Log(NS_ERR, "SO_REUSEADDR: %s (ignored)",
			    strerror(errno));
		}

		if (bind(rv, res->ai_addr, res->ai_addrlen) == -1) {
			cause = "bind";
			close(rv);
			continue;
		}
		if (listen(rv, 5) == -1) {
			cause = "listen";
			close(rv);
			continue;
		}
		servsocks[nservsocks++] = rv;
		FD_SET(rv, &servfds);
		if (rv> maxfd)
			maxfd = rv;
	}
	if (nservsocks == 0) {
		AG_SetError("%s: %s", cause, strerror(errno));
		return (-1);
	}
	freeaddrinfo(res0);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sig_chld;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_flags = 0;
	sa.sa_handler = sig_die;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR1, &sa, NULL);

	for (;;) {
		fd_set rfds = servfds;
		struct timeval tv;

		tv.tv_sec = nsCallbackSecs;
		tv.tv_usec = nsCallbackUsecs;
		rv = select(maxfd+1, &rfds, NULL, NULL, &tv);
		if (rv == 0 && cmdcallback != NULL) {
			cmdcallback();
		} else if (rv == -1) {
			if (errno == EINTR) {
				continue;
			}
			AG_SetError("select: %s", strerror(errno));
			return (-1);
		}

		for (i = 0; i < nservsocks; i++) {
			union sockunion paddr;
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
			size_t paddrlen = sizeof(paddr);
#else
			int paddrlen = sizeof(paddr);
#endif
			pid_t pid;

			if (!FD_ISSET(servsocks[i], &rfds))
				continue;

			rv = accept(servsocks[i], (struct sockaddr *)&paddr,
			    &paddrlen);
			if (rv == -1) {
				fprintf(stderr, "accept: %s\n",
				    strerror(errno));
				continue;
			}
			if ((pid = fork()) == 0) {	/* Child */
				dup2(rv, 0);
				dup2(rv, 1);

				for (i = 0; i < nservsocks; i++) {
					close(servsocks[i]);
				}
				sa.sa_handler = SIG_DFL;
				sigaction(SIGCHLD, &sa, NULL);

				sa.sa_handler = sig_urg;	/* OOB */
				sa.sa_flags = 0;
				sigaction(SIGURG, &sa, NULL);

				sa.sa_handler = sig_pipe;	/* Lost conn. */
				sigaction(SIGPIPE, &sa, NULL);

				sigfillset(&sa.sa_mask);	/* Fatal */
				sa.sa_flags = SA_RESTART;
				sa.sa_handler = sig_quit;
				sigaction(SIGHUP, &sa, NULL);
				sigaction(SIGINT, &sa, NULL);
				sigaction(SIGQUIT, &sa, NULL);
				sigaction(SIGTERM, &sa, NULL);
				sigaction(SIGUSR1, &sa, NULL);

				i = 1;
				if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, &i,
				    sizeof(i)) == -1) {
					NS_Log(NS_ERR,
					    "SO_OOBINLINE: %s (ignored)",
					    strerror(errno));
				}
				if (fcntl(fileno(stdin), F_SETOWN, getpid())
				    == -1) {
					NS_Log(NS_ERR,
					    "fcntl F_SETOWN: %s (ignored)",
					    strerror(errno));
				}
				if (setvbuf(stdout, NULL, _IOLBF, 0) == EOF) {
					NS_Log(NS_ERR, "_IOLBF stdout: %s",
					    strerror(errno));
					exit(1);
				}
				NS_QueryLoop();
			}
			close(rv);
		}
	}
	return (0);
}

#endif /* SERVER */
