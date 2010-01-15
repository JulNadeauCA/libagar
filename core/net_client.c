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
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

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
#include <signal.h>

#include "net_command.h"
#include "net_client.h"

#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif

enum {
	RDBUF_INIT =	4096,
	RDBUF_GROW =	1024,
	RDBUF_MAX =	256 * (1024 * 1024),
	REQBUF_MAX =	4096
};

int ncWarnOnReconnect = 1;		/* Warn on reconnection */
int ncReconnectAttempts = 4;		/* Connection retries */
int ncReconnectIval = 2;		/* Interval between retries (secs) */

void
NC_InitSubsystem(Uint flags)
{
}

int
NC_Write(NC_Session *client, const char *fmt, ...)
{
	char req[REQBUF_MAX];
	ssize_t wrote, len;
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(req, sizeof(req), fmt, ap);
	va_end(ap);

	len = strlen(req);
	wrote = write(client->sock, req, len);
	if (wrote == -1) {
		AG_SetError("Write error: %s", strerror(errno));
		return (-1);
	} else if (wrote != len) {
		AG_SetError("Short write");
		return (-1);
	}
	return (0);
}

static int
ParseItemCount(const char *buf, unsigned int *ip)
{
	char numbuf[13];
	char *endp;
	long lval;

	Strlcpy(numbuf, buf, sizeof(numbuf));
	errno = 0;
	lval = strtol(numbuf, &endp, 10);
	if (numbuf[0] == '\0' || endp[0] != '\0') {
		*ip = 0;
		return (0);
	}
	if ((errno == ERANGE &&
	    (lval == AG_LONG_MAX || lval == AG_LONG_MIN)) ||
	    (lval > AG_INT_MAX || lval < AG_INT_MIN)) {
		AG_SetError("Item count out of range");
		return (-1);
	}
	*ip = lval;
	return (0);
}

/* Parse an error message that occured during negotiation. */
static char *
GetServerError(NC_Session *client)
{
	if (((client->read.buf[0] == '!') || (client->read.buf[0] == '1')) &&
	    client->read.buf[1] == ' ') {
		return (&client->read.buf[2]);
	}
	return (client->read.buf);
}

/*
 * Send a query to the server and expect an immediate response consisting
 * in an array of binary items.
 */
NC_Result *
NC_Query(NC_Session *client, const char *fmt, ...)
{
	char req[REQBUF_MAX];
	char *bufp;
	NC_Result *res;
	int i;
	unsigned int count;
	va_list ap;
	char *s;
	size_t totsz = 0, pos;
	
	if (client == NULL) {
		AG_SetError("Not connected to server.");
		return (NULL);
	}

	va_start(ap, fmt);
	Vsnprintf(req, sizeof(req), fmt, ap);
	Strlcat(req, "\n", sizeof(req));
	va_end(ap);

sendreq:
	/* Issue the server request. */
	if (NC_Write(client, "%s\n", req) == -1) {
		return (NULL);
	}
	if (NC_Read(client, 3) < 1) {
		if (NC_Reconnect(client) == 0) {
			goto sendreq;
		}
		return (NULL);
	}

	switch (client->read.buf[0]) {
	case '0':
		break;
	case '!':
	case '1':
		AG_SetError("Server error: `%s'", GetServerError(client));
		return (NULL);
	default:
		AG_SetError("Illegal server response: `%s'", client->read.buf);
		return (NULL);
	}

	/* Parse the list/item size specification. */
	res = Malloc(sizeof(NC_Result));
	bufp = &client->read.buf[2];
	for (i = 0; (s = AG_Strsep(&bufp, ":")) != NULL; i++) {
		if (s[0] == '\0') {
			break;
		}
		if (ParseItemCount(s, &count) == -1) {
			if (i == 0) {
				Free(res);
			} else {
				for (i = 0; i < res->argc; i++) {
					Free(res->argv[i]);
				}
				if (res->argv != NULL) {
					Free(res->argv);
					Free(res->argv_len);
				}
			}
			return (NULL);
		}

		if (i == 0) {
			res->argc = count;
			if (count == 0) {
				res->argv = NULL;
				res->argv_len = NULL;
				return (res);
			} else {
				res->argv = Malloc(count*sizeof(char *));
				res->argv_len = Malloc(count*sizeof(size_t));
			}
		} else {
			res->argv[i-1] = Malloc(count);
			res->argv_len[i-1] = count;
			totsz += count;
		}
	}
		
	/* Read the items. */
	if (NC_Write(client, "1\n") == -1)
		goto fail;
	
	if (NC_ReadBinary(client, totsz) < totsz) {
		goto fail;
	}
	for (i = 0, pos = 0; i < res->argc; i++) {
		memcpy(res->argv[i], &client->read.buf[pos], res->argv_len[i]);
		pos += res->argv_len[i];
	}
	if (NC_Write(client, "0\n") == -1) {
		goto fail;
	}
	return (res);
fail:
	NC_FreeResult(res);
	return (NULL);
}

/*
 * Send a query to the server and expect a (possibly slowly delivered)
 * stream of data in response.
 */
NC_Result *
NC_QueryBinary(NC_Session *client, const char *fmt, ...)
{
	char sizbuf[16];
	char req[REQBUF_MAX];
	NC_Result *res;
	size_t binread = 0, binsize;
	char *dst;
	ssize_t rv, i;
	va_list ap;

	if (client == NULL) {
		AG_SetError("Not connected to server.");
		return (NULL);
	}

	/* Issue the request. */
	va_start(ap, fmt);
	Vsnprintf(req, sizeof(req), fmt, ap);
	va_end(ap);
sendreq:
	if (NC_Write(client, "%s\n\n", req) == -1) {
		return (NULL);
	}
	if ((rv = NC_Read(client, 15)) < 1) {
		if (NC_Reconnect(client) == 0) {
			goto sendreq;
		}
		return (NULL);
	}
	if (rv < 15) {
		AG_SetError("Malformed binary size packet.");
		return (NULL);
	}
	if (client->read.buf[0] != '0') {
		AG_SetError("Bad request: %s", client->read.buf);
		return (NULL);
	}

	/* Parse the size specification. */
	for (i = 2; i < rv; i++) {
		char *bufp = &client->read.buf[i];
		char *bsp = &sizbuf[i-2];

		if (*bufp == '\n') {
			*bsp = '\0';
			break;
		}
		*bsp = *bufp;
	}
	binsize = atoi(sizbuf);

	/* Allocate the response structure. */
	res = Malloc(sizeof(NC_Result));
	res->argv = Malloc(sizeof(char *));
	res->argv_len = Malloc(sizeof(size_t));
	res->argc = 1;
	res->argv[0] = dst = Malloc(binsize);
	res->argv_len[0] = binsize;

	/* Read the binary data. */
	while (binread < binsize) {
readbin:
		rv = read(client->sock, dst, binsize);
		if (rv == -1) {
			if (errno == EINTR) {
				goto readbin;
			}
			AG_SetError("Read error: %s", strerror(errno));
			goto fail;
		} else if (rv == 0) {
			break;
		}
		binread += rv;
		dst += rv;
	}
	if (binread < binsize) {
		AG_SetError("Incomplete binary response.");
		goto fail;
	}
	printf("downloaded %lu bytes\n", (unsigned long)binread);
	return (res);
fail:
	Free(res->argv[0]);
	Free(res->argv_len);
	Free(res->argv);
	Free(res);
	return (NULL);
}

void
NC_FreeResult(NC_Result *res)
{
	int i;

	if (res->argv != NULL) {
		for (i = 0; i < res->argc; i++) {
			Free(res->argv[i]);
		}
		Free(res->argv);
	}
	if (res->argv_len != NULL) {
		Free(res->argv_len);
	}
	Free(res);
}

/* Destroy the current server connection and establish a new one. */
int
NC_Reconnect(NC_Session *client)
{
	char host_save[NC_HOSTNAME_MAX];
	char port_save[NC_PORTNUM_MAX];
	char user_save[NC_USERNAME_MAX];
	char pass_save[NC_PASSWORD_MAX];
	int try, retries;

	Strlcpy(host_save, client->host, sizeof(host_save));
	Strlcpy(port_save, client->port, sizeof(port_save));
	Strlcpy(user_save, client->user, sizeof(user_save));
	Strlcpy(pass_save, client->pass, sizeof(pass_save));

	if (ncWarnOnReconnect)
		fprintf(stderr, "%s: reconnecting...\n", AG_GetError());

	NC_Disconnect(client);

	for (try = 0, retries = ncReconnectAttempts;
	     try < retries;
	     try++) {
		if (NC_Connect(client, host_save, port_save, user_save,
		    pass_save) == 0) {
			break;
		}
		sleep(ncReconnectIval);
	}
	if (try == retries) {
		AG_SetError("Could not reconnect to server.");
		return (-1);
	}
	return (0);
}

long
NC_Read(NC_Session *client, size_t nbytes)
{
	ssize_t rv;
	size_t i;

	client->read.len = 0;
	for (;;) {
		if (client->read.len+nbytes > client->read.maxlen) {  /* Grow */
			client->read.maxlen += nbytes+RDBUF_GROW;
			if ((RDBUF_MAX > 0) &&
			    (client->read.maxlen > RDBUF_MAX)) {
				AG_SetError("Illegal server response");
				return (-1);
			}
			client->read.buf = Realloc(client->read.buf,
			    client->read.maxlen);
		}

		rv = read(client->sock, client->read.buf+client->read.len,
		    nbytes);
		if (rv == -1) {
			AG_SetError("Read error: %s", strerror(errno));
			return (-1);
		} else if (rv == 0) {
			AG_SetError("EOF from server");
			return (-1);
		}
		/* XXX add a timeout; server aborts may cause infinite loop. */
		for (i = client->read.len; i < client->read.len+rv; i++) {
			if (client->read.buf[i] == '\n') {
				client->read.buf[client->read.len+rv-1] = '\0';
				return (long)(client->read.len+rv);
			}
		}
		client->read.len += (size_t)rv;
	}

	AG_SetError("Illegal server response");
	return (-1);
}

long
NC_ReadBinary(NC_Session *client, size_t nbytes)
{
	ssize_t rv;

	client->read.len = 0;
	for (;;) {
		if (client->read.len+nbytes > client->read.maxlen) {  /* Grow */
			client->read.maxlen += nbytes+RDBUF_GROW;
			if ((RDBUF_MAX > 0) &&
			    (client->read.maxlen > RDBUF_MAX)) {
				AG_SetError("Illegal server response");
				return (-1);
			}
			client->read.buf = Realloc(client->read.buf,
			    client->read.maxlen);
		}

		rv = read(client->sock, client->read.buf+client->read.len,
		    nbytes);
		if (rv == -1) {
			AG_SetError("Read error: %s", strerror(errno));
			return (-1);
		} else if (rv == 0) {
			AG_SetError("EOF from server");
			return (-1);
		}
		client->read.len += (size_t)rv;
		if (client->read.len >= nbytes)
			return ((long)client->read.len);
	}

	AG_SetError("Illegal server response");
	return (-1);
}

static int
Authenticate(NC_Session *client, const char *user, const char *pass)
{
	if (NC_Write(client, "password\n") == -1 ||
	    NC_Read(client, 32) < 1 ||
	    strcmp(client->read.buf, "ok-send-auth") != 0) {
		AG_SetError("Authentication protocol error: `%s'",
		    client->read.buf);
		return (-1);
	}
	if (NC_Write(client, "%s:%s\n", user, pass) == -1 ||
	    NC_Read(client, 32) < 1 ||
	    strcmp(client->read.buf, "ok") != 0) {
		AG_SetError("Authentication failed");
		return (-1);
	}
	return (0);

}

/* Negotiate the protocol version. */
static int
ProtoNegotiate(NC_Session *client)
{
	if (NC_Read(client, 32) < 1) {
		AG_SetError("Server did not respond");
		return (-1);
	}
	Strlcpy(client->server_proto, client->read.buf,
	    sizeof(client->server_proto));

	if (NC_Write(client, "%s\n", client->client_proto) == -1 ||
	    NC_Read(client, 32) < 1) {
		AG_SetError("Server protocol error");
		return (-1);
	}
	if (strncmp(client->read.buf, "auth:", strlen("auth:")) != 0) {
		AG_SetError("Server version mismatch");
		return (-1);
	}
	return (0);
}

/* Establish a connection to the server and authenticate. */
int
NC_Connect(NC_Session *client, const char *host, const char *port,
    const char *user, const char *pass)
{
	char fbuf[1024];
	const char *cause = NULL;
	struct addrinfo hints, *res, *res0;
	int s, rv;

	/* Look in ~/.<app-name>rc for the login information. */
	if (host == NULL || port == NULL || user == NULL || pass == NULL) {
		char file[AG_PATHNAME_MAX];
		char *s, *fbufp;
		FILE *f;

#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
		{
			struct passwd *pwd;

			if ((pwd = getpwuid(getuid())) == NULL) {
				AG_SetError("Who are you?");
				return (-1);
			}
			Strlcpy(file, pwd->pw_dir, sizeof(file));
			Strlcat(file, "/.", sizeof(file));
		}
#else
		Strlcpy(file, "./", sizeof(file));
#endif
		Strlcat(file, client->name, sizeof(file));
		Strlcat(file, "rc", sizeof(file));

		if ((f = fopen(file, "r")) == NULL) {
			AG_SetError("%s: %s", file, strerror(errno));
			return (-1);
		}
		if (fread(fbuf, sizeof(fbuf), 1, f) < 1) {
			AG_SetError("%s: %s", file, strerror(errno));
			fclose(f);
			return (-1);
		}
		fclose(f);

		fbufp = fbuf;
		while ((s = AG_Strsep(&fbufp, "\n")) != NULL) {
			const char *lv, *rv;

			if (s[0] == '#' )
				continue;
			
			if ((lv = AG_Strsep(&s, ":=")) == NULL ||
			    (rv = AG_Strsep(&s, ":=")) == NULL)
				continue;
			
			if (Strcasecmp(lv, "host") == 0) host = rv;
			if (Strcasecmp(lv, "port") == 0) port = rv;
			if (Strcasecmp(lv, "user") == 0) user = rv;
			if (Strcasecmp(lv, "pass") == 0) pass = rv;
		}
	}

	/* Connect to the server. */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		AG_SetError("%s:%s: %s", host, port, gai_strerror(rv));
		return (-1);
	}
	for (s = -1, res = res0;
	     res != NULL;
	     res = res->ai_next) {
		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (s < 0) {
			cause = "socket";
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
			cause = "connect";
			close(s);
			s = -1;
			continue;
		}
		break;
	}
	if (s == -1) {
		AG_SetError("%s: %s", cause, strerror(errno));
		goto fail_resolution;
	}
	client->sock = s;
	Strlcpy(client->host, host, sizeof(client->host));
	Strlcpy(client->port, port, sizeof(client->port));
	Strlcpy(client->user, user, sizeof(client->user));
	Strlcpy(client->pass, pass, sizeof(client->pass));

	/* Negotiate the protocol version and authenticate. */
	if (ProtoNegotiate(client) == -1 ||
	    Authenticate(client, user, pass) == -1) {
		AG_SetError("Server error: %s", GetServerError(client));
		goto fail_close;
	}

	freeaddrinfo(res0);
	return (0);
fail_close:
	NC_Disconnect(client);
fail_resolution:
	freeaddrinfo(res0);
	return (-1);
}

void
NC_Disconnect(NC_Session *client)
{
	if (client->sock != -1) {
		close(client->sock);
		client->sock = -1;
	}
}

void
NC_Init(NC_Session *client, const char *name, const char *ver)
{
	client->name = name;
	client->host[0] = '\0';
	client->port[0] = '\0';
	client->user[0] = '\0';
	client->pass[0] = '\0';
	client->sock = -1;

	client->read.buf = Malloc(RDBUF_INIT);
	client->read.maxlen = RDBUF_INIT;
	client->read.len = 0;
	client->server_proto[0] = '\0';

	Strlcpy(client->client_proto, name, sizeof(client->client_proto));
	Strlcat(client->client_proto, " ", sizeof(client->client_proto));
	Strlcat(client->client_proto, ver, sizeof(client->client_proto));
}

void
NC_Destroy(NC_Session *client)
{
	NC_Disconnect(client);
	Free(client->read.buf);
}
#endif /* AG_NETWORK */
