/*	$Csoft: server.c,v 1.6 2005/09/04 01:57:04 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <agar/config/network.h>
#ifdef NETWORK

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
#include <err.h>

#include "net.h"
#include "sockunion.h"
#include "fgetln.h"

#define MAX_SERVER_SOCKS 64

const char *server_name;
const char *server_ver;

const char *server_host = NULL;
const char *server_port = "6571";
pid_t server_pid = 0;

AGN_ServerCmd  *server_cmds = NULL;
unsigned int   nserver_cmds = 0;
AGN_ServerAuth *server_auths = NULL;
unsigned int   nserver_auths = 0;

static void (*errhandler)(void) = NULL;
static void (*cmdcallback)(void) = NULL;
int callback_secs = 1;
int callback_usecs = 0;

static void	**list_items;
static size_t	 *list_itemsz;
static unsigned int nlist_items;

void
AGN_ServerBeginList(void)
{
	list_items = NULL;
	list_itemsz = NULL;
	nlist_items = 0;
}

void
AGN_ServerEndList(void)
{
	char ack[2];
	int i;
	
	printf("0 %010u:", nlist_items);
	for (i = 0; i < nlist_items; i++) {
		printf("%012lu:", (unsigned long)list_itemsz[i]);
	}
	fputc('\n', stdout);
	fgets(ack, 2, stdin);
	
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	for (i = 0; i < nlist_items; i++) {
		void *itembuf = list_items[i];
		size_t itemsz = list_itemsz[i];

		if (fwrite(itembuf, 1, itemsz, stdout) < itemsz) {
#ifdef HAVE_SYSLOG
			syslog(LOG_ERR, "error writing item");
#endif
			exit(1);
		}
	}
	fflush(stdout);
	setvbuf(stdout, NULL, _IOLBF, 0);
	fgets(ack, 2, stdin);

	if (nlist_items > 0) {
		Free(list_items, M_NETBUF);
		Free(list_itemsz, M_NETBUF);
		list_items = NULL;
		list_itemsz = NULL;
		nlist_items = 0;
	}
}

void
AGN_ServerListItem(void *buf, size_t len)
{
	if (nlist_items == 0) {
		list_items = Malloc(sizeof(void *), M_NETBUF);
		list_itemsz = Malloc(sizeof(size_t), M_NETBUF);
	} else {
		list_items = Realloc(list_items,
		    (nlist_items+1)*sizeof(void *));
		list_itemsz = Realloc(list_itemsz,
		    (nlist_items+1)*sizeof(size_t));
	}
	list_items[nlist_items] = buf;
	list_itemsz[nlist_items] = len;

	nlist_items++;
}

void
AGN_ServerListString(const char *fmt, ...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	if (vasprintf(&buf, fmt, ap) == -1) {
		err(1, "vasprintf: out of memory");
	}
	va_end(ap);

	AGN_ServerListItem(buf, strlen(buf)+1);
}

/*
 * Register a generic error handler function to invoke upon any failed
 * request.
 */
void
AGN_ServerSetErrorFn(void (*errh)(void))
{
	errhandler = errh;
}

/* Register a function to execute at every ival secs. */
void
AGN_ServerRegCallback(void (*cb)(void), int secs, int usecs)
{
	cmdcallback = cb;
	callback_secs = secs;
	callback_usecs = usecs;
}

/* Register a server command. */
void
AGN_ServerRegCmd(const char *name, int (*func)(AGN_Command *, void *),
    void *arg)
{
	server_cmds = Realloc(server_cmds, (nserver_cmds+1) *
					   sizeof(AGN_ServerCmd));
	server_cmds[nserver_cmds].name = name;
	server_cmds[nserver_cmds].func = func;
	server_cmds[nserver_cmds].arg = arg;
	nserver_cmds++;
}

/* Register an authentication method. */
void
AGN_ServerRegAuth(const char *name, int (*func)(void *), void *arg)
{
	server_auths = Realloc(server_auths, (nserver_auths+1) *
					     sizeof(AGN_ServerAuth));
	server_auths[nserver_auths].name = name;
	server_auths[nserver_auths].func = func;
	server_auths[nserver_auths].arg = arg;
	nserver_auths++;
}

static void
AGN_ProcessCommand(AGN_Command *ncmd)
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
					printf("1 %s\n", AGN_GetError());
				}
			}
			fflush(stdout);
			return;
		}
	}
	AGN_ServerDie(1, "unimplemented command");
}

/*
 * Negotiate the version, authenticate and loop processing requests from
 * the client.
 */
static void
AGN_QueryLoop(void)
{
	char tmp[BUFSIZ];
	char servproto[32];
	char *buf, *lbuf = NULL, *value, *p;
	AGN_Command ncmd;
	size_t len;
	ssize_t rv;
	int seq = 0;
	int i;

	snprintf(servproto, sizeof(servproto), "%s %s\n", server_name,
	    server_ver);
	fputs(servproto, stdout);
	fflush(stdout);
	
	if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
		goto read_failure;
	}
	if (strcmp(tmp, servproto) != 0) {
		AGN_ServerLog(LOG_ERR, "incompatible version: `%s'!=`%s'", tmp,
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
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "unknown auth mode: `%s'", tmp);
#endif
		fputs("unknown auth mode\n", stdout);
		exit(1);
	}
	fputs("ok-send-auth\n", stdout);
	fflush(stdout);

	if (server_auths[i].func(server_auths[i].arg) == 0) {
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "auth error: %s", AGN_GetError());
#endif
		printf("auth error: %s\n", AGN_GetError());
		exit(1);
	}
	fputs("ok\n", stdout);
	fflush(stdout);

	while ((buf = AGN_Fgetln(stdin, &len)) != NULL) {
		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
		} else {
			if ((lbuf = malloc(len+1)) == NULL) {
				AGN_ServerDie(1, "malloc line");
			}
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}

		if (seq++ == 0) {
			/* Initiate a new request. */
			AGN_InitCommand(&ncmd);
			if (strlcpy(ncmd.name, buf, sizeof(ncmd.name)) >=
			    sizeof(ncmd.name)) {
				AGN_DestroyCommand(&ncmd);
				Free(lbuf, M_NETBUF);
				AGN_ServerDie(1, "command name too big");
			}
		} else if ((value = strchr(buf, '=')) != NULL) {
			/*
			 * Append an argument to the current request.
			 */
			AGN_CommandArg *narg;

			ncmd.args = Realloc(ncmd.args,
			    ++ncmd.nargs * sizeof(AGN_CommandArg));
			narg = &ncmd.args[ncmd.nargs-1];
			narg->value = Strdup(value+1);
			narg->size = sizeof(value);

			*value = '\0';
			if (strlcpy(narg->key, buf, sizeof(narg->key)) >=
			    sizeof(narg->key)) {
				AGN_DestroyCommand(&ncmd);
				Free(lbuf, M_NETBUF);
				AGN_ServerDie(1, "command key is too big");
			}
		} else {
			AGN_ProcessCommand(&ncmd);
			AGN_DestroyCommand(&ncmd);
			seq = 0;
		}

		if (lbuf != NULL) {
			Free(lbuf, M_NETBUF);
			lbuf = NULL;
		}
	}
read_failure:
	if (ferror(stdin)) {
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "read from client: %m");
#endif
		exit(1);
	} else if (feof(stdin)) {
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "EOF from client");
#endif
		exit(1);
	}
	exit(0);
}

void
AGN_ServerDie(int rv, const char *fmt, ...)
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

void
AGN_ServerLog(int level, const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fprintf(stderr, "%s\n", buf);
#ifdef HAVE_SYSLOG
	syslog(level, "%s", buf);
#endif
}

/* Initiate a binary transfer. */
void
AGN_ServerBinaryMode(size_t nbytes)
{
	if (setvbuf(stdout, NULL, _IONBF, 0) == EOF) {
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "_IONBF stdout: %m");
#endif
		exit(1);
	}
	printf("0 %012lu\n", (unsigned long)nbytes);
	fflush(stdout);
}

/* Switch back to command mode. */
void
AGN_ServerCommandMode(void)
{
	if (setvbuf(stdout, NULL, _IOLBF, 0) == EOF) {
#ifdef HAVE_SYSLOG
		syslog(LOG_ERR, "_IOLBF stdout: %m");
#endif
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
cmd_quit(AGN_Command *cmd, void *arg)
{
	AGN_ServerDie(0, "Kongen leve!\n");
	return (0);
}

int
AGN_ServerListen(const char *srvname, const char *srvver, const char *srvhost,
    const char *srvport)
{
	struct addrinfo hints, *res, *res0;
	extern char *__progname;
	const char *cause = NULL;
	int rv, nservsocks, maxfd = 0;
	int i, servsocks[MAX_SERVER_SOCKS];
	fd_set servfds;
	struct sigaction sa;

	AGN_ServerRegCmd("quit", cmd_quit, NULL);

	server_pid = getpid();
	server_name = srvname;
	server_ver = srvver;
	server_host = srvhost;
	server_port = srvport;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(server_host, server_port, &hints, &res0))
	    != 0) {
		AGN_SetError("%s", gai_strerror(rv));
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
#ifdef HAVE_SYSLOG
			syslog(LOG_ERR, "SO_REUSEADDR: %m (ignored)");
#endif
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
		AGN_SetError("%s: %s", cause, strerror(errno));
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

		tv.tv_sec = callback_secs;
		tv.tv_usec = callback_usecs;
		rv = select(maxfd+1, &rfds, NULL, NULL, &tv);
		if (rv == 0 && cmdcallback != NULL) {
			cmdcallback();
		} else if (rv == -1) {
			if (errno == EINTR) {
				continue;
			}
			AGN_SetError("select: %s", strerror(errno));
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
#ifdef HAVE_SYSLOG
					syslog(LOG_ERR,
					    "SO_OOBINLINE: %m (ignored)");
#endif
				}
				if (fcntl(fileno(stdin), F_SETOWN, getpid())
				    == -1) {
#ifdef HAVE_SYSLOG
					syslog(LOG_ERR,
					    "fcntl F_SETOWN: %m (ignored)");
#endif
				}
				if (setvbuf(stdout, NULL, _IOLBF, 0) == EOF) {
#ifdef HAVE_SYSLOG
					syslog(LOG_ERR, "_IOLBF stdout: %m");
#endif
					exit(1);
				}
				AGN_QueryLoop();
			}
			close(rv);
		}
	}
	return (0);
}

#endif /* NETWORK */
