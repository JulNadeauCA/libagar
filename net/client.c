/*	$Csoft: client.c,v 1.7 2005/09/04 01:57:04 vedge Exp $	*/

/*
 * Copyright (c) 2004-2005 CubeSoft Communications, Inc.
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
#include <pwd.h>
#include <err.h>

#include "net.h"
#include "client.h"

enum {
	RDBUF_INIT =	4096,
	RDBUF_GROW =	1024,
	RDBUF_MAX =	256 * (1024 * 1024),
	REQBUF_MAX =	4096
};

int agnClientWarnOnReconnect = 1;	/* Warn on reconnection */
int agnClientReconnectAttempts = 4;	/* Connection retries */
int agnClientReconnectIval = 2;		/* Interval between retries (secs) */

int
AGC_Write(AGC_Session *client, const char *fmt, ...)
{
	char req[REQBUF_MAX];
	ssize_t wrote, len;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(req, sizeof(req), fmt, ap);
	va_end(ap);

	len = strlen(req);
	wrote = write(client->sock, req, len);
	if (wrote == -1) {
		AGN_SetError("Write error: %s", strerror(errno));
		return (-1);
	} else if (wrote != len) {
		AGN_SetError("Short write");
		return (-1);
	}
	return (0);
}

static int
AGN_ParseItemCount(const char *buf, unsigned int *ip)
{
	char numbuf[13];
	char *endp;
	long lval;

	strlcpy(numbuf, buf, sizeof(numbuf));
	errno = 0;
	lval = strtol(numbuf, &endp, 10);
	if (numbuf[0] == '\0' || endp[0] != '\0') {
		*ip = 0;
		return (0);
	}
	if ((errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN)) ||
	    (lval > INT_MAX || lval < INT_MIN)) {
		AGN_SetError("Item count out of range");
		return (-1);
	}
	*ip = lval;
	return (0);
}

/* Parse an error message that occured during negotiation. */
static char *
AGC_GetServerError(AGC_Session *client)
{
	if (((client->read.buf[0] == '!') || (client->read.buf[0] == '1')) &&
	    client->read.buf[1] == ' ') {
		return (&client->read.buf[2]);
	}
	return (client->read.buf);
}

AGC_Result *
AGC_Query(AGC_Session *client, const char *fmt, ...)
{
	char req[REQBUF_MAX];
	char *bufp;
	AGC_Result *res;
	int i;
	unsigned int count;
	va_list ap;
	char *s;
	size_t totsz = 0, pos;
	
	if (client == NULL) {
		AGN_SetError("Not connected to server.");
		return (NULL);
	}

	va_start(ap, fmt);
	vsnprintf(req, sizeof(req), fmt, ap);
	strlcat(req, "\n", sizeof(req));
	va_end(ap);

sendreq:
	/* Issue the server request. */
	if (AGC_Write(client, "%s\n", req) == -1) {
		return (NULL);
	}
	if (AGC_Read(client, 3) < 1) {
		if (AGC_Reconnect(client) == 0) {
			goto sendreq;
		}
		return (NULL);
	}

	switch (client->read.buf[0]) {
	case '0':
		break;
	case '!':
	case '1':
		AGN_SetError("Server error: `%s'", AGC_GetServerError(client));
		return (NULL);
	default:
		AGN_SetError("Illegal server response: `%s'", client->read.buf);
		return (NULL);
	}

	/* Parse the list/item size specification. */
	res = Malloc(sizeof(AGC_Result));
	bufp = &client->read.buf[2];
	for (i = 0; (s = strsep(&bufp, ":")) != NULL; i++) {
		if (s[0] == '\0') {
			break;
		}
		if (AGN_ParseItemCount(s, &count) == -1) {
			if (i == 0) {
				free(res);
			} else {
				for (i = 0; i < res->argc; i++) {
					free(res->argv[i]);
				}
				if (res->argv != NULL) {
					free(res->argv);
					free(res->argv_len);
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
				res->argv = Malloc(count * sizeof(char *));
				res->argv_len = Malloc(count * sizeof(size_t));
			}
		} else {
			res->argv[i-1] = Malloc(count);
			res->argv_len[i-1] = count;
			totsz += count;
		}
	}
		
	/* Read the items. */
	if (AGC_Write(client, "1\n") == -1)
		goto fail;
	
	if (AGC_ReadBinary(client, totsz) < totsz) {
		goto fail;
	}
	for (i = 0, pos = 0; i < res->argc; i++) {
		memcpy(res->argv[i], &client->read.buf[pos], res->argv_len[i]);
		pos += res->argv_len[i];
	}
	if (AGC_Write(client, "0\n") == -1) {
		goto fail;
	}
	return (res);
fail:
	AGC_FreeResult(res);
	return (NULL);
}

AGC_Result *
AGC_QueryBinary(AGC_Session *client, const char *fmt, ...)
{
	char sizbuf[16];
	char req[REQBUF_MAX];
	AGC_Result *res;
	size_t binread = 0, binsize;
	size_t len = 0;
	char *dst;
	ssize_t rv;
	int i;
	va_list ap;

	if (client == NULL) {
		AGN_SetError("Not connected to server.");
		return (NULL);
	}

	/* Issue the request. */
	va_start(ap, fmt);
	vsnprintf(req, sizeof(req), fmt, ap);
	va_end(ap);
sendreq:
	if (AGC_Write(client, "%s\n\n", req) == -1) {
		return (NULL);
	}
	if ((rv = AGC_Read(client, 15)) < 1) {
		if (AGC_Reconnect(client) == 0) {
			goto sendreq;
		}
		return (NULL);
	}
	if (rv < 15) {
		AGN_SetError("Malformed binary size packet.");
		return (NULL);
	}
	if (client->read.buf[0] != '0') {
		AGN_SetError("Bad request: %s", client->read.buf);
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
	res = Malloc(sizeof(AGC_Result));
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
			AGN_SetError("Read error: %s", strerror(errno));
			goto fail;
		} else if (rv == 0) {
			break;
		}
		binread += rv;
		dst += rv;
	}
	if (binread < binsize) {
		AGN_SetError("Incomplete binary response.");
		goto fail;
	}
	printf("downloaded %lu bytes\n", (u_long)binread);
	return (res);
fail:
	free(res->argv[0]);
	free(res->argv_len);
	free(res->argv);
	free(res);
	return (NULL);
}

void
AGC_FreeResult(AGC_Result *res)
{
	int i;

	if (res->argv != NULL) {
		for (i = 0; i < res->argc; i++) {
			free(res->argv[i]);
		}
		free(res->argv);
	}
	if (res->argv_len != NULL) {
		free(res->argv_len);
	}
	free(res);
}

/* Destroy the current server connection and establish a new one. */
int
AGC_Reconnect(AGC_Session *client)
{
	char host_save[AGN_HOSTNAME_MAX];
	char port_save[AGN_PORTNUM_MAX];
	char user_save[AGN_USERNAME_MAX];
	char pass_save[AGN_PASSWORD_MAX];
	int try, retries;

	strlcpy(host_save, client->host, sizeof(host_save));
	strlcpy(port_save, client->port, sizeof(port_save));
	strlcpy(user_save, client->user, sizeof(user_save));
	strlcpy(pass_save, client->pass, sizeof(pass_save));

	if (agnClientWarnOnReconnect)
		fprintf(stderr, "%s: reconnecting...\n", AGN_GetError());

	AGC_Disconnect(client);

	for (try = 0, retries = agnClientReconnectAttempts;
	     try < retries;
	     try++) {
		if (AGC_Connect(client, host_save, port_save, user_save,
		    pass_save) == 0) {
			break;
		}
		sleep(agnClientReconnectIval);
	}
	if (try == retries) {
		AGN_SetError("Could not reconnect to server.");
		return (-1);
	}
	return (0);
}

ssize_t
AGC_Read(AGC_Session *client, size_t nbytes)
{
	ssize_t rv;
	size_t i;

	client->read.len = 0;
	for (;;) {
		if (client->read.len+nbytes > client->read.maxlen) {  /* Grow */
			client->read.maxlen += nbytes+RDBUF_GROW;
			if ((RDBUF_MAX > 0) &&
			    (client->read.maxlen > RDBUF_MAX)) {
				AGN_SetError("Illegal server response");
				return (-1);
			}
			client->read.buf = Realloc(client->read.buf,
			    client->read.maxlen);
		}

		rv = read(client->sock, client->read.buf+client->read.len,
		    nbytes);
		if (rv == -1) {
			AGN_SetError("Read error: %s", strerror(errno));
			return (-1);
		} else if (rv == 0) {
			AGN_SetError("EOF from server");
			return (-1);
		}
		/* XXX add a timeout; server aborts may cause infinite loop. */
		for (i = client->read.len; i < client->read.len+rv; i++) {
			if (client->read.buf[i] == '\n') {
				client->read.buf[client->read.len+rv-1] = '\0';
				return ((ssize_t)client->read.len+rv);
			}
		}
		client->read.len += (size_t)rv;
	}

	AGN_SetError("Illegal server response");
	return (-1);
}

ssize_t
AGC_ReadBinary(AGC_Session *client, size_t nbytes)
{
	ssize_t rv;
	size_t i;

	client->read.len = 0;
	for (;;) {
		if (client->read.len+nbytes > client->read.maxlen) {  /* Grow */
			client->read.maxlen += nbytes+RDBUF_GROW;
			if ((RDBUF_MAX > 0) &&
			    (client->read.maxlen > RDBUF_MAX)) {
				AGN_SetError("Illegal server response");
				return (-1);
			}
			client->read.buf = Realloc(client->read.buf,
			    client->read.maxlen);
		}

		rv = read(client->sock, client->read.buf+client->read.len,
		    nbytes);
		if (rv == -1) {
			AGN_SetError("Read error: %s", strerror(errno));
			return (-1);
		} else if (rv == 0) {
			AGN_SetError("EOF from server");
			return (-1);
		}
		client->read.len += (size_t)rv;
		if (client->read.len >= nbytes)
			return (client->read.len);
	}

	AGN_SetError("Illegal server response");
	return (-1);
}

static int
AGC_Auth(AGC_Session *client, const char *user, const char *pass)
{
	if (AGC_Write(client, "password\n") == -1 ||
	    AGC_Read(client, 32) < 1 ||
	    strcmp(client->read.buf, "ok-send-auth") != 0) {
		AGN_SetError("Authentication protocol error: `%s'",
		    client->read.buf);
		return (-1);
	}
	if (AGC_Write(client, "%s:%s\n", user, pass) == -1 ||
	    AGC_Read(client, 32) < 1 ||
	    strcmp(client->read.buf, "ok") != 0) {
		AGN_SetError("Authentication failed");
		return (-1);
	}
	return (0);

}

/* Negotiate the protocol version. */
static int
AGC_Negotiate(AGC_Session *client)
{
	if (AGC_Read(client, 32) < 1) {
		AGN_SetError("Server did not respond");
		return (-1);
	}
	strlcpy(client->server_proto, client->read.buf,
	    sizeof(client->server_proto));

	if (AGC_Write(client, "%s\n", client->client_proto) == -1 ||
	    AGC_Read(client, 32) < 1) {
		AGN_SetError("Server protocol error");
		return (-1);
	}
	if (strncmp(client->read.buf, "auth:", strlen("auth:")) != 0) {
		AGN_SetError("Server version mismatch");
		return (-1);
	}
	return (0);
}

/* Establish a connection to the server and authenticate. */
int
AGC_Connect(AGC_Session *client, const char *host, const char *port,
    const char *user, const char *pass)
{
	char fbuf[1024];
	const char *cause = NULL;
	struct addrinfo hints, *res, *res0;
	struct passwd *pwd;
	int s, rv;
	
	if (host == NULL || port == NULL || user == NULL || pass == NULL) {
		char file[MAXPATHLEN];
		char *s, *fbufp;
		FILE *f;
	
		if ((pwd = getpwuid(getuid())) == NULL) {
			AGN_SetError("Who are you?");
			return (-1);
		}
		strlcpy(file, pwd->pw_dir, sizeof(file));
		strlcat(file, "/.", sizeof(file));
		strlcat(file, client->name, sizeof(file));
		strlcat(file, "rc", sizeof(file));

		if ((f = fopen(file, "r")) == NULL) {
			AGN_SetError("%s: %s", file, strerror(errno));
			return (-1);
		}
		fread(fbuf, sizeof(fbuf), 1, f);
		fclose(f);

		fbufp = fbuf;
		while ((s = strsep(&fbufp, "\n")) != NULL) {
			const char *lv, *rv;

			if (s[0] == '#' )
				continue;
			
			if ((lv = strsep(&s, ":=")) == NULL ||
			    (rv = strsep(&s, ":=")) == NULL)
				continue;
			
			if (strcasecmp(lv, "host") == 0) host = rv;
			if (strcasecmp(lv, "port") == 0) port = rv;
			if (strcasecmp(lv, "user") == 0) user = rv;
			if (strcasecmp(lv, "pass") == 0) pass = rv;
		}
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		AGN_SetError("%s:%s: %s", host, port, gai_strerror(rv));
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
		AGN_SetError("%s: %s", cause, strerror(errno));
		goto fail_resolution;
	}

	client->sock = s;
	strlcpy(client->host, host, sizeof(client->host));
	strlcpy(client->port, port, sizeof(client->port));
	strlcpy(client->user, user, sizeof(client->user));
	strlcpy(client->pass, pass, sizeof(client->pass));

	if (AGC_Negotiate(client) == -1 ||
	    AGC_Auth(client, user, pass) == -1) {
		AGN_SetError("Server error: %s", AGC_GetServerError(client));
		goto fail_close;
	}

	freeaddrinfo(res0);
	return (0);
fail_close:
	AGC_Disconnect(client);
fail_resolution:
	freeaddrinfo(res0);
	return (-1);
}

void
AGC_Disconnect(AGC_Session *client)
{
	if (client->sock != -1) {
		close(client->sock);
		client->sock = -1;
	}
}

void
AGC_Init(AGC_Session *client, const char *name, const char *ver)
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

	strlcpy(client->client_proto, name, sizeof(client->client_proto));
	strlcat(client->client_proto, " ", sizeof(client->client_proto));
	strlcat(client->client_proto, ver, sizeof(client->client_proto));
}

void
AGC_Destroy(AGC_Session *client)
{
	AGC_Disconnect(client);
	free(client->read.buf);
}

