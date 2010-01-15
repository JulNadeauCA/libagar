/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/ag_network.h>
#ifdef AG_NETWORK

#include "core.h"

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
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "net_command.h"
#include "net_client.h"
#include "net_server.h"
#include "net_sockunion.h"
#include "net_fgetln.h"

#include <config/have_syslog.h>
#include <config/have_syslog_r.h>
#include <config/have_vsyslog.h>
#include <config/have_vsyslog_r.h>
#if defined(HAVE_SYSLOG)
#include <syslog.h>
const int nsSyslogLevels[] = {
	LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING,
	LOG_ERR, LOG_CRIT, LOG_ALERT, LOG_EMERG
};
#else
const char *nsLogLevelNames[] = {
	"DEBUG", "INFO", "NOTICE", "WARNING",
	"ERR", "CRIT", "ALERT", "EMERG"
};
#endif /* HAVE_SYSLOG */

#include <config/have_setproctitle.h>
#ifdef HAVE_SETPROCTITLE
#define Setproctitle setproctitle
#else
#define Setproctitle(arg...) ((void)0)
#endif

#define MAX_SERVER_SOCKS 64

void
NS_InitSubsystem(Uint flags)
{
	AG_RegisterClass(&nsServerClass);
	AG_RegisterClass(&nsClientClass);
}

/* Server: General-purpose write() in server context. */
int
NS_Write(NS_Server *ns, int fd, const void *data, size_t len)
{
	size_t nwrote;
	ssize_t rv;

	for (nwrote = 0; nwrote < len; ) {
		rv = write(fd, data+nwrote, len-nwrote);
		if (rv == -1) {
			if (errno == EINTR) {
				if (ns->sigCheckFn != NULL) {
					ns->sigCheckFn(ns);
				}
				continue;
			} else {
				AG_SetError("Write error: %s", strerror(errno));
				return (-1);
			}
		} else if (rv == 0) {
			AG_SetError("EOF");
			return (-1);
		}
		nwrote += rv;
	}
	return (0);
}

/* Server: General-purpose read() in server context. */
int
NS_Read(NS_Server *ns, int fd, void *data, size_t len)
{
	size_t nread;
	ssize_t rv;

	for (nread = 0; nread< len; ) {
		rv = read(fd, data+nread, len-nread);
		if (rv == -1) {
			if (errno == EINTR) {
				if (ns->sigCheckFn != NULL) {
					ns->sigCheckFn(ns);
				}
				continue;
			} else {
				AG_SetError("Read error: %s", strerror(errno));
				return (-1);
			}
		} else if (rv == 0) {
			AG_SetError("EOF");
			return (-1);
		}
		nread += rv;
	}
	return (0);
}

/* Create a new server instance. */
NS_Server *
NS_ServerNew(void *parent, Uint flags, const char *name, const char *proto,
    const char *protoVer, const char *port)
{
	NS_Server *ns;

	ns = Malloc(sizeof(NS_Server));
	AG_ObjectInit(ns, &nsServerClass);
	AG_ObjectSetNameS(ns, name);
	ns->flags |= flags;
	NS_ServerSetProtocol(ns, proto, protoVer);
	NS_ServerBind(ns, NULL, port);
	AG_ObjectAttach(parent, ns);
	return (ns);
}

static void
InitServer(void *obj)
{
	NS_Server *ns = obj;

	ns->flags = 0;
	ns->host = NULL;
	ns->port = NULL;
	ns->listenProc = 0;
	ns->cmds = NULL;
	ns->ncmds = 0;
	ns->authModes = 0;
	ns->nAuthModes = 0;
	ns->listItems = NULL;

	ns->errorFn = NULL;
	ns->sigCheckFn = NULL;
	ns->loginFn = NULL;
	ns->logoutFn = NULL;

	TAILQ_INIT(&ns->clients);
}

static void
DestroyServer(void *obj)
{
	NS_Server *ns = obj;

	Free(ns->cmds);
	Free(ns->authModes);
	Free(ns->listItems);
}

/* Server: Set protocol version string to use. */
void
NS_ServerSetProtocol(NS_Server *ns, const char *proto, const char *ver)
{
	ns->protoName = proto;
	ns->protoVer = ver;
}

/* Server: Set the bind() hostname to use. */
void
NS_ServerBind(NS_Server *ns, const char *hostName, const char *portName)
{
	ns->host = hostName;
	ns->port = portName;
}

/* Log an error / informational message from the daemon. */
void
NS_Log(enum ns_log_lvl loglvl, const char *fmt, ...)
{
#if defined(HAVE_VSYSLOG_R) || defined(HAVE_SYSLOG_R)
	struct syslog_data sdata = SYSLOG_DATA_INIT;
#endif
#if !defined(HAVE_VSYSLOG) && defined(HAVE_SYSLOG)
	char msg[256];
#endif
	va_list ap;

	va_start(ap, fmt);
#if defined(HAVE_VSYSLOG_R)
	vsyslog_r(nsSyslogLevels[loglvl], &sdata, fmt, ap);
#elif defined(HAVE_VSYSLOG)
	vsyslog(nsSyslogLevels[loglvl], fmt, ap);
#elif defined(HAVE_SYSLOG_R)
	Vsnprintf(msg, sizeof(msg), fmt, ap);
	syslog_r(nsSyslogLevels[loglvl], &sdata, "%s", msg);
#elif defined(HAVE_SYSLOG)
	Vsnprintf(msg, sizeof(msg), fmt, ap);
	syslog(nsSyslogLevels[loglvl], "%s", msg);
#else
	fprintf(stderr, "[%s] ", nsLogLevelNames[loglvl]);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
#endif
	va_end(ap);
}

/* Child: Initiate a List response. */
void
NS_BeginList(NS_Server *ns)
{
#ifdef AG_DEBUG
	if (ns->listItems != NULL)
		AG_FatalError("Nested NS_BeginList() calls");
#endif
	ns->listItems = NULL;
	ns->listItemSize = NULL;
	ns->listItemCount = 0;
}

/* Child: Send a completed List response. */
void
NS_EndList(NS_Server *ns)
{
	char ack[2];
	char *s;
	int i;
	
	printf("0 %010u:", ns->listItemCount);
	for (i = 0; i < ns->listItemCount; i++) {
		printf("%012lu:", (unsigned long)ns->listItemSize[i]);
	}
	fputc('\n', stdout);
	s = fgets(ack, 2, stdin);
	
	fflush(stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	for (i = 0; i < ns->listItemCount; i++) {
		void *itembuf = ns->listItems[i];
		size_t itemsz = ns->listItemSize[i];

		if (NS_Write(ns, STDOUT_FILENO, itembuf, itemsz) == -1) {
			NS_Log(NS_ERR, "EndList: %s", AG_GetError());
			exit(1);
		}
	}
	fflush(stdout);
	setvbuf(stdout, NULL, _IOLBF, 0);
	s = fgets(ack, 2, stdin);

	if (ns->listItemCount > 0) {
		Free(ns->listItems);
		Free(ns->listItemSize);
		ns->listItemSize = NULL;
		ns->listItemCount = 0;
	}
	ns->listItems = NULL;
}

/* Child: Append a data item to the current List response. */
void
NS_ListItem(NS_Server *ns, void *buf, size_t len)
{
	if (ns->listItemCount == 0) {
		ns->listItems = Malloc(sizeof(void *));
		ns->listItemSize = Malloc(sizeof(size_t));
	} else {
		ns->listItems = Realloc(ns->listItems,
		    (ns->listItemCount+1)*sizeof(void *));
		ns->listItemSize = Realloc(ns->listItemSize,
		    (ns->listItemCount+1)*sizeof(size_t));
	}
	ns->listItems[ns->listItemCount] = buf;
	ns->listItemSize[ns->listItemCount] = len;
	ns->listItemCount++;
}

/* Child: Append a string item to the current List response. */
void
NS_ListString(NS_Server *ns, const char *fmt, ...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&buf, fmt, ap);
	va_end(ap);

	NS_ListItem(ns, buf, strlen(buf)+1);
}

/*
 * Server: Register a "login" callback routine, to be invoked just after
 * successful authentication of a client.
 */
void
NS_RegLoginFn(NS_Server *ns, NS_LoginFn fn)
{
	ns->loginFn = fn;
}

/*
 * Server: Register a "logout" callback routine, to be invoked just after
 * a normal session logout.
 */
void
NS_RegLogoutFn(NS_Server *ns, NS_LogoutFn fn)
{
	ns->logoutFn = fn;
}

/*
 * Server: Register an alternate error handler routine for failed commands.
 * The default routine simply returns an error message (code=1) to the client.
 */
void
NS_RegErrorFn(NS_Server *ns, NS_ErrorFn fn)
{
	ns->errorFn = fn;
}

/*
 * Server: Register a signal check function, to be invoked whenever the
 * process should check for pending signals (e.g., after an EINTR return
 * from a system call).
 */
void
NS_RegSigCheckFn(NS_Server *ns, NS_SigCheckFn fn)
{
	ns->sigCheckFn = fn;
}

/* Register a server command. */
void
NS_RegCmd(NS_Server *ns, const char *name, NS_CommandFn fn, void *arg)
{
	ns->cmds = Realloc(ns->cmds, (ns->ncmds+1)*sizeof(NS_Cmd));
	ns->cmds[ns->ncmds].name = name;
	ns->cmds[ns->ncmds].fn = fn;
	ns->cmds[ns->ncmds].arg = arg;
	ns->ncmds++;
	Debug(ns, "Registered function: %s (%p)", name, arg);
}

/*
 * Register an authentication method. The callback routine is expected
 * to return 0 on success. On error, it should return -1 and set an
 * error message.
 */
void
NS_RegAuthMode(NS_Server *ns, const char *name, NS_AuthFn fn, void *arg)
{
	ns->authModes = Realloc(ns->authModes, (ns->nAuthModes+1) *
					         sizeof(NS_Auth));
	ns->authModes[ns->nAuthModes].name = name;
	ns->authModes[ns->nAuthModes].fn = fn;
	ns->authModes[ns->nAuthModes].arg = arg;
	ns->nAuthModes++;
	Debug(ns, "Registered authmode: %s (%p)", name, arg);
}

static void
ProcessCommand(NS_Server *ns, NS_Command *ncmd)
{
	int i;

	for (i = 0; i < ns->ncmds; i++) {
		if (strcmp(ns->cmds[i].name, ncmd->name) != 0) {
			continue;
		}
		if (ns->cmds[i].fn(ns, ncmd, ns->cmds[i].arg) == -1) {
			if (ns->errorFn == NULL ||
			    ns->errorFn(ns) == -1) {
				NS_Message(ns, 1, "%s", AG_GetError());
			}
		}
		fflush(stdout);
		return;
	}
	NS_Logout(ns, 1, "unimplemented command");
}

/*
 * Child: Main routine - Authenticate and loop processing requests
 * from the client.
 */
static void
ChildQueryLoop(NS_Server *ns)
{
	char tmp[AG_BUFFER_MAX];
	char prot[32];
	char *buf, *lbuf = NULL, *value;
	NS_Command ncmd;
	size_t len;
	int seq = 0;
	int i;

	Snprintf(prot, sizeof(prot), "%s %s\n", ns->protoName, ns->protoVer);
	fputs(prot, stdout);
	fflush(stdout);
	
	if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
		goto read_failure;
	}
	if (strcmp(tmp, prot) != 0) {
		NS_Log(NS_ERR, "incompatible version: `%s'!=`%s'", tmp, prot);
		fputs("Incompatible client version\n", stdout);
		exit(1);
	}

	Strlcpy(tmp, "auth:", sizeof(tmp));
	for (i = 0; i < ns->nAuthModes; i++) {
		Strlcat(tmp, ns->authModes[i].name, sizeof(tmp));
		if (i < ns->nAuthModes)
			Strlcat(tmp, ",", sizeof(tmp));
	}
	Strlcat(tmp, "\n", sizeof(tmp));
	fputs(tmp, stdout);

	if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
		goto read_failure;
	}
	for (i = 0; i < ns->nAuthModes; i++) {
		if (strncmp(tmp, ns->authModes[i].name,
		    strlen(ns->authModes[i].name)-1) == 0)
			break;
	}
	if (i == ns->nAuthModes) {
		NS_Log(NS_NOTICE, "Bad auth mode: `%s'", tmp);
		fputs("! Bad auth mode\n", stdout);
		exit(1);
	}
	fputs("ok-send-auth\n", stdout);
	fflush(stdout);

	if (ns->authModes[i].fn(ns, ns->authModes[i].arg) == 0) {
		NS_Log(NS_ERR, "Auth failed: %s", AG_GetError());
		NS_Message(ns, 1, "Auth failed: %s", AG_GetError());
		exit(1);
	}
	if (ns->loginFn != NULL) {
		if (ns->loginFn(ns, NULL) == -1) {
			NS_Log(NS_ERR, "Login failed: %s", AG_GetError());
			NS_Message(ns, 1, "Login failed: %s", AG_GetError());
			exit(1);
		}
	}
	
	fputs("ok\n", stdout);
	fflush(stdout);

	while ((buf = NS_Fgetln(stdin, &len)) != NULL) {
		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
		} else {
			if ((lbuf = TryMalloc(len+1)) == NULL) {
				NS_Logout(ns, 1, "malloc line");
			}
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}

		if (seq++ == 0) {
			/* Initiate a new request. */
			NS_InitCommand(&ncmd);
			if (Strlcpy(ncmd.name, buf, sizeof(ncmd.name)) >=
			    sizeof(ncmd.name)) {
				NS_DestroyCommand(&ncmd);
				Free(lbuf);
				NS_Logout(ns, 1, "command name too big");
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
			if (Strlcpy(narg->key, buf, sizeof(narg->key)) >=
			    sizeof(narg->key)) {
				NS_DestroyCommand(&ncmd);
				Free(lbuf);
				NS_Logout(ns, 1, "command key is too big");
			}
		} else {
			ProcessCommand(ns, &ncmd);
			NS_DestroyCommand(&ncmd);
			seq = 0;
		}

		if (lbuf != NULL) {
			Free(lbuf);
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

/* Child: Terminate the current session (daemon child context). */
void
NS_Logout(NS_Server *ns, int rv, const char *fmt, ...)
{
	char buf[AG_BUFFER_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	Vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	NS_Message(ns, 1, "%s", buf);
	
	if (ns->logoutFn != NULL) {
		ns->logoutFn(ns, NULL);
	}
	exit(rv);
}

/* Child: Write a numerical status code to the client. */
void
NS_Message(NS_Server *ns, int rv, const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	printf("%d %s\n", rv, s);
	fflush(stdout);
	Free(s);
}

/* Initiate a binary transfer. */
void
NS_BeginData(NS_Server *ns, size_t nbytes)
{
	if (setvbuf(stdout, NULL, _IONBF, 0) == EOF) {
		NS_Log(NS_ERR, "_IONBF stdout: %s", strerror(errno));
		exit(1);
	}
	NS_Message(ns, 0, "%012lu", (unsigned long)nbytes);
	fflush(stdout);
}

/* Send a chunk of binary data. */
size_t
NS_Data(NS_Server *ns, char *buf, size_t len)
{
	return NS_Write(ns, STDOUT_FILENO, buf, len);
}

/* Terminate binary transfer. */
void
NS_EndData(NS_Server *ns)
{
	fflush(stdout);
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
#if defined(HAVE_SYSLOG_R)
	struct syslog_data sdata = SYSLOG_DATA_INIT;
	syslog_r(LOG_ERR, &sdata, "client got signal %d", sigraised);
#elif defined(HAVE_SYSLOG)
	syslog(LOG_ERR, "client got signal %d", sigraised);
#endif
	_exit(0);
}

static void
sig_pipe(int sigraised)
{
#if defined(HAVE_SYSLOG_R)
	struct syslog_data sdata = SYSLOG_DATA_INIT;
	syslog_r(LOG_DEBUG, &sdata, "client lost connection");
#elif defined(HAVE_SYSLOG)
	syslog(LOG_DEBUG, "client lost connection");
#endif
	_exit(0);
}

static void
sig_die(int sigraised)
{
#if defined(HAVE_SYSLOG_R)
	struct syslog_data sdata = SYSLOG_DATA_INIT;
	syslog_r(LOG_DEBUG, &sdata, "server got signal %d", sigraised);
#elif defined(HAVE_SYSLOG)
	syslog(LOG_DEBUG, "server got signal %d", sigraised);
#endif
	kill(0, SIGUSR1);
	_exit(0);
}

static int
cmd_quit(NS_Server *ns, NS_Command *cmd, void *arg)
{
	NS_Logout(ns, 0, "logout");
	return (0);
}

/* Main loop of the listening process. */
int
NS_ServerLoop(NS_Server *ns)
{
	struct addrinfo hints, *res, *res0;
	const char *cause = NULL;
	int rv, nservsocks, maxfd = 0;
	int i, servsocks[MAX_SERVER_SOCKS];
	fd_set servfds;
	struct sigaction sa;

	NS_RegCmd(ns, "quit", cmd_quit, NULL);

	ns->listenProc = getpid();

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(ns->host, ns->port, &hints, &res0)) != 0) {
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

		rv = select(maxfd+1, &rfds, NULL, NULL, NULL);
		if (rv == -1) {
			if (errno == EINTR) {
				if (ns->sigCheckFn != NULL) {
					ns->sigCheckFn(ns);
				}
				continue;
			}
			AG_SetError("select: %s", strerror(errno));
			return (-1);
		}
		for (i = 0; i < nservsocks; i++) {
			union sockunion paddr;
			socklen_t paddrlen = sizeof(paddr);
			pid_t pid;

			if (!FD_ISSET(servsocks[i], &rfds))
				continue;

			rv = accept(servsocks[i], (struct sockaddr *)&paddr,
			    &paddrlen);
			if (rv == -1) {
				NS_Log(NS_ERR, "serv socket accept(): %s", strerror(errno));
				continue;
			}
			if ((pid = fork()) == -1) {
				NS_Log(NS_ERR, "serv fork: %s", strerror(errno));
				close(rv);
				continue;
			} else if (pid != 0) {
				close(rv);
				continue;
			}

			/* In child process */

			Setproctitle("%s", ns->protoName);
#ifdef HAVE_SYSLOG
			openlog(ns->protoName, LOG_PID, LOG_LOCAL0);
#endif
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
				NS_Log(NS_ERR, "SO_OOBINLINE: %s (ignored)",
				    strerror(errno));
			}
			if (fcntl(fileno(stdin), F_SETOWN, getpid()) == -1) {
				NS_Log(NS_ERR, "fcntl F_SETOWN: %s (ignored)",
				    strerror(errno));
			}
			if (setvbuf(stdout, NULL, _IOLBF, 0) == EOF) {
				NS_Log(NS_ERR, "_IOLBF stdout: %s",
				    strerror(errno));
				exit(1);
			}
			ChildQueryLoop(ns);
		}
	}
	return (0);
}

static void
InitClient(void *obj)
{
	NS_Client *cl = obj;

	cl->host[0] = '\0';
}

AG_ObjectClass nsServerClass = {
	"NS_Server",
	sizeof(NS_Server),
	{ 0,0 },
	InitServer,
	NULL,			/* free */
	DestroyServer,
	NULL,			/* load */
	NULL,			/* save */
	NULL			/* edit */
};

AG_ObjectClass nsClientClass = {
	"NS_Client",
	sizeof(NS_Client),
	{ 0,0 },
	InitClient,
	NULL,			/* free */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL			/* edit */
};
#endif /* AG_NETWORK */
