/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Agar HTTP application server interface. This provides a multiprocess
 * environment where isolated Worker processes can handle HTTP queries
 * concurrently behind a cluster of Frontend processes.
 *
 * Frontend processes work as HTTP/1.1 servers. They handle authentication,
 * user registration and other (and sessionless operations). They are also
 * responsible for managing the pool of Worker processes and forwarding HTTP
 * queries to them.
 *
 * Worker processes are usually associated with user sessions (which can be
 * limited according to site policies). Since Frontend and Worker processes
 * communicate over managed Unix sockets, Worker processes can run chrooted,
 * or under restricted system modes (such as OpenBSD's pledge(2)). Only the
 * session and event sockets need to be accessible by Workers.
 *
 * This API also provides features to facilitate web development:
 *
 * - Argument parsing and validation. URL-encoded query strings, JSON and
 *   multipart/form-data forms are handled implicitely. Parameters can be
 *   accessed by the WEB_Get*() family of functions, which provide basic
 *   validation for common data types. Eliminating the need for Base64, forms
 *   in multipart/form-data may contain (potentially large) binary blobs,
 *   and this is correctly handled by our parser.
 *
 * - Compression. Using zlib, the Worker process conditionally performs
 *   compression (and chunking) of data on its end.
 *
 * - Template engine. Parameters are set with Set() and Cat(). WEB_OutputHTML()
 *   and WEB_PutJSON_HTML() will return the document with substitutions applied.
 *
 * - Modules, to organize the process of mapping HTTP requests to the
 *   application's routines. Frontend processes will not serve any files. The
 *   framework has no concept of a filesystem (though file access can still
 *   be implemented by a Module).
 */ 

/*
 * Frontends can also become providers of Push events (text/event-stream)
 * for authenticated users. For this to work well, twice as many Frontend
 * instances as number of expected concurrent users should be allocated,
 * since requests may generate events to be delivered back to the client:
 *
 * [Cluster]
 * 	[Frontend 1, BUSY] ---> POST ---> [Worker Process] --+
 *	[Frontend 2, IDLE]                                   |
 *	[Frontend 3, EVENT] <---- (text/event-stream) <------+
 */

#include <agar/core/core.h>
#include <agar/net/web.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <sysexits.h>
#include <dirent.h>

#include <agar/config/enable_nls.h>
#include <agar/config/version.h>
#include <agar/config/have_zlib.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

char webLogFile[FILENAME_MAX];			/* Logfile path */

WEB_Module   **webModules = NULL;		/* Module instances */
Uint           webModuleCount = 0;	
char         **webLangs = NULL;			/* Supported language list */
Uint           webLangCount = 0;
WEB_LanguageFn webLanguageFn = NULL;		/* Language switch callback */
void          *webLanguageFnArg = NULL;		/* webLanguageFn() argument */
char           webHomeOp[WEB_OPNAME_MAX];	/* Default operation */
WEB_MenuFn     webMenuFn = NULL;		/* Menu constructor routine */
void          *webMenuFnArg = NULL;		/* webMenuFn() argument */
WEB_LogFn      webLogFn = NULL;
WEB_DestroyFn  webDestroyFn = NULL;

char webWorkerSess[WEB_SESSID_MAX];		/* Session ID (in Worker) */
char webWorkerUser[WEB_USERNAME_MAX];		/* Username (in Worker) */
struct web_variableq webVars;			/* Template variables */
Uint webQueryCount;				/* Query counter */
struct web_session_socketq webWorkSockets;	/* Frontend->Worker sockets */

static volatile sig_atomic_t termFlag=0, chldFlag=0, pipeFlag=0;

static int  webEventSource;			   /* Is an event source */
static Uint webClusterID;			   /* Server instance no */
static int  webFrontSockets[WEB_MAXWORKERSOCKETS]; /* Worker->Frontend */
static Uint webFrontSocketCount;
static int  webCtrlSock;			   /* Local control socket */
static char webPeerAddress[256];		   /* Peer address */

static const int   webLogLvlNameLength = 6;
static const char *webLogLvlNames[] = {
	" emerg",
	" alert",
	"  crit",
	"   err",
	"  warn",
	"notice",
	"  info",
	" debug",
	" query",
	"worker",
	" event"
};
static const char *webKillEvent = "event: KILL\n"
                                  "data: 0\n\n";

/* #define WEB_DEBUG_FORMDATA */
#define WEB_DEBUG_QUERIES
#define WEB_DEBUG_REQUESTS
/* #define WEB_DEBUG_TRANSFER */

#ifndef MIN
#define MIN(a,b) AG_MIN((a),(b))
#endif
#ifndef MAX
#define MAX(a,b) AG_MAX((a),(b))
#endif

/* Parse HTTP request URL (with optional arguments). */
static int
ParseURL(WEB_Query *_Nonnull q, const char *_Nonnull s)
{
	char path[WEB_URL_MAX];
	char *c, *pathArgs, *pc;

	if (*s != '/') {
		WEB_SetCode(q, "400 Bad Request");
		return (-1);
	}
	if (Strlcpy(path, &s[1], sizeof(path)) >= sizeof(path)) {
		WEB_SetCode(q, "414 Request-URI Too Long");
		return (-1);
	}
	if ((c = strpbrk(path, "?&/")) != NULL) {
		pathArgs = &c[1];
		*c = '\0';
	} else {
		pathArgs = "";
	}
	if (strlen(path) >= WEB_OPNAME_MAX) {
		WEB_SetCode(q, "414 Request-URI Too Long");
		return (-1);
	}
	for (pc = path; *pc != '\0'; pc++) {
		if (!isalnum(*pc) && !strchr("_-.", *pc)) {
			WEB_SetCode(q, "400 Bad Request");
			return (-1);
		}
	}
	WEB_SetS(q, "op", (path[0] != '\0') ? path : webHomeOp);

	if (pathArgs[0] != '\0') {
		if (strchr(pathArgs,'=')) {
			if (WEB_ParseFormUrlEncoded(q, pathArgs,
			    WEB_GET_ARGUMENT) == -1) {
				WEB_SetCode(q, "400 Bad Request");
				return (-1);
			}
		} else {
			WEB_SetS(q, "_argument", pathArgs);
		}
	}
	return (0);
}

/* Parse the HTTP "Cookie:" header. */
static int
ParseCookie(WEB_Query *_Nonnull q, char *_Nonnull s)
{
	WEB_Cookie *ck;
	char *sp = s, *t, *end;

	while ((t = Strsep(&sp, ";")) != NULL) {
		char *sKey, *sVal;

		if (strchr(t, '=') == NULL) { continue; }
		if ((sKey = Strsep(&t, "=")) == NULL) { continue; }
		WEB_TRIM_WHITESPACE(sKey, end);
		if ((sVal = Strsep(&t, "=")) != NULL) {
			WEB_TRIM_WHITESPACE(sVal, end);
		} else {
			sVal = "";
		}
		if ((ck = WEB_LookupCookie(q, sKey)) == NULL) {
			if (!(ck = TryMalloc(sizeof(WEB_Cookie)))) {
				WEB_SetCode(q, "500 Internal Server Error");
				return (-1);
			}
			Strlcpy(ck->name, sKey, sizeof(ck->name));
			TAILQ_INSERT_HEAD(&q->cookies, ck, cookies);
			q->nCookies++;
		}
		ck->expires[0] = '\0';
		ck->domain[0] = '\0';
		ck->path[0] = '\0';
		ck->flags = 0;
		Strlcpy(ck->value, sVal, sizeof(ck->value));
	}
	return (0);
}

/* Parse the HTTP "Accept-Language:" header and negotiate the default. */
static int
ParseAcceptLanguage(WEB_Query *_Nonnull q, char *_Nonnull s)
{
	char *sp = s, *t;
	char **lang;
	Uint i;

	q->nAcceptLangs = 0;
	while ((t = Strsep(&sp, ",;")) != NULL) {
		if (t[0] == 'q' && t[1] == '=') {	/* XXX quality */
			continue;
		}
		if (q->nAcceptLangs+1 > WEB_LANGS_MAX) {
			WEB_SetCode(q, "400 Bad Request");
			return (-1);
		}
		Strlcpy(q->acceptLangs[q->nAcceptLangs++], t, WEB_LANG_CODE_MAX);
	}
	for (lang = &webLangs[0]; *lang != NULL; lang++) {
		for (i = 0; i < q->nAcceptLangs; i++) {
			if (strcasecmp(q->acceptLangs[i], *lang) != 0) {
				continue;
			}
			Strlcpy(q->lang, *lang, sizeof(q->lang));
			break;
		}
		if (i < q->nAcceptLangs)
			break;
	}
	return (0);
}

/*
 * HTTP methods
 */

/* Common in all HTTP frontend methods. */
static __inline__ int
WEB_InitFrontQuery(WEB_Query *_Nonnull q, WEB_Method meth, int sock,
    const char *_Nonnull url)
{
	time_t t = time(NULL);
	struct tm tm;

	WEB_QueryInit(q, webLangs[0]);
	q->method = meth;
	q->sock = sock;
	if (ParseURL(q, url) == -1) {
		return (-1);
	}
	if (gmtime_r(&t, &tm) != NULL) {
		strftime(q->date, sizeof(q->date), "%a, %d %h %Y %T %Z", &tm);
		WEB_SetHeaderS(q, "Date", q->date);
	}
	WEB_SetHeaderS(q, "Server", agProgName);
	return (0);
}

/* Parse Range request header */
static int
ParseRange(WEB_Query *_Nonnull q, char *_Nonnull s)
{
	char *from, *to;
	const char *err;

	if (strchr(s, ',') != NULL ||			/* Single range only */
	    (from = Strsep(&s, "-")) == NULL ||
	    (to = Strsep(&s, "-")) == NULL) {
		goto fail_416;
	}
	q->rangeFrom = (int)strtonum(from, 0, AG_INT_MAX, &err);
	if (err) { goto fail_416; }
	q->rangeTo = (int)strtonum(to, 0, AG_INT_MAX, &err);
	if (err) { goto fail_416; }
	q->flags |= WEB_QUERY_RANGE;
	return (0);
fail_416:
	WEB_SetCode(q, "416 Range Not Satisfiable");
	return (-1);
}

/* Parse HTTP headers common to all methods. */
static __inline__ int
WEB_ParseHeader(WEB_Query *_Nonnull q, char *_Nonnull s)
{
	if (strcasecmp(s, "Connection: keep-alive") == 0) {
		q->flags |= WEB_QUERY_KEEPALIVE;
#ifdef HAVE_ZLIB
	} else if (strncasecmp(s, "Accept-Encoding: ",17)==0 &&
	           (strcasestr(&s[17], "deflate") != NULL)) {	/* or x-deflate */
		q->flags |= WEB_QUERY_DEFLATE;
#endif
	} else if (strncasecmp(s, "X-Forwarded-For: ",17)==0) {
		q->flags |= WEB_QUERY_PROXIED;
		Strlcpy(q->userIP, &s[17], sizeof(q->userIP));
	} else if (strncasecmp(s, "X-Forwarded-Host: ",18)==0) {
		Strlcpy(q->userHost, &s[18], sizeof(q->userHost));
	} else if (strncasecmp(s, "User-Agent: ",12)==0) {
		Strlcpy(q->userAgent, s, sizeof(q->userAgent));
	} else if (strncasecmp(s, "Accept-Language: ",17)==0) {
		return ParseAcceptLanguage(q, &s[17]);
	} else if (strncasecmp(s, "Cookie: ",8)==0) {
		return ParseCookie(q, &s[8]);
	} else if (strncasecmp(s, "Range: ",7)==0) {
		return ParseRange(q, &s[7]);
	}
	return (0);
}

/* Maintain keep-alive if requested. */
static __inline__ int
WEB_KeepAlive(WEB_Query *_Nonnull q)
{
	if (q->flags & WEB_QUERY_KEEPALIVE) {
		WEB_SetHeaderS(q, "Connection", "keep-alive");
/*		WEB_SetHeaderS(q, "Keep-Alive", "timeout=30, max=25"); */
		return (1);
	} else {
		WEB_SetHeaderS(q, "Connection", "close");
		return (0);
	}
}

static int
WEB_FrontGET(int sock, const char *_Nonnull url, char *_Nonnull pHeaders,
    void *_Nonnull rdBuf, AG_Size rdBufLen, const WEB_SessionOps *_Nonnull Sops)
{
	char *headers = pHeaders, *s, *cEnd;
	WEB_Query q;
	int rv;
	
	WEB_Log(WEB_LOG_QUERY,
	    "GET: url=%s, headers=[%s], rdBufLen=%lu", url,
	    pHeaders, (Ulong)rdBufLen);

	if (WEB_InitFrontQuery(&q, WEB_METHOD_GET, sock, url) == -1) {
		goto fail;
	}
	while ((s = strsep(&headers, "\n")) != NULL) {
		if ((cEnd = strchr(s,'\r')) != NULL) {
			*cEnd = '\0';
		}
		if (WEB_ParseHeader(&q, s) == -1) {
			rv = 0;
			goto fail;
		}
	}
	q.flags |= WEB_QUERY_CONTENT_READ;
	rv = WEB_ProcessQuery(&q, Sops, rdBuf, rdBufLen);
	WEB_QueryDestroy(&q);
	return (rv);
fail:
	WEB_SetHeaderS(&q, "Content-Language", "en");
	WEB_OutputError(&q, AG_GetError());
	rv = WEB_KeepAlive(&q);
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	WEB_LogErr("GET(%s): %s", url, AG_GetError());
	return (rv);
}

static int
WEB_FrontHEAD(int sock, const char *_Nonnull url, char *_Nonnull pHeaders,
    void *_Nonnull rdBuf, AG_Size rdBufLen, const WEB_SessionOps *_Nonnull Sops)
{
	char *headers = pHeaders, *s, *cEnd;
	WEB_Query q;
	int rv;

	WEB_Log(WEB_LOG_QUERY,
	    "HEAD: url=%s, headers=[%lu], rdBufLen=%lu", url,
	    strlen(pHeaders), (Ulong)rdBufLen);

	if (WEB_InitFrontQuery(&q, WEB_METHOD_HEAD, sock, url) == -1) {
		goto fail;
	}
	while ((s = strsep(&headers, "\n")) != NULL) {
		if ((cEnd = strchr(s,'\r')) != NULL) {
			*cEnd = '\0';
		}
		if (WEB_ParseHeader(&q, s) == -1) {
			rv = 0;
			goto fail;
		}
	}
	q.flags |= WEB_QUERY_CONTENT_READ;
	rv = WEB_ProcessQuery(&q, Sops, rdBuf, rdBufLen);
	WEB_QueryDestroy(&q);
	return (rv);
fail:
	WEB_SetHeaderS(&q, "Content-Language", "en");
	WEB_OutputError(&q, AG_GetError());
	rv = WEB_KeepAlive(&q);
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	WEB_LogErr("HEAD(%s): %s", url, AG_GetError());
	return (rv);
}

static int
WEB_FrontPOST(int sock, const char *_Nonnull url, char *_Nonnull pHeaders,
    void *_Nonnull pRdBuf, AG_Size rdBufLen, const WEB_SessionOps *_Nonnull Sops)
{
	char *rdBuf = pRdBuf;
	char *headers = pHeaders, *s, *cEnd;
	WEB_Query q;
	int rv = 0;
	
	WEB_Log(WEB_LOG_QUERY,
	    "POST: url=%s, headers=[%s], rdBufLen=%lu", url,
	    pHeaders, (Ulong)rdBufLen);

	if (WEB_InitFrontQuery(&q, WEB_METHOD_POST, sock, url) == -1) {
		goto fail;
	}
	while ((s = strsep(&headers, "\n")) != NULL) {
		if ((cEnd = strchr(s,'\r')) != NULL) {
			*cEnd = '\0';
		}
		if (strncasecmp(s, "Content-Type: ",14)==0) {
			Strlcpy(q.contentType, &s[14], sizeof(q.contentType));
		} else if (strncasecmp(s, "Content-Length: ",16)==0) {
			q.contentLength = (AG_Size)strtol(&s[16], NULL, 10);
		} else if (WEB_ParseHeader(&q, s) == -1) {
			goto fail;
		}
	}

	if (((strncmp(q.contentType, "application/x-www-form-urlencoded",33)==0) ||
	     strncmp(q.contentType, "application/json",16)==0) &&
	     q.contentLength > 0) {
		/*
		 * Parse URL-encoded arguments (must fit in existing buffer).
		 */
		if (q.contentLength > WEB_FRONTEND_RDBUFSIZE-rdBufLen-1) {
			WEB_SetCode(&q, "400 Bad Request");
			AG_SetError("Urlenc body too large (max %lu)",
			    (Ulong)(WEB_FRONTEND_RDBUFSIZE - rdBufLen));
			goto fail;
		}
		if (WEB_SYS_Read(sock, &rdBuf[rdBufLen], q.contentLength - rdBufLen) == -1) {
			WEB_SetCode(&q, "500 Internal Server Error");
			goto fail;
		}
		((char *)rdBuf)[q.contentLength] = '\0';
		WEB_LogDebug("URLENCODED: Parsing %lu bytes (\"%s\")",
		    (Ulong)q.contentLength, (char *)rdBuf);
		if (WEB_ParseFormUrlEncoded(&q, rdBuf, WEB_POST_ARGUMENT) == -1) {
			WEB_SetCode(&q, "400 Bad Request");
			goto fail;
		}
		q.flags |= WEB_QUERY_CONTENT_READ;
	} else if (strncmp(q.contentType, "multipart/form-data",19) == 0 &&
	           q.contentLength > 0) {
		WEB_LogDebug("FORMDATA: Reading %lu bytes", (Ulong)q.contentLength);
		if (q.contentLength > WEB_FORMDATA_MAX) {
			WEB_LogErr("Client Content-Length: %luK > %uK",
			    (Ulong)(q.contentLength/1024), WEB_FORMDATA_MAX/1024);
			WEB_SetCode(&q, "400 Bad Request");
			goto fail;
		}
		/*
		 * Let the Worker process parse the multipart/form-data
		 * contents generically (after QueryLoad).
		 */
	} else {
		/* For all other Content-Type's, let the ops handle the read. */
	}
	rv = WEB_ProcessQuery(&q, Sops, rdBuf, rdBufLen);
	WEB_QueryDestroy(&q);
	return (rv);
fail:
	WEB_SetHeaderS(&q, "Content-Language", "en");
	WEB_OutputError(&q, AG_GetError());
	rv = WEB_KeepAlive(&q);
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	WEB_LogErr("POST(%s): %s", url, AG_GetError());
	return (rv);
}

static int
WEB_FrontOPTIONS(int sock, const char *_Nonnull url, char *_Nonnull pHeaders,
    void *_Nonnull rdBuf, AG_Size rdBufLen, const WEB_SessionOps *_Nonnull Sops)
{
	char *headers = pHeaders, *s, *cEnd;
	WEB_Query q;
	int rv;

	WEB_Log(WEB_LOG_QUERY,
	    "OPTIONS : url=%s, headers=[%lu], rdBufLen=%lu", url,
	    strlen(pHeaders), (Ulong)rdBufLen);

	if (url[0] == '*') {
		url = "/";
	}
	if (WEB_InitFrontQuery(&q, WEB_METHOD_OPTIONS, sock, url) == -1) {
		goto fail;
	}
	while ((s = strsep(&headers, "\n")) != NULL) {
		if ((cEnd = strchr(s,'\r')) != NULL) {
			*cEnd = '\0';
		}
		if (strncasecmp(s, "Content-Type: ",14)==0) {
			Strlcpy(q.contentType, &s[14], sizeof(q.contentType));
		} else if (strncasecmp(s, "Content-Length: ",16)==0) {
			q.contentLength = (AG_Size)strtol(&s[16], NULL, 10);
		} else if (WEB_ParseHeader(&q, s) == -1) {
			rv = 0;
			goto fail;
		}
	}
	if (q.contentLength > 0 && q.contentType[0] == '\0') {
		WEB_SetCode(&q, "400 Bad Request");
		goto fail;
	}
	if (q.contentLength > rdBufLen) {
		char ignore[1024];
		if (q.contentLength-rdBufLen > sizeof(ignore)) {
			WEB_SetCode(&q, "400 Bad Request");
			q.flags &= ~(WEB_QUERY_KEEPALIVE);	/* Close */
			goto fail;
		}
		WEB_SYS_Read(sock, ignore, q.contentLength-rdBufLen);
	}
	q.flags |= WEB_QUERY_CONTENT_READ;
	rv = WEB_KeepAlive(&q);
	WEB_BeginFrontQuery(&q, url, Sops);
	WEB_SetHeaderS(&q, "Allow", "GET,HEAD,POST,OPTIONS");
	WEB_SetHeaderS(&q, "Vary", "User-Agent");
	WEB_SetHeaderS(&q, "Content-Length", "0");
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	return (rv);
fail:
	WEB_SetHeaderS(&q, "Content-Language", "en");
	WEB_OutputError(&q, AG_GetError());
	rv = WEB_KeepAlive(&q);
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	WEB_LogErr("OPTIONS(%s): %s", url, AG_GetError());
	return (rv);
}

static int
WEB_MethodNotAllowed(int sock, const char *_Nonnull url,
    char *_Nonnull pHeaders, void *_Nonnull rdBuf, AG_Size rdBufLen,
    const WEB_SessionOps *_Nonnull Sops)
{
	WEB_Query q;

	WEB_InitFrontQuery(&q, 0, sock, url);
	WEB_SetCode(&q, "405 Method not allowed");
	WEB_SetHeaderS(&q, "Allow", "GET, HEAD, POST, OPTIONS");
	WEB_FlushQuery(&q);
	WEB_QueryDestroy(&q);
	return (0);
}

/*
 * Implemented HTTP frontend methods
 */
const WEB_MethodOps webMethods[] = {
	{ "GET",	WEB_FrontGET },
	{ "HEAD",	WEB_FrontHEAD },
	{ "POST",	WEB_FrontPOST },
	{ "OPTIONS",	WEB_FrontOPTIONS },
	{ "PUT",	WEB_MethodNotAllowed },
	{ "DELETE",	WEB_MethodNotAllowed },
	{ "TRACE",	WEB_MethodNotAllowed },
	{ "CONNECT",	WEB_MethodNotAllowed }
};

void
WEB_RegisterModule(WEB_ModuleClass *Cmod)
{
	AG_ObjectClass *Cobj = (AG_ObjectClass *)Cmod;
	WEB_Module *mod;

	AG_RegisterClass(Cmod);

	mod = Malloc(Cobj->size);
	AG_ObjectInit(mod, Cobj);
	webModules = Realloc(webModules, (webModuleCount+1)*sizeof(WEB_Module *));
	webModules[webModuleCount++] = mod;
}

/* Escape characters as described in RFC1738. */
char *
WEB_EscapeURL(WEB_Query *q, const char *url)
{
	char hex[3];
	size_t len;
	const u_char *sp;
	char *dst, *dp;

	len = strlen(url)+1;
	dp = dst = Malloc(len);
	hex[2] = '\0';
	for (sp = (const u_char *)url; *sp != '\0'; dp++, sp++) {
		const char *url_unsafe = "<>\"#%{}|\\^~[]`", *p;
		int unsafe = 0;

		if (!isgraph(*sp)) {
			unsafe++;
		} else {
			for (p = &url_unsafe[0]; *p != '\0'; p++) {
				if (*p == *sp) {
					unsafe++;
					break;
				}
			}
		}
		if (unsafe) {
			snprintf(hex, sizeof(hex), "%02x", *sp);
			len += 3;
			dst = Realloc(dst, len);
			dp[0] = '%';
			dp[1] = hex[0];
			dp[2] = hex[1];
			dp += 2;
		} else {
			*dp = *sp;
		}
	}
	*dp = '\0';
	return (dst);
}

/*
 * Unescape characters as described in RFC1738, except for NUL sequences
 * which are replaced by underscores.
 */
char *
WEB_UnescapeURL(WEB_Query *q, const char *url)
{
	char hex[3];
	const char *sp;
	char *dst, *dp;
	Uint n;

	dp = dst = Malloc(strlen(url) + 2);
	hex[2] = '\0';
	for (sp = url; *sp != '\0'; dp++, sp++) {
		if (sp[0] == '%' && isxdigit(sp[1]) && isxdigit(sp[2])) {
			hex[0] = sp[1];
			hex[1] = sp[2];
			n = (Uint)strtoul(hex, NULL, 16);
			if (n == '\0') {
				*dp = '_';
			} else {
				*dp = n;
			}
			sp += 2;
		} else if (sp[0] == '+') {
			*dp = ' ';
		} else {
			*dp = sp[0];
		}
	}
	*dp = '\0';
	return (dst);
}

/* Parse application/x-www-form-urlencoded arguments (modifies data). */
int
WEB_ParseFormUrlEncoded(WEB_Query *q, char *qsinput, enum web_argument_type t)
{
	WEB_Argument *arg = NULL;
	char *qstring, *qs, *s;
	char *name, *value;
	int nargs;

/*	WEB_LogDebug("WEB_ParseFormUrlEncoded(%s,%d)", qsinput, t); */

	if ((qs = qstring = TryStrdup(qsinput)) == NULL)
		return (-1);

	for (nargs = 0;
	    (s = Strsep(&qs, "&")) != NULL && nargs < WEB_MAX_ARGS;
	     nargs++) {
		if ((arg = TryMalloc(sizeof(WEB_Argument))) == NULL) {
			free(qstring);
			return (-1);
		}
		if ((name = Strsep(&s, "=")) != NULL && *name != '\0') {
			if (Strlcpy(arg->key, name, sizeof(arg->key)) >=
			    sizeof(arg->key)) {
				AG_SetErrorS("Key is too long");
				goto fail;
			}
			if ((value = Strsep(&s, "=")) != NULL) {
				if ((arg->value = WEB_UnescapeURL(q, value)) == NULL) {
					goto fail;
				}
				arg->len = strlen(arg->value)+1;
				if (arg->len > WEB_ARG_LENGTH_MAX) {
					free(arg->value);
					free(arg);
					goto fail;
				}
			} else {
				if ((arg->value = TryStrdup("")) == NULL) {
					goto fail;
				}
				arg->len = 0;
			}
		    	arg->type = t;
			arg->contentType[0] = '\0';
			TAILQ_INSERT_TAIL(&q->args, arg, args);
			q->nArgs++;
		}
	}
	free(qstring);
	return (0);
fail:
	free(arg);
	free(qstring);
	return (-1);
}

/*
 * Parse multipart/form-data content into WEB_Argument items.
 * Parts may include (potentially large) binary blobs. This is done in-Worker.
 */
int
WEB_ReadFormData(WEB_Query *q, int sock)
{
	char *s, *t, *tEnd, *buf, *c, *cStart, *cEnd, *cNext;
	size_t lenBoundary, lenPart;
	WEB_Argument *arg = NULL;
	int partIndex = 0;
	char boundary[73];

	boundary[0] = '-';
	boundary[1] = '-';
	boundary[2] = '\0';
	if (!(s = strstr(q->contentType,"boundary=")) || s[9]=='\0' ||
	    Strlcat(boundary, &s[9], sizeof(boundary)) >= sizeof(boundary)) {
		AG_SetErrorS("Bad boundary");
		return (-1);
	}
	lenBoundary = strlen(boundary);
#ifdef WEB_DEBUG_FORMDATA
	WEB_LogDebug("FormData: Content-Length: %lu", (Ulong)q->contentLength);
	WEB_LogDebug("FormData: Boundary: \"%s\"", boundary);
#endif
	if ((buf = TryMalloc(q->contentLength + 1)) == NULL) {
		return (-1);
	}
	if (WEB_SYS_Read(sock, buf, q->contentLength) != 0) {
		AG_SetError("stdin: %s", strerror(errno));
		goto fail;
	}
	buf[q->contentLength] = '\0';
	c = &buf[0];

#ifdef WEB_DEBUG_REQUESTS
	{
		FILE *dbgOut = fopen("debug-form.txt", "w");
		if (dbgOut != NULL) {
			fwrite(buf, 1, q->contentLength, dbgOut);
			fclose(dbgOut);
		}
	}
#endif
	while (*c != '\0') {
		if (*c != '-' ||
		    (partIndex==0 && strncmp(c, boundary, lenBoundary) != 0)) {
			c++;
			continue;
		}
		c += lenBoundary + 2;	/* + \r\n */
		if ((cEnd = strchr(c,'\n')) == NULL) {
			AG_SetErrorS("Incomplete MIME header");
			goto fail;
		}
		cStart = &cEnd[1];
		while (isspace(*cStart)) { cStart++; }
		cNext = (*cEnd != '\0' && cEnd[1] == 'C') ? &cEnd[1] : NULL;
		*cEnd = '\0';
		
		/* Extract name from Content-Disposition header. */
		if (strncasecmp(c, "Content-Disposition:", 20) == 0) {
			s = c;
			while ((t = strsep(&s, ";")) != NULL) {
				while (isspace(*t)) { t++; }
				if (strncmp(t, "name=\"", 6) == 0 &&
				    (tEnd = strchr(&t[6], '"')) != NULL) {
					*tEnd = '\0';
					t += 6;
					break;
				}
			}
			if (t == NULL) 		/* End or header without name */
				break;
		} else {
			break;
		}
		if ((arg = TryMalloc(sizeof(WEB_Argument))) == NULL) {
			goto fail;
		}
		arg->type = WEB_POST_ARGUMENT;
		Strlcpy(arg->key, t, sizeof(arg->key));

		/* Extract type from optional Content-Type header. */
		if (cNext && strncasecmp(cNext, "Content-Type:", 13) == 0 &&
		    (tEnd = strchr(&cNext[13], '\r')) != NULL) {
			if ((cEnd = strchr(cNext,'\n')) == NULL) {
				AG_SetErrorS("Incomplete Content-Type header");
				free(arg);
				goto fail;
			}
			*tEnd = '\0';
			Strlcpy(arg->contentType,
			    isspace(cNext[13]) ? &cNext[14] : &cNext[13],
			    sizeof(arg->contentType));
			cStart = &cEnd[3]; /* 1+\r\n */
		} else {
			arg->contentType[0] = '\0';
		}

		if ((cEnd = (char *)memmem(cStart, (&buf[q->contentLength] - cStart),
		    boundary, lenBoundary)) == NULL) {
			AG_SetError("Incomplete FORM data (part #%u)", partIndex);
			free(arg);
			goto fail;
		}
		lenPart = (size_t)(cEnd - cStart);
		partIndex++;
		if (lenPart+1 > WEB_ARG_LENGTH_MAX) {
			AG_SetError("%s: Too big (max %uK)", arg->key,
			    WEB_ARG_LENGTH_MAX/1024);
			free(arg);
			goto fail;
		}
		if ((arg->value = TryMalloc(lenPart+1)) == NULL) {
			free(arg);
			goto fail;
		}
		memcpy(arg->value, cStart, lenPart);

		if (lenPart >= 2 &&
		    arg->value[lenPart-1] == '\n' &&		/* Strip \r\n */
		    arg->value[lenPart-2] == '\r') {
			arg->value[lenPart-2] = '\0';
			arg->len = lenPart-2+1;
		} else {
			arg->value[lenPart] = '\0';
			arg->len = lenPart+1;
		}
#ifdef WEB_DEBUG_FORMDATA
		WEB_LogDebug("FormData: Part %d: \"%s\"=[%s]", partIndex,
		    arg->key, arg->value);
#endif
		TAILQ_INSERT_TAIL(&q->args, arg, args);
		q->nArgs++;
		c = cEnd;
	}

	free(buf);
	return (0);
fail:
	free(buf);
	return (-1);
}

/* Set a cookie value. */
WEB_Cookie *_Nonnull
WEB_SetCookieS(WEB_Query *q, const char *name, const char *value)
{
	WEB_Cookie *ck;
	
	TAILQ_FOREACH(ck, &q->cookies, cookies) {
		if (strcmp(ck->name, name) == 0)
			break;
	}
	if (ck == NULL) {
		ck = Malloc(sizeof(WEB_Cookie));
		Strlcpy(ck->name, name, sizeof(ck->name));
		TAILQ_INSERT_HEAD(&q->cookies, ck, cookies);
		q->nCookies++;
	}
	ck->expires[0] = '\0';
	ck->domain[0] = '\0';
	ck->path[0] = '\0';
	ck->flags = 0;
	Strlcpy(ck->value, value, sizeof(ck->value));
	return (ck);
}

/* Set a cookie value (format string variant). */
WEB_Cookie *
WEB_SetCookie(WEB_Query *q, const char *name, const char *fmt, ...)
{
	WEB_Cookie *ck;
	va_list ap;

	ck = WEB_SetCookieS(q, name, "");
	va_start(ap, fmt);
	vsnprintf(ck->value, sizeof(ck->value), fmt, ap);
	va_end(ap);
	return (ck);
}

void
WEB_DelCookie(WEB_Query *q, const char *name)
{
	WEB_Cookie *ck;

	ck = WEB_SetCookieS(q, name, "");
	Strlcpy(ck->expires, "Thu, 01 Jan 1970 00:00:01 GMT", sizeof(ck->expires));
}

/* Register a language-switch callback function. */
void
WEB_SetLanguageFn(WEB_LanguageFn fn, void *p)
{
	webLanguageFn = fn;
	webLanguageFnArg = p;
}

/* Register an alternate function to construct the menu. */
void
WEB_SetMenuFn(WEB_MenuFn fn, void *p)
{
	webMenuFn = fn;
	webMenuFnArg = p;
}

/* Set cleanup routine to be called on application server exit. */
void
WEB_SetDestroyFn(WEB_DestroyFn fn)
{
	webDestroyFn = fn;
}

/* Initialize a WEB_Query structure. */
void
WEB_QueryInit(WEB_Query *q, const char *lang)
{
	q->method = 0;
	q->flags = 0;
	q->compressLvl = WEB_DATA_COMPRESS_LVL;
	q->nAcceptLangs = 0;
	TAILQ_INIT(&q->args);
	TAILQ_INIT(&q->cookies);
	q->nArgs = 0;
	q->nCookies = 0;
	q->contentType[0] = '\0';
	q->contentLength = 0;
	q->sess = NULL;
	q->sock = -1;
	q->date[0] = '\0';
	q->userIP[0] = '\0';
	q->userHost[0] = '\0';
	q->userAgent[0] = '\0';
	q->code[0] = '\0';
	Strlcpy(q->lang, lang, sizeof(q->lang));

	WEB_ClearHeaders(q, "HTTP/1.0 200 OK\r\n");
	
	q->data = NULL;
	q->dataSize = 0;
	q->dataLen = 0;
}

/* Prepare for processing a Frontend or a Worker query. */
static __inline__ void
WEB_BeginQueryCommon(WEB_Query *_Nonnull q)
{
	if (q->data == NULL) {
		q->data = Malloc(WEB_DATA_BUFSIZE);
		q->dataSize = WEB_DATA_BUFSIZE;
	}
	q->dataLen = 0;

	SetS("_error", "");
	SetS("_lang", q->lang);
}

/* Initialize Frontend environment for processing a sessionless query. */
void
WEB_BeginFrontQuery(WEB_Query *q, const char *op, const WEB_SessionOps *Sops)
{
	WEB_BeginQueryCommon(q);
	
	/* Never allow Range requests for sessionless queries. */
	q->flags &= ~(WEB_QUERY_RANGE);

	if (Sops->beginFrontQuery != NULL) {
		Sops->beginFrontQuery(q, op);
	} else {
		SetS("_user", "(nobody)");
		SetS("_theme", "Default");
		Set("_modules",
		    "<li>"
		      "<a style='font-weight:bold' href='/'>"
		      "%s&nbsp;%s</a>"
		    "</li>"
		    "<li role='separator' class='divider'></li>",
		    WEB_GLYPHICON(log-in), _("Log in"));
	}
}

/* Initialize Worker environment for processing an authenticated query. */
void
WEB_BeginWorkerQuery(WEB_Query *q)
{
	const char *op;
	WEB_Cookie *ck;
	VAR *v;
	int i;

	WEB_BeginQueryCommon(q);

	WEB_SetHeaderS(q, "Date", q->date);
	if (q->flags & WEB_QUERY_KEEPALIVE) {
		WEB_SetHeaderS(q, "Connection", "keep-alive");
	}
	TAILQ_FOREACH(ck, &q->cookies, cookies) {
		char sc[WEB_COOKIE_SET_MAX];

		Strlcpy(sc, ck->name, sizeof(sc));
		Strlcat(sc, "=", sizeof(sc));
		Strlcat(sc, ck->value, sizeof(sc));
		if (ck->expires[0] != '\0') {
			Strlcat(sc, "; Expires=", sizeof(sc));
			Strlcat(sc, ck->expires, sizeof(sc));
		}
		if (ck->domain[0] != '\0') {
			Strlcat(sc, "; Domain=", sizeof(sc));
			Strlcat(sc, ck->domain, sizeof(sc));
		}
		if (ck->path[0] != '\0') {
			Strlcat(sc, "; Path=", sizeof(sc));
			Strlcat(sc, ck->path, sizeof(sc));
		}
		if (ck->flags & WEB_COOKIE_SECURE) {
			Strlcat(sc, "; Secure", sizeof(sc));
		}
		WEB_AppendHeaderS(q, "Set-Cookie", sc);
	}

	if ((op = WEB_Get(q, "op", WEB_OPNAME_MAX)) != NULL) {
		SetS("op", op);
	} else {
		SetS("op", webHomeOp);
	}

	/* Generate the navigation menu. */
	v = SetS("_modules", NULL);
	if (webMenuFn != NULL) {				/* Custom */
		webMenuFn(q, v, webMenuFnArg);
		return;
	}
	for (i = 0; i < webModuleCount; i++) {
		WEB_Module *mod = webModules[i];
		WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;
		WEB_MenuSection *sec;

		if (Cmod->menu != NULL) {
			Cmod->menu(mod, q, v);
		} else if (Cmod->menuSections == NULL) {
#ifdef ENABLE_NLS
			Cat(v, "<li><a href='/%s'>%s%s</a></li>",
			    Cmod->name, Cmod->icon, gettext(Cmod->lname));
#else
			Cat(v, "<li><a href='/%s'>%s%s</a></li>",
			    Cmod->name, Cmod->icon, Cmod->lname);
#endif
		} else {
			if (Cmod->menuSections[0].name != NULL) {
				CatS(v,
				    "<li class='dropdown'>"
				      "<a href='#' class='dropdown-toggle' "
				       "data-toggle='dropdown' role='button' "
				       "aria-haspopup='true' aria-expanded='false'>");
				CatS(v, Cmod->icon);
#ifdef ENABLE_NLS
				CatS(v, gettext(Cmod->lname));
#else
				CatS(v, Cmod->lname);
#endif

				CatS(v, "<span class='caret'></span></a>"
				        "<ul class='dropdown-menu'>");
				for (sec = &Cmod->menuSections[0];
				     sec->name != NULL;
				     sec++) {
					Cat(v,
					    "<li><a href='/%s'>%s</a></li>",
					    sec->cmd, sec->name);
				}
				CatS(v, "</ul>"
				        "</li>");
			}
		}
	}

	Cat(v,
	    "<li><a href='/logout'>%s&nbsp;%s</a></li>"
	    "<li role='separator' class='divider'></li>",
	    WEB_GLYPHICON(log-out),
	    _("Logout"));
}

#ifdef HAVE_ZLIB
static void
WEB_FlushQuery_DEFLATE(WEB_Query *_Nonnull q)
{
	Uint8 out[WEB_DATA_BUFSIZE];
	size_t nRead=0, nWrote=0, nGzipped;
	char chunkHead[16];
	int flush=0, rv;
	z_stream strm;

/*	WEB_LogDebug("FlushQuery_DEFLATE(method=%s, head=%u, data=%lu, lvl=%d)",
	    webMethods[q->method].name, q->headLen, q->dataLen, q->compressLvl); */
	
	WEB_SetHeaderS(q, "Content-Encoding", "deflate");

	/*
	 * Write headers. Use chunked transfer encoding if method requires a
	 * body (except for HEAD method in which case we just compute compressed
	 * length and return headers with Content-Length).
	 */
	if (q->method != WEB_METHOD_HEAD) {
		WEB_SetHeaderS(q, "Transfer-Encoding", "chunked");
		WEB_WriteHeaders(q->sock, q);
	}
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if ((rv = deflateInit(&strm, q->compressLvl)) != Z_OK) {
		WEB_LogErr("deflateInit: error %d", rv);
		return;
	}
	do {
		if ((q->dataLen - nRead) < sizeof(out)-2) {
			strm.avail_in = q->dataLen - nRead;
			flush = Z_FINISH;
		} else {
			strm.avail_in = sizeof(out)-2;
			flush = Z_NO_FLUSH;
		}
		if (strm.avail_in == 0) {
			break;
		}
		strm.next_in = &q->data[nRead];
		nRead += strm.avail_in;
		do {
			strm.avail_out = sizeof(out)-2;
			strm.next_out = out;
#if 0
			WEB_LogDebug("deflate(%u -> %u, %s)", strm.avail_in,
			    strm.avail_out, flush == Z_FINISH ? "FINISH" :
			                                        "NO_FLUSH");
#endif
			if ((rv = deflate(&strm, flush)) == Z_STREAM_ERROR) {
				WEB_LogErr("deflate: error %d", rv);
				AG_FatalError("deflate failed");
			}
			nGzipped = (sizeof(out)-2 - strm.avail_out);
			if (q->method != WEB_METHOD_HEAD && nGzipped > 0) {
				struct iovec vec[2];
				size_t chunkHeadLen;

				chunkHeadLen = snprintf(chunkHead,
				    sizeof(chunkHead), "%lx\r\n",
				    (Ulong)nGzipped);
				if (chunkHeadLen >= sizeof(chunkHead)) {
					AG_FatalError("chunkHeadLen");
				}
				out[nGzipped  ] = '\r';
				out[nGzipped+1] = '\n';

				vec[0].iov_base = chunkHead;
				vec[0].iov_len =  chunkHeadLen;
				vec[1].iov_base = out;
				vec[1].iov_len =  nGzipped+2;
				if (writev(q->sock, vec, 2) == -1)
					WEB_LogErr("Deflate writev: %s", strerror(errno));
			}
			nWrote += nGzipped+2;
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);		/* all input used */
	} while (flush != Z_FINISH);
	assert(rv == Z_STREAM_END);			/* stream complete */

	deflateEnd(&strm);
	WEB_LogDebug("DEFLATE: %lu -> %lu bytes (%.0f%% saving)",
	    (Ulong)q->dataLen, (Ulong)nWrote,
	    ((float)q->dataLen/(float)nWrote)*100.0f);

	if (q->method != WEB_METHOD_HEAD) {
		WEB_SYS_Write(q->sock, "0\r\n\r\n",5);
	} else {
		WEB_SetHeader(q, "Content-Length", "%lu", nWrote);
		WEB_WriteHeaders(q->sock, q);
	}
}
#endif /* HAVE_ZLIB */

/* Validate and process a Range request. */
static void
WEB_FlushQuery_RANGE(WEB_Query *_Nonnull q)
{
	Uint rangeLen;
	
	WEB_LogDebug("FlushQuery_RANGE(head=%u, range=%d-%d/%lu)",
	    q->headLen, q->rangeFrom, q->rangeTo, (Ulong)q->dataLen);

	if (q->rangeFrom > q->rangeTo ||
	    q->rangeTo > q->dataLen ||
	    (q->rangeFrom + q->rangeTo) > q->dataLen) {
		goto fail_416;
	}
	rangeLen = (q->rangeTo - q->rangeFrom);

	WEB_SetCode(q, "206 Partial Content");
	WEB_SetHeader(q, "Content-Range", "bytes %ld-%ld/%lu",
	    q->rangeFrom, q->rangeTo, q->dataLen);
	WEB_SetHeader(q, "Content-Length", "%u", rangeLen);

	/* Write HTTP headers and partial content. */
	WEB_WriteHeaders(q->sock, q);
	if (q->method != WEB_METHOD_HEAD)
		WEB_SYS_Write(q->sock, &q->data[q->rangeFrom], rangeLen);

	return;
fail_416:
	WEB_SetCode(q, "416 Range Not Satisfiable");
	WEB_SetHeaderS(q, "Content-Language", "en");
	WEB_OutputError(q, "Requested range is not satisfiable");
	WEB_WriteHeaders(q->sock, q);
	WEB_SYS_Write(q->sock, q->data, q->dataLen);
}

static __inline__ void
WEB_ClearQuery(WEB_Query *_Nonnull q)
{
	free(q->data);
	q->data = NULL;
	q->dataSize = 0;
	q->dataLen = 0;
}

/*
 * Write HTTP response headers and requested entity-body to client.
 * If the data size exceeds the compression threshold, enable compression and
 * and use chunked transfer encoding.
 */
void
WEB_FlushQuery(WEB_Query *q)
{
	if (q->flags & WEB_QUERY_RANGE) {		/* Range request */
		WEB_FlushQuery_RANGE(q);
#ifdef HAVE_ZLIB
	} else if ((q->flags & WEB_QUERY_DEFLATE) &&		/* Gzip */
	          !(q->flags & WEB_QUERY_NOCOMPRESSION) &&
	           (q->dataLen >= WEB_DATA_COMPRESS_MIN)) {
		WEB_FlushQuery_DEFLATE(q);
#endif
	} else {
/*		WEB_LogDebug("FlushQuery_PLAIN(head=%u, data=%lu)",
		    q->headLen, q->dataLen); */

		if (q->dataLen > 0) {
			WEB_SetHeader(q, "Content-Length", "%lu", q->dataLen);
		}
		WEB_WriteHeaders(q->sock, q);
		if (q->method != WEB_METHOD_HEAD)
			WEB_SYS_Write(q->sock, q->data, q->dataLen);
	}
	WEB_ClearQuery(q);
}

/*
 * Standard execution routine for module commands.
 * Handle special types [json] and [json-status].
 */
static void
WEB_CommandExec(WEB_Query *q, const char *op, WEB_Command *cmd)
{
	WEB_SetHeaderS(q, "Accept-Ranges", "bytes");
	WEB_SetHeaderS(q, "Vary", "Accept-Language,"
	                          "Accept-Encoding,"
	                          "User-Agent");
	WEB_SetHeaderS(q, "Last-Modified", q->date);
	WEB_SetHeaderS(q, "Content-Language", q->lang);
	WEB_SetHeaderS(q, "Cache-Control", "no-cache, no-store, "
	                                   "must-revalidate");
	WEB_SetHeaderS(q, "Expires", "0");

	if (cmd->type != NULL && strcmp(cmd->type, "[json-status]")==0) {
		WEB_SetHeaderS(q, "Content-Type", "application/json; "
		                                  "charset=utf8");
		if (cmd->fn(q) == 0) {
			WEB_PutS(q, "{\"code\": 0}\r\n");
		} else {
			WEB_PutS(q, "{\"code\": -1,");
			WEB_PutJSON_NoHTML_S(q, "error", AG_GetError());
			WEB_PutS(q, "\"backend_version\": \"" VERSION "\"}\r\n");
			WEB_LogErr("/%s: %s", op, AG_GetError());
		}
	} else if (cmd->type != NULL && strcmp(cmd->type, "[json]")==0) {
		WEB_SetHeaderS(q, "Content-Type", "application/json; "
		                                  "charset=utf8");
		WEB_PutS(q, "{\"lang\": \"");
		WEB_PutS(q, q->lang);
		WEB_PutS(q, "\",");

		if (cmd->fn(q) == 0) {
			WEB_PutS(q, "\"code\": 0}\r\n");
		} else {
			WEB_PutS(q, "\"code\": -1,");
			WEB_PutJSON_NoHTML_S(q, "error", AG_GetError());
			WEB_PutS(q, "\"backend_version\": \"" VERSION "\"}\r\n");
			WEB_LogErr("/%s: %s", op, AG_GetError());
		}
	} else {
		if (cmd->type != NULL) {
			if (strcmp(cmd->type, "text/html")==0) {
				WEB_SetHeaderS(q, "Content-Type", "text/html; "
				                                  "charset=utf8");
				if (cmd->fn(q) != 0)
					WEB_OutputError(q, AG_GetError());
			} else {
				WEB_SetHeaderS(q, "Content-Type", cmd->type);
				if (cmd->fn(q) != 0)
					WEB_LogErr("/%s: %s", op, AG_GetError());
			}
		} else {
			if (cmd->fn(q) != 0)
				WEB_LogErr("/%s: %s", op, AG_GetError());
		}
	}
}

/* Execute a module query (in Worker process). */
int
WEB_ExecWorkerQuery(WEB_Query *q)
{
	const char *op, *c;
	const WEB_Argument *opArg;
	WEB_ModuleClass *Cmod = NULL;
	WEB_Command *cmd;
	int i;

	if ((opArg = WEB_GetArgument(q, "op")) != NULL) {
		if (opArg->len >= WEB_OPNAME_MAX) {
			WEB_SetCode(q, "413 Request too large");
			goto fail;
		}
		op = opArg->value;
		for (c = op; *c != '\0'; c++)
			if (!isalnum(*c) && !strchr("_-.", *c)) {
				WEB_SetCode(q, "404 Not Found");
				goto fail;
			}
	} else {
		op = webHomeOp;
	}

	if (strcmp(op, "logout") == 0) {
		WEB_ClearQuery(q);
		WEB_CloseSession(q->sess);
		return (1);
	}
	for (i = 0; i < webModuleCount; i++) {
		q->mod = webModules[i];
		Cmod = (WEB_ModuleClass *)OBJECT(q->mod)->cls;
		if (strncmp(Cmod->name, op, strlen(Cmod->name)) == 0)
			break;
	}
	if (i == webModuleCount) {
		WEB_SetCode(q, "404 Not Found");
		goto fail;
	}
	AG_SetErrorS("Missing argument");	/* Fallback error message */

	for (cmd = Cmod->commands; cmd->name != NULL; cmd++) {
		if (strcmp(cmd->name, op) == 0) {
			WEB_CommandExec(q, op, cmd);
			break;
		}
	}
	if (cmd->name == NULL) {
		WEB_SetCode(q, "404 Not Found");
		goto fail;
	}
	WEB_FlushQuery(q);
	return (0);
fail:
	WEB_SetHeaderS(q, "Content-Language", "en");
	WEB_SetHeaderS(q, "Cache-Control", "no-cache, no-store, must-revalidate");
	WEB_SetHeaderS(q, "Expires", "0");
	WEB_OutputError(q, "The requested URL was not found on this server");
	WEB_FlushQuery(q);
	return (0);
}

/* Update the value of an existing header (less common case). */
void
WEB_EditHeader(WEB_Query *q, char *cLine, const char *value)
{
	char *cSep, *cEnd;
	size_t oldLen, newLen;

	WEB_LogDebug("EditHeader(%s => [%s])", cLine, value);
	WEB_LogDebug("EditHeader BEFORE=[%s]", q->head);

	if ((cSep = strchr(cLine, ':')) == NULL || cSep[1] != ' ' ||
	    (cEnd = strchr(&cSep[1], '\r')) == NULL) {
		AG_FatalError("Bad header");
	}
	cSep++;
	oldLen = cEnd-cSep+1;
	newLen = strlen(value);
	if ((q->headLen - oldLen + newLen) >= sizeof(q->head)) {
		AG_FatalError("Too big");
	}
	if (newLen != oldLen && (q->headLen - oldLen) > 0) {
		memmove(&cSep[1+newLen], &cSep[1+oldLen-2],
			&q->head[q->headLen] - &cSep[1+oldLen-2] + 1);
	}
	memcpy(&cSep[1], value, newLen);
	cSep[newLen+1] = '\r';
	cSep[newLen+2] = '\n';
	if (newLen != oldLen) {
		WEB_UpdateHeaderLines(q);
	}
	WEB_LogDebug("EditHeader AFTER=[%s]", q->head);
}

/* Return 1 if an argument exists and is nonempty. */
int
WEB_GetBool(WEB_Query *q, const char *key)
{
	WEB_Argument *arg;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		return (0);
	}
	return (*arg->value != '\0');
}

/* Validate an integer argument and return its value into pRet. */
int
WEB_GetInt(WEB_Query *q, const char *key, int *pRet)
{
	WEB_Argument *arg;
	char *ep;
	long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing Argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtol(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if ((errno == ERANGE) &&
	    ((rv == LONG_MAX || rv == LONG_MIN) ||
	     (rv > INT_MAX || rv < INT_MIN))) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = (int)rv;
	return (0);
}
int
WEB_GetUint(WEB_Query *q, const char *key, Uint *pRet)
{
	WEB_Argument *arg;
	char *ep;
	unsigned long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtoul(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (errno == ERANGE && rv == UINT_MAX) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = (Uint)rv;
	return (0);
}
int
WEB_GetUint64(WEB_Query *q, const char *key, Uint64 *pRet)
{
	WEB_Argument *arg;
	char *ep;
	unsigned long long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtoull(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (errno == ERANGE && rv == ULLONG_MAX) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = (Uint64)rv;
	return (0);
}
int
WEB_GetSint64(WEB_Query *q, const char *key, Sint64 *pRet)
{
	WEB_Argument *arg;
	char *ep;
	long long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtoll(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (errno == ERANGE && rv == LLONG_MAX) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = (Sint64)rv;
	return (0);
}

/* Version of WEB_GetInt() with range-checking. */
int
WEB_GetIntR(WEB_Query *q, const char *key, int *pRet, int min, int max)
{
	WEB_Argument *arg;
	char *ep;
	long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtol(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if ((errno == ERANGE) && ((rv == LONG_MAX || rv == LONG_MIN) ||
	     (rv > INT_MAX || rv < INT_MIN))) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	if (rv < min || rv > max) {
		AG_SetError("%s: Out of range (%d-%d)", key, min, max);
		return (-1);
	}
	*pRet = (int)rv;
	return (0);
}
int
WEB_GetUintR(WEB_Query *q, const char *key, Uint *pRet, Uint min, Uint max)
{
	WEB_Argument *arg;
	char *ep;
	unsigned long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtoul(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if ((errno == ERANGE) && ((rv == UINT_MAX) || (rv > UINT_MAX))) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	if (rv < min || rv > max) {
		AG_SetError("%s: Out of range (%d-%d)", key, min, max);
		return (-1);
	}
	*pRet = (Uint)rv;
	return (0);
}

/*
 * Obtain a range bounded by two integers (or a single number, in which case
 * set pMin=pMax).
 */
int
WEB_GetIntRange(WEB_Query *q, const char *key, int *pMin, const char *sep,
    int *pMax)
{
	char buf[WEB_INT_RANGE_MAX];
	char *val, *c, *rangeFrom, *rangeTo, *t;
	WEB_Argument *arg;
	int nSeps;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	val = arg->value;

	if (strpbrk(val, sep)) {				/* Is a range */
		for (c=val, nSeps=0; *c != '\0'; c++) {
			if (!isdigit(*c) && !isspace(*c) && !strchr(sep,*c)) {
				break;
			}
			if (strchr(sep,*c) && ++nSeps > 1) { break; }
		}
		if (*c != '\0') {
			AG_SetError("%s: Invalid range (near \"%s\")", key, c);
			return (-1);
		}
		Strlcpy(buf, val, sizeof(buf));
		t = buf;
		if ((rangeFrom = strsep(&t,sep)) && *rangeFrom != '\0' &&
		    (rangeTo = strsep(&t,sep)) && *rangeTo != '\0') {
			*pMin = (int)strtol(rangeFrom, NULL, 10);
			*pMax = (int)strtol(rangeTo, NULL, 10);
		}
	} else {
		*pMin = *pMax = (int)strtol(val, NULL, 10);
	}
	if (*pMin > *pMax) {
		AG_SetError("%s: Invalid range (%d > %d)", key, *pMin, *pMax);
		return (-1);
	}
	return (0);
}

/* Version of WEB_GetIntR() with range-checking and min=0. */
int
WEB_GetEnum(WEB_Query *q, const char *key, Uint *pRet, Uint last)
{
	WEB_Argument *arg;
	char *ep;
	long rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtol(arg->value, &ep, 10);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (((errno == ERANGE) && ((rv == LONG_MAX || rv == LONG_MIN))) ||
	    rv > INT_MAX || rv < 0 || rv >= last) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = (Uint)rv;
	return (0);
}

/* Validate a floating-point argument and return its value into pRet. */
int
WEB_GetFloat(WEB_Query *q, const char *key, float *pRet)
{
	WEB_Argument *arg;
	char *ep;
	float rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtof(arg->value, &ep);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (errno == ERANGE) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = rv;
	return (0);
}

/* Validate a double-precision argument and return its value into pRet. */
int
WEB_GetDouble(WEB_Query *q, const char *key, double *pRet)
{
	WEB_Argument *arg;
	char *ep;
	double rv;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (-1);
	}
	errno = 0;
	rv = strtod(arg->value, &ep);
	if (arg->value[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: Not a number", key);
		return (-1);
	}
	if (errno == ERANGE) {
		AG_SetError("%s: Out of range", key);
		return (-1);
	}
	*pRet = rv;
	return (0);
}

/*
 * Modify the value associated with a web argument. If the argument does
 * not exist, create it.
 */
void
WEB_SetS(WEB_Query *q, const char *key, const char *val)
{
	WEB_Argument *arg;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		arg = Malloc(sizeof(WEB_Argument));
		arg->type = WEB_GET_ARGUMENT;
		arg->contentType[0] = '\0';
		Strlcpy(arg->key, key, sizeof(arg->key));
		TAILQ_INSERT_TAIL(&q->args, arg, args);
		q->nArgs++;
	} else {
		if (arg->value != NULL)
			free(arg->value);
	}
	arg->value = (val != NULL) ? Strdup(val) : Strdup("");
	arg->len = strlen(val)+1;
}

/*
 * Modify the value associated with a web argument. If the argument does
 * not exist, create it.
 */
void
WEB_Set(WEB_Query *q, const char *key, const char *fmt, ...)
{
	WEB_Argument *arg;
	va_list ap;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		arg = Malloc(sizeof(WEB_Argument));
		arg->type = WEB_GET_ARGUMENT;
		arg->contentType[0] = '\0';
		Strlcpy(arg->key, key, sizeof(arg->key));
		TAILQ_INSERT_TAIL(&q->args, arg, args);
		q->nArgs++;
	} else {
		if (arg->value != NULL)
			free(arg->value);
	}
	if (fmt != NULL) {
		va_start(ap, fmt);
		if (vasprintf(&arg->value, fmt, ap) == -1) {
			arg->value = NULL;
			arg->len = 0;
		} else {
			arg->len = strlen(arg->value)+1;
		}
		va_end(ap);
	} else {
		arg->value = NULL;
		arg->len = 0;
	}
}

/* Delete the given web argument if it exists. */
int
WEB_Unset(WEB_Query *q, const char *key)
{
	WEB_Argument *arg;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		return (-1);
	}
	TAILQ_REMOVE(&q->args, arg, args);
	q->nArgs--;

	free(arg->value);
	free(arg);
	return (0);
}

/*
 * Get a pointer to a C string argument. Return NULL if the named
 * argument is undefined or exceeds len-1 bytes.
 */
const char *
WEB_Get(WEB_Query *q, const char *key, AG_Size len)
{
	WEB_Argument *arg;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (NULL);
	}
	if (arg->len > len) {
		AG_SetError("Argument too long \"%s\" (exceeds %lu bytes)",
		    key, (unsigned long)len);
		return (NULL);
	}
	return (arg->value);
}

/*
 * Variant of WEB_Get() which trims leading and ending whitespaces off
 * the value of the argument. Size checking is performed on the trimmed string.
 */
const char *
WEB_GetTrim(WEB_Query *q, const char *key, AG_Size len)
{
	WEB_Argument *arg;
	char *s, *end;

	TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	if (arg == NULL || (s = arg->value) == NULL) {
		AG_SetError("Missing argument \"%s\"", key);
		return (NULL);
	}
	WEB_TRIM_WHITESPACE(s, end);
	if (strlen(s) >= len) {
		AG_SetError("Argument too long \"%s\" (exceeds %lu bytes)",
		    key, (unsigned long)len);
		return (NULL);
	}
	return (s);
}

void
WEB_PutJSON(WEB_Query *q, const char *key, const char *fmt, ...)
{
	char *val;
	va_list ap;
	
	va_start(ap, fmt);
	if (vasprintf(&val, fmt, ap) == -1) {
		va_end(ap);
		return;
	}
	va_end(ap);

	WEB_PutJSON_S(q, key, val);
	free(val);
}

/* Release the resources allocated by a WEB query and check for signals. */
void
WEB_QueryDestroy(WEB_Query *q)
{
	WEB_Argument *arg, *narg;
	WEB_Cookie *c, *nc;

	for (c = TAILQ_FIRST(&q->cookies);
	     c != TAILQ_END(&q->cookies);
	     c = nc) {
		nc = TAILQ_NEXT(c, cookies);
		free(c);
	}
	for (arg = TAILQ_FIRST(&q->args);
	     arg != TAILQ_END(&q->args);
	     arg = narg) {
		narg = TAILQ_NEXT(arg, args);
		free(arg->value);
		free(arg);
	}
	Free(q->data);
}

/* Serialize WEB_Query data (+ 32-bit data offset) to fd. */
int
WEB_QuerySave(int fd, const WEB_Query *q)
{
	char buf[WEB_QUERY_MAX];	
	AG_DataSource *ds;
	AG_CoreSource *cs;
	WEB_Argument *arg;
	WEB_Cookie *ck;
	Uint32 length;
	Uint i;

	if (!(ds = AG_OpenCore(&buf[4], sizeof(buf)-4))) /* Skip data offset */
		return (-1);

	AG_WriteUint8(ds, (Uint8)q->method);
	AG_WriteUint8(ds, (Uint8)q->flags);
	AG_WriteString(ds, q->date);
	AG_WriteString(ds, q->userIP);
	AG_WriteString(ds, q->userHost);
	AG_WriteString(ds, q->userAgent);

	AG_WriteUint8(ds, (Uint8)q->nAcceptLangs);
	for (i = 0; i < q->nAcceptLangs; i++) {
		AG_WriteString(ds, q->acceptLangs[i]);
	}
	AG_WriteUint32(ds, (Uint32)q->nArgs);
	TAILQ_FOREACH(arg, &q->args, args) {
		AG_WriteUint8(ds, (Uint8)arg->type);
		AG_WriteString(ds, arg->contentType);
		AG_WriteString(ds, arg->key);
		AG_WriteUint32(ds, (Uint32)arg->len);
		AG_Write(ds, arg->value, arg->len);
	}
	AG_WriteUint32(ds, (Uint32)q->nCookies);
	TAILQ_FOREACH(ck, &q->cookies, cookies) {
		AG_WriteString(ds, ck->name);
		AG_WriteString(ds, ck->value);
		AG_WriteUint32(ds, ck->flags);
	}
	AG_WriteString(ds, q->contentType);
	AG_WriteUint32(ds, (Uint32)q->contentLength);

	cs = AG_CORE_SOURCE(ds);
	length = cs->offs;
	memcpy(&buf[0], &length, sizeof(Uint32));	/* Write data offset */

	if (WEB_SYS_Write(fd, buf, length+4) == -1) {
		if (errno == EPIPE) {
			AG_SetErrorS("EPIPE");
		} else {
			AG_SetErrorS(strerror(errno));
		}
		goto fail;
	}
	AG_CloseCore(ds);
	return (0);
fail:
	AG_CloseCore(ds);
	return (-1);
}

/* Read serialized WEB_Query data. */
int
WEB_QueryLoad(WEB_Query *q, const void *data, AG_Size dataLen)
{
	AG_DataSource *ds;
	Uint count;
	int i;

	if ((ds = AG_OpenConstCore(data, dataLen)) == NULL) {
		return (-1);
	}
	if ((q->method = (WEB_Method)AG_ReadUint8(ds)) >= WEB_METHOD_LAST) {
		AG_SetErrorS("Bad method");
		goto fail;
	}
	q->flags = (Uint)AG_ReadUint8(ds);
	AG_CopyString(q->date, ds, sizeof(q->date));
	AG_CopyString(q->userIP, ds, sizeof(q->userIP));
	AG_CopyString(q->userHost, ds, sizeof(q->userHost));
	AG_CopyString(q->userAgent, ds, sizeof(q->userAgent));

	if ((count = AG_ReadUint8(ds)) >= WEB_LANGS_MAX) {	/* Languages */
		AG_SetErrorS("Too many languages");
		goto fail;
	}
	for (i=0, q->nAcceptLangs=0; i < count; i++) {
		if (AG_CopyString(q->acceptLangs[i], ds, WEB_LANG_CODE_MAX) == -1) {
			goto fail;
		}
		q->nAcceptLangs++;
	}

	if ((count = AG_ReadUint32(ds)) > WEB_MAX_ARGS) {	/* Arguments */
		AG_SetErrorS("Too many args");
		goto fail;
	}
	for (i = 0; i < count; i++) {
		WEB_Argument *arg;
		
		if ((arg = TryMalloc(sizeof(WEB_Argument))) == NULL) {
			goto fail;
		}
		arg->type = (enum web_argument_type)AG_ReadUint8(ds);
		if (AG_CopyString(arg->contentType, ds, sizeof(arg->contentType)) == -1 ||
		    AG_CopyString(arg->key, ds, sizeof(arg->key)) == -1) {
			free(arg);
			goto fail;
		}
		if ((arg->len = AG_ReadUint32(ds)) > WEB_ARG_LENGTH_MAX ||
		    (arg->value = TryMalloc(arg->len)) == NULL) {
			AG_SetError("%s: Too big", arg->key);
			free(arg);
			goto fail;
		}
		if (AG_Read(ds, arg->value, arg->len) == -1) {
			free(arg->value);
			free(arg);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&q->args, arg, args);
		q->nArgs++;
	}
	
	if ((count = AG_ReadUint32(ds)) > WEB_MAX_COOKIES) {	/* Cookies */
		AG_SetErrorS("Too many cookies");
		goto fail;
	}
	for (i = 0; i < count; i++) {
		WEB_Cookie *ck;
		Uint32 ckfl;
		
		if ((ck = TryMalloc(sizeof(WEB_Cookie))) == NULL) {
			goto fail;
		}
		if (AG_CopyString(ck->name, ds, sizeof(ck->name)) == -1 ||
		    AG_CopyString(ck->value, ds, sizeof(ck->value)) == -1 ||
		    AG_Read(ds, &ckfl, sizeof(ckfl)) == -1) {
			free(ck);
			goto fail;
		}
		ck->flags = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(ckfl) :
	                                                          AG_SwapLE32(ckfl);
		ck->expires[0] = '\0';
		ck->domain[0] = '\0';
		ck->path[0] = '\0';
		TAILQ_INSERT_TAIL(&q->cookies, ck, cookies);
		q->nCookies++;
	}

	/* Client-supplied content */
	AG_CopyString(q->contentType, ds, sizeof(q->contentType));
	q->contentLength = (AG_Size)AG_ReadUint32(ds);

	AG_CloseConstCore(ds);
	return (0);
fail:
	AG_CloseConstCore(ds);
	return (-1);
}

static void SigPIPE(int sigraised) { pipeFlag++; }
static void SigCHLD(int sigraised) { chldFlag++; }
static void SigTERM(int sigraised) { termFlag++; }

void
WEB_CheckSignals(void)
{
	if (pipeFlag) {
		pipeFlag = 0;
		if (webWorkerUser[0] != '\0')		/* Fatal in Worker */
			WEB_Exit(1, "SIGPIPE");
	}
	if (termFlag) {
		WEB_Exit(0, "SIGTERM");
	}
	if (chldFlag) {
		pid_t pid;
		int status;

		chldFlag = 0;
		do {
			WEB_ControlCmd cmd;
			int i, rv;

			if ((pid = waitpid(WAIT_ANY, &status, WNOHANG)) <= 0) {
				continue;
			}
			/*
			 * Notify all Frontend processes to preemptively close
			 * any pipes associated with this process.
			 */
			bzero(&cmd, sizeof(cmd));
			cmd.type = WEB_CONTROL_WORKER_CHLD;
			cmd.data.workerQuit.pid = pid;
			cmd.data.workerQuit.status = status;
			for (i = 1; ; i++) {
				if (i == webClusterID) {	/* Self */
					continue;
				}
				if ((rv = WEB_ControlCommand(i, &cmd)) == 1) {
					break;
				} else if (rv == -1) {
					WEB_LogErr("SIGCHLD: %s", AG_GetError());
					continue;
				}
			}
#if 0
			if (WIFSIGNALED(status)) {
				WEB_LogNotice("Subprocess %d terminated "
				              "(signal %d)", pid,
					      WTERMSIG(status));
			} else if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					WEB_LogNotice("Subprocess %d terminated "
					              "(exit %d)", pid,
						      WEXITSTATUS(status));
				}
			} else {
				WEB_LogNotice("Subprocess %d terminated", pid);
			}
#endif
		} while (pid > 0 || (pid == -1 && errno == EINTR));
	}
}

/* Initialize the HTTP application server. */
void
WEB_Init(Uint clusterID, int eventSource)
{
	struct sigaction sa;

#ifdef AG_NAMESPACES
	AG_RegisterNamespace("Web", "WEB_", "http://libagar.org/");
#endif
	AG_RegisterClass(&webModuleClass);

	Strlcpy(webLogFile, agProgName, sizeof(webLogFile));
	Strlcat(webLogFile, ".log", sizeof(webLogFile));

	webFrontSocketCount = 0;
	webCtrlSock = -1;
	webQueryCount = 0;
	webClusterID = clusterID;
	webPeerAddress[0] = '\0';
	webEventSource = eventSource;
	
	TAILQ_INIT(&webVars);
	TAILQ_INIT(&webWorkSockets);
	SetGlobalS("_progname", agProgName);
	SetGlobal("WEB_USERNAME_MAX", "%d", WEB_USERNAME_MAX);
	SetGlobal("WEB_PASSWORD_MAX", "%d", WEB_PASSWORD_MAX);
	SetGlobal("WEB_EMAIL_MAX", "%d", WEB_EMAIL_MAX);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGURG, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigPIPE;
	sigaction(SIGPIPE, &sa, NULL);
	
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigCHLD;
	sigaction(SIGCHLD, &sa, NULL);

	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigTERM;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	webHomeOp[0] = '\0';
	webWorkerSess[0] = '\0';
	webWorkerUser[0] = '\0';
	
	if (mkdir(WEB_PATH_SESSIONS, 0700) == -1 && errno != EEXIST)
		WEB_Log(WEB_LOG_EMERG, "%s: %s", WEB_PATH_SESSIONS, AG_GetError());
}

void
WEB_Destroy(void)
{
	VAR *V, *Vnext;
	WEB_SessionSocket *sock, *sockNext;
	
	if (webDestroyFn != NULL)
		webDestroyFn();

	for (V = TAILQ_FIRST(&webVars);
	     V != TAILQ_END(&webVars);
	     V = Vnext) {
		Vnext = TAILQ_NEXT(V, vars);
		Free(V->value);
		Free(V);
	}
	for (sock = TAILQ_FIRST(&webWorkSockets);
	     sock != TAILQ_END(&webWorkSockets);
	     sock = sockNext) {
		sockNext = TAILQ_NEXT(sock, sockets);
		free(sock);
	}
	
/*	AG_UnregisterClass(&webModuleClass); */
#ifdef AG_NAMESPACES
/*	AG_UnregisterNamespace("WEB"); */
#endif
}

/* Set output log file path */
void
WEB_SetLogFile(const char *path)
{
	Strlcpy(webLogFile, path, sizeof(webLogFile));
}

/* Set callback routine for WEB_Log() */
void
WEB_SetLogFn(WEB_LogFn fn)
{
	webLogFn = fn;
}

/*
 * Clean up and terminate the running process gracefully.
 * Log an optional string before exiting.
 */
void
WEB_Exit(int excode, const char *fmt, ...)
{
	if (fmt != NULL) {
		char msg[256];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(msg, sizeof(msg), fmt, ap);
		va_end(ap);
		WEB_LogNotice("%s; exiting", msg);
	} else if (excode != 0) {
		WEB_LogNotice("exit code %d", excode);
	}

	WEB_Destroy();
	AG_Destroy();
	exit(excode);
}

#ifndef HAVE_SETPROCTITLE
void
WEB_SetProcTitle(const char *fmt, ...)
{
	return;
}
#endif /* !HAVE_SETPROCTITLE */

static __inline__ void
WEB_LogToFile(enum web_loglvl level, const char *_Nonnull s)
{
	char buf[WEB_ERROR_MAX], *p = buf;
	size_t hlen, alen, slen, rem;
	char pid[10];
	const char *c;
	FILE *f;

	snprintf(pid, sizeof(pid), "%d", getpid());
	alen = strlen(pid);
	slen = strlen(s);
	hlen = 1+webLogLvlNameLength+1+alen+2+1;

	if (hlen+slen >= sizeof(buf)) {
		slen = sizeof(buf) - hlen;		/* Truncate */
	}
	*p='[';							p++;
	memcpy(p, pid, alen);					p += alen;
	*p = ' ';						p++;
	memcpy(p, webLogLvlNames[level], webLogLvlNameLength);	p += webLogLvlNameLength;
	*p = ']';						p++;
	*p = ' ';						p++;
	rem = slen;
	for (c = s; *c != '\0'; c++) {
		*p = isprint(*c) ? *c : '*';
		p++;
		if (--rem == 0)
			break;
	}
	*p = '\n'; p++;
	
	if ((f = fopen(webLogFile, "a")) != NULL) {
		fwrite(buf, hlen+slen, 1, f);
		fclose(f);
	}
}

/*
 * Emit an error message to the error output. Format the message into a
 * fixed-size buffer in order to limit the size of log entries.
 */
void
WEB_Log(enum web_loglvl level, const char *fmt, ...)
{
	char msg[WEB_ERROR_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	
	if (webLogFn != NULL) {
		webLogFn(level, msg);
		return;
	}
	WEB_LogToFile(level, msg);
}
void
WEB_LogS(enum web_loglvl level, const char *s)
{
	if (webLogFn != NULL) {
		webLogFn(level, s);
		return;
	}
	WEB_LogToFile(level, s);
}

void
WEB_LogErr(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_ERR, buf);
}

void
WEB_LogWarn(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_WARNING, buf);
}

void
WEB_LogInfo(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_INFO, buf);
}

void
WEB_LogNotice(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_NOTICE, buf);
}

void
WEB_LogDebug(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_DEBUG, buf);
}

void
WEB_LogWorker(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_WORKER, buf);
}

void
WEB_LogEvent(const char *fmt, ...)
{
	char buf[WEB_ERROR_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	WEB_LogS(WEB_LOG_EVENT, buf);
}

/*
 * Write a formatted string to the standard output, assuming the formatted
 * text is consistent with the current output Content-Type.
 */
void
WEB_Printf(WEB_Query *q, const char *fmt, ...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	if (vasprintf(&buf, fmt, ap) == -1) {
		va_end(ap);
		return;
	}
	va_end(ap);

	WEB_Write(q, buf, strlen(buf));
	free(buf);
}

/* Make a message HTML-safe */
static __inline__ void
MessageToHTML(char *_Nonnull h, AG_Size hLen, const char *_Nonnull msg)
{
	const char *c;
	char *d;

	for (c = &msg[0], d = &h[0];
	    *c != '\0' && d < &h[hLen-5];
	     c++) {
		if      (*c == '<') { memcpy(d, "&lt;", 4); d+=4; }
		else if (*c == '>') { memcpy(d, "&gt;", 4); d+=4; }
		else {
			*d = *c;
			d++;
		}
	}
	*d = '\0';
}

/* Find the named HTML document and copy its absolute path into dst. */
static int
FindDoc(WEB_Query *_Nonnull q, const char *_Nonnull name,
    char *_Nonnull dst, AG_Size dst_len, AG_Offset *_Nonnull docLen)
{
	char path[FILENAME_MAX];
	struct stat sb;

	Strlcpy(path, "html/", sizeof(path)); 
	Strlcat(path, name, sizeof(path)); 
	if (stat(path, &sb) == 0 &&
	    Strlcpy(dst, path, dst_len) < dst_len) {
		*docLen = sb.st_size;
		return (0);
	}
	AG_SetError("Document not found: %s", name);
	return (-1);
}

/*
 * Write an HTML document to the query output, appling the chain of
 * input filters which are registered for the `text/html' content-type.
 *
 * TODO: Implement a small memory cache
 */
int
WEB_OutputHTML(WEB_Query *q, const char *name)
{
	char file[FILENAME_MAX];
	char path[FILENAME_MAX];
	AG_DataSource *ds;
	char *data;
	AG_Offset len;

	/* XXX inefficient */
	/* TODO: memory cache */
	Strlcpy(file, name, sizeof(file));
	Strlcat(file, ".html.", sizeof(file));
	Strlcat(file, q->lang, sizeof(file));
	if (FindDoc(q, file, path,sizeof(path), &len) == -1) {
		Strlcpy(file, name, sizeof(file));
		Strlcat(file, ".html.en", sizeof(file));
		if (FindDoc(q, file, path,sizeof(path), &len) == -1)
			goto fail;
	}
	if ((ds = AG_OpenFile(path, "r")) == NULL) {
		goto fail;
	}
	if ((data = TryMalloc(len)) == NULL) {
		AG_CloseFile(ds);
		goto fail;
	}
	if (AG_Read(ds, data, len) == -1) {
		AG_CloseFile(ds);
		free(data);
		goto fail;
	}
	AG_CloseFile(ds);

	/* Perform variable substitution and translation. Write to q->data. */
	WEB_VAR_FilterDocument(q, data, len);
	free(data);
	return (0);
fail:
	WEB_LogErr("WEB_OutputHTML: %s", AG_GetError());
	return (-1);
}

/*
 * Write an HTML fragment to query output in [json] mode.
 */
int
WEB_PutJSON_HTML(WEB_Query *q, const char *key, const char *name)
{
	char file[FILENAME_MAX];
	char path[FILENAME_MAX];
	AG_DataSource *ds;
	char *data;
	AG_Offset len;

	WEB_PutC(q, '"');
	WEB_PutS(q, key);
	WEB_PutS(q, "\": \"");
	
	/* XXX inefficient */

	Strlcpy(file, name, sizeof(file));
	Strlcat(file, ".html.", sizeof(file));
	Strlcat(file, q->lang, sizeof(file));
	if (FindDoc(q, file, path,sizeof(path), &len) == -1) {
		Strlcpy(file, name, sizeof(file));
		Strlcat(file, ".html.en", sizeof(file));
		if (FindDoc(q, file, path,sizeof(path), &len) == -1)
			goto fail_open;
	}
	
	if ((ds = AG_OpenFile(path, "r")) == NULL) {
		goto fail_open;
	}
	if ((data = TryMalloc(len)) == NULL ||
	    AG_Read(ds, data, len) == -1) {
		goto fail;
	}
	AG_CloseFile(ds);
	
	/* Perform variable substitution and translation. */
	WEB_VAR_FilterFragment(q, data, len);

	free(data);
	WEB_PutS(q, "\",");
	return (0);
fail:
	AG_CloseFile(ds);
	free(data);
fail_open:
	WEB_Log(WEB_LOG_CRIT, "WEB_PutJSON_HTML: %s", AG_GetError());
	WEB_PutS(q, AG_GetError());
	WEB_PutS(q, "\",");
	return (-1);
}

/* Write a formatted HTML error to the standard output. */
void
WEB_OutputError(WEB_Query *q, const char *msg)
{
	char htmlMsg[WEB_ERROR_MAX];

	MessageToHTML(htmlMsg, sizeof(htmlMsg), msg);
	Set("_error",
	    "<div class='alert alert-danger' role='alert'>"
	    "%s: <strong>%s</strong></div>",
	    _("Error"), htmlMsg);
	WEB_OutputHTML(q, "error");
}

/* Set a dismissible error message. */
void
WEB_SetError(const char *fmt, ...)
{
	char errMsg[WEB_ERROR_MAX], htmlMsg[WEB_ERROR_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(errMsg, sizeof(errMsg), fmt, ap);
	va_end(ap);
	MessageToHTML(htmlMsg, sizeof(htmlMsg), errMsg);
	Set("_error",
	    "<center><div class='alert alert-danger alert-dismissible' "
	     "role='alert'>"
	      "%s: <strong>%s</strong>"
	      " <a class='close' data-dismiss='alert' title='%s'>&times;</a>"
	    "</div></center>",
	    _("Error"), htmlMsg, _("Dismiss"));
}
void
WEB_SetErrorS(const char *msg)
{
	char htmlMsg[WEB_ERROR_MAX];

	MessageToHTML(htmlMsg, sizeof(htmlMsg), msg);
	Set("_error",
	    "<center><div class='alert alert-danger alert-dismissible' "
	     "role='alert'>"
	      "%s: <strong>%s</strong>"
	      " <a class='close' data-dismiss='alert' title='%s'>&times;</a>"
	    "</div></center>",
	    _("Error"), htmlMsg, _("Dismiss"));
}

/* Set a dismissible success message. */
void
WEB_SetSuccess(const char *fmt, ...)
{
	char msg[WEB_ERROR_MAX], htmlMsg[WEB_ERROR_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	MessageToHTML(htmlMsg, sizeof(htmlMsg), msg);
	Set("_error",
	    "<div class='alert alert-success alert-dismissible' role='alert'>"
	      "<span class='sr-only'>%s: </span>%s"
	      " <a class='close' data-dismiss='alert' title='%s'>&times;</a>"
	    "</div>",
	     _("Success: "), htmlMsg, _("Dismiss"));
}

/* Return 1 if session ID is a valid, running session */
static __inline__ int
ValidSessionID(const char *_Nonnull sessID)
{
	char path[FILENAME_MAX];
	struct stat sb;
	const char *c;
	int n;

	if (sessID[0] == '\0') {
		goto fail;
	}
	for (c = &sessID[0], n = 0;
	    *c != '\0';
	    c++, n++) {
		if (n+1 >= WEB_SESSID_MAX || !isdigit(*c))
			goto fail;
	}
	Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
	Strlcat(path, sessID, sizeof(path));
	if (stat(path, &sb) != 0) {
		goto fail;
	}
	return (1);
fail:
	AG_SetErrorS("Session has expired");
	return (0);
}

/* Send a KILL message to an active push event listener */
static int
KillListener(struct sockaddr_un *_Nonnull sun, socklen_t sunLen)
{
	int fd, status;

	WEB_LogWarn("Sending KILL to Listener: %s", sun->sun_path);

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		AG_SetError("socket: %s", strerror(errno));
		return (-1);
	}
try_connect:
	if (connect(fd, (struct sockaddr *)sun, sunLen) == -1) {
		if (errno == EINTR || errno == EAGAIN) {
			WEB_CheckSignals();
			goto try_connect;
		} else if (errno == ECONNREFUSED || errno == ENOENT) {
			WEB_LogWarn("KillListener: %s; assume process "
			            "exited already.", strerror(errno));
			unlink(sun->sun_path);
			close(fd);
			return (0);
		} else {
			AG_SetErrorS(strerror(errno));
			close(fd);
			return (-1);
		}
	}
	if (WEB_SYS_Write(fd, webKillEvent, strlen(webKillEvent)) == -1 ||
	    WEB_SYS_Read(fd, &status, sizeof(int)) == -1) {
		goto fail;
	}
	WEB_LogDebug("Kill packet returned = %d", status);
	close(fd);
	return (status);
fail:
	WEB_LogErr("KillListener: %s", strerror(errno));
	close(fd);
	return (-1);
}

/*
 * Post an Event to the specified destination.
 *
 * Valid destination types include:
 *
 *	NULL		Send according to filterFn return value.
 * 	"*"		Broadcast to all active event sources.
 *	"L=xx"		Send to all sessions in given language.
 *	"S=ID"		Send to a specific session ID.
 *	"username"	Send to all running sessions by a given user.
 */
int
WEB_PostEventS(const char *match, WEB_EventFilterFn filterFn,
    const void *filterFnArg, const char *type, const char *data)
{
	char msgBuf[WEB_EVENT_MAX];
	size_t msgBufLen;
	struct sockaddr_un sun;
	socklen_t sunLen;
	struct dirent *dent;
	DIR *dir;

	msgBufLen = snprintf(msgBuf, sizeof(msgBuf),
	    "type: %s\n"
	    "data: %s\n"
	    "\n", type, data);
	if (msgBufLen >= sizeof(msgBuf)) {
		AG_SetErrorS("Too big");
		return (-1);
	}

	sun.sun_family = AF_UNIX;

	if ((dir = opendir(WEB_PATH_EVENTS)) == NULL) {
		AG_SetError("%s: %s", WEB_PATH_EVENTS, strerror(errno));
		return (-1);
	}
	while ((dent = readdir(dir)) != NULL) {
		char name[WEB_SESSID_MAX+1+WEB_USERNAME_MAX+1];
		char *pName=name, *pSessID, *pUser, *pLang;
		int sock;

		Strlcpy(name, dent->d_name, sizeof(name));
		if (!(pSessID = Strsep(&pName, ":")) ||
		    !(pUser = Strsep(&pName, ":")) ||
		    !(pLang = Strsep(&pName, ":"))) {
			continue;
		}
		if (filterFn != NULL &&
		    filterFn(pSessID, pUser, pLang, filterFnArg) != 0) {
			continue;
		} else if (match != NULL) {
			if (match[0] == 'S' && match[1] == '=') {
				if (strcmp(&match[2], pSessID) != 0)
					continue;
			} else if (match[0] == 'L' && match[1] == '=') {
				if (strcmp(&match[2], pLang) != 0)
					continue;
			} else {
				if (strcmp(match, pUser) != 0)
					continue;
			}
		}

		Strlcpy(sun.sun_path, WEB_PATH_EVENTS, sizeof(sun.sun_path));
		Strlcat(sun.sun_path, dent->d_name, sizeof(sun.sun_path));
		sun.sun_len = strlen(sun.sun_path)+1;
		sunLen = sun.sun_len + sizeof(sun.sun_family);

		if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			AG_SetError("socket: %s", strerror(errno));
			return (-1);
		}
try_connect:
		if (connect(sock, (struct sockaddr *)&sun, sunLen) == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				goto try_connect;
			} else if (errno == ECONNREFUSED || errno == ENOENT) {
				WEB_LogWarn("PostEvent: %s; removing %s",
				    strerror(errno), sun.sun_path);
				unlink(sun.sun_path);
				close(sock);
				return (0);
			} else {
				AG_SetErrorS(strerror(errno));
				close(sock);
				return (-1);
			}
		}
		if (WEB_SYS_Write(sock, msgBuf, msgBufLen) == -1) {
			WEB_LogWarn("PostEvent: [%s]", strerror(errno));
		}
		close(sock);
	}
	closedir(dir);
	return (0);
}

int
WEB_PostEvent(const char *match, WEB_EventFilterFn filterFn,
    const void *filterFnArg, const char *type, const char *fmt, ...)
{
	char msg[WEB_EVENT_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	return WEB_PostEventS(match, filterFn, filterFnArg, type, msg);
}

static __inline__ void
CloseWorkSocket(WEB_SessionSocket *_Nonnull sock)
{
	WEB_LogDebug("CloseWorkSocket(%p, %d)", sock, sock->fd);
	if (sock->fd != -1) {
		close(sock->fd);
	}
	TAILQ_REMOVE(&webWorkSockets, sock, sockets);
	free(sock);
}

/* Process a control socket command. */
static int
WEB_HandleControlCmd(int ctrlSock)
{
	struct sockaddr paddr;
	socklen_t paddrLen = sizeof(paddr);
	WEB_ControlCmd cmd;
	WEB_SessionSocket *ss;
	int sock, status = 0;

try_accept:
	if ((sock = accept(ctrlSock, &paddr, &paddrLen)) == -1) {
		if (errno == EINTR) {
			WEB_CheckSignals();
			goto try_accept;
		} else {
			AG_SetError("accept: %s", strerror(errno));
			return (-1);
		}
	}
	if (WEB_SYS_Read(sock, &cmd, sizeof(cmd)) == -1) {
		goto fail;
	}
	switch (cmd.type) {
	case WEB_CONTROL_WORKER_CHLD:
		{
			WEB_SessionSocket *sock, *sockNext;
	
			/*
			 * Close all open sockets associated with the
			 * given Worker PID.
			 */
			for (sock = TAILQ_FIRST(&webWorkSockets);
			     sock != TAILQ_END(&webWorkSockets);
			     sock = sockNext) {
				sockNext = TAILQ_NEXT(sock, sockets);
				if (sock->workerPID == cmd.data.workerQuit.pid)
					CloseWorkSocket(sock);
			}
		}
		break;
	case WEB_CONTROL_SHUTDOWN:
		WEB_LogNotice("Cmd: SHUTDOWN");
		if (cmd.data.shutdown.reason[0] == '\0') {
			Strlcpy(cmd.data.shutdown.reason, "SHUTDOWN",
			    sizeof(cmd.data.shutdown.reason));
		}
		TAILQ_FOREACH(ss, &webWorkSockets, sockets) {
			if (!ss->workerIsMyChld) {
				continue;
			}
			WEB_LogNotice("Cmd: Worker %d is mine, killing it",
			    ss->workerPID);
			if (kill(ss->workerPID, SIGTERM) == -1)
				WEB_LogNotice("Worker TERM: %s", strerror(errno));
		}
		if (cmd.flags & WEB_CONTROL_CMD_SYNC) {
			if (WEB_SYS_Write(sock, &status, sizeof(status)) == -1)
				goto fail;
		}
		close(sock);
		WEB_Exit(1, cmd.data.shutdown.reason);
		/* NOTREACHED */
		break;
	case WEB_CONTROL_NOOP:
		WEB_LogNotice("Cmd: NOOP");
		break;
	default:
		AG_SetError("Cmd: Bad command %d", cmd.type);
		goto fail;
	}
	if (cmd.flags & WEB_CONTROL_CMD_SYNC) {
		if (WEB_SYS_Write(sock, &status, sizeof(status)) == -1)
			goto fail;
	}
	close(sock);
	return (0);
fail:
	status = -1;
	if (WEB_SYS_Write(sock, &status, sizeof(status)) == -1) {
		goto fail;
	}
	close(sock);
	return (-1);
}


/*
 * Listen for Push events from Worker processes and relay them
 * to the client as text/event-stream.
 */
static int
WEB_EventListener(WEB_Query *_Nonnull q, const WEB_SessionOps *_Nonnull Sops,
    const char *_Nonnull sessID, const char *_Nonnull username)
{
	int evSock, clntSock, status=0;
	struct sockaddr_un sun;
	Uint nEventsOrig = 0;
	socklen_t sunLen;
	struct stat sb;
	WEB_Session *S;
	int try;
		
	if (WEB_GetInt(q, "try", &try) == -1) {
		try = 0;
	} else {
		if (++try > WEB_EVENT_MAXRETRY) {
			AG_SetError("Event Listener is stuck");
			return (-1);
		}
	}

	sun.sun_family = AF_UNIX;
	Strlcpy(sun.sun_path, WEB_PATH_EVENTS, sizeof(sun.sun_path));
	Strlcat(sun.sun_path, sessID, sizeof(sun.sun_path));
	Strlcat(sun.sun_path, ":", sizeof(sun.sun_path));
	Strlcat(sun.sun_path, username, sizeof(sun.sun_path));
	Strlcat(sun.sun_path, ":", sizeof(sun.sun_path));
	Strlcat(sun.sun_path, q->lang, sizeof(sun.sun_path));
	sun.sun_len = strlen(sun.sun_path)+1;
	sunLen = sun.sun_len + sizeof(sun.sun_family);

	if (stat(sun.sun_path, &sb) == 0) {
		WEB_BeginFrontQuery(q, "events", Sops);
		if (KillListener(&sun, sunLen) == -1) {
			WEB_LogEvent("KillListener: %s", AG_GetError());
			return (-1);
		}
		WEB_ClearHeaders(q, "HTTP/1.0 302 Found\r\n");
		WEB_SetHeader(q, "Location", "/events?try=%d", try);
		WEB_SetHeaderS(q, "Retry-After", "1");
		WEB_SetHeaderS(q, "Cache-Control", "no-cache");
		WEB_SetHeaderS(q, "Expires", "0");
		WEB_SetHeaderS(q, "Content-Type", "text/plain");
		WEB_SetHeaderS(q, "Content-Length", "0");
		WEB_FlushQuery(q);
		return (0);
	}

	if ((evSock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		AG_SetError("socket: %s", strerror(errno));
		WEB_LogEvent("%s; aborting", AG_GetError());
		return (-1);
	}
	if (bind(evSock, (const struct sockaddr *)&sun, sunLen) == -1 ||
	    listen(evSock, 5) == -1) {
		AG_SetError("bind: %s", strerror(errno));
		goto fail;
	}
	chmod(sun.sun_path, 0700);

	WEB_SetHeaderS(q, "Content-Type", "text/event-stream");
	WEB_SetHeaderS(q, "Cache-Control", "no-cache");
	WEB_SetHeaderS(q, "Expires", "0");
#ifdef WEB_COMPAT_NGINX
	WEB_SetHeaderS(q, "X-Accel-Buffering", "no");
#endif
#ifdef WEB_CHUNKED_EVENTS
	WEB_SetHeaderS(q, "Transfer-Encoding", "chunked");
#endif
	WEB_SetHeaderS(q, "Connection", "close");
	q->flags &= ~(WEB_QUERY_KEEPALIVE);

	if (WEB_WriteHeaders(q->sock, q) == -1)
		goto fail;
	
	if ((S = TryMalloc(Sops->size)) == NULL) {
		goto fail;
	}
	WEB_SessionInit(S, Sops);
	if (WEB_SessionLoad(S, sessID) == -1) {
		goto fail_sess;
	}
	WEB_LogEvent("Attached to session %s (%s), nEvents=%u",
	    S->id, GetSV(S,"user"), S->nEvents);
	
	nEventsOrig = S->nEvents;
	WEB_SetProcTitle("events %s (%u)", username, S->nEvents);
	WEB_PostEvent(NULL, NULL, NULL, "message", "logged-in:%s", username);
	
	while (!termFlag) {
		char buf[WEB_EVENT_MAX];
#ifdef WEB_CHUNKED_EVENTS
		char   chunkHead[16];
		size_t chunkHeadLen;
#endif
		char   msgId[64];
		size_t msgIdLen;
		struct iovec msgv[4];
		fd_set rdFds;
		int maxFd = 0, iovcnt;
		struct timeval tv;
		Uint nRead = 0;
	
		tv.tv_usec = 0;
		tv.tv_sec = WEB_EVENT_PING_IVAL;

		FD_ZERO(&rdFds);
		FD_SET(evSock, &rdFds);
		FD_SET(q->sock, &rdFds);
		FD_SET(webCtrlSock, &rdFds);

		if (evSock > maxFd) { maxFd = evSock; }
		if (q->sock > maxFd) { maxFd = q->sock; }
		if (webCtrlSock > maxFd) { maxFd = webCtrlSock; }

		if (select(maxFd+1, &rdFds, NULL, NULL, &tv) < -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetError("select: %s", strerror(errno));
				goto fail_sess;
			}
		}
		
		/* Control socket event? */
		if (FD_ISSET(webCtrlSock, &rdFds)) {
			if (WEB_HandleControlCmd(webCtrlSock) == -1)
				WEB_LogErr("Control socket (in ev): %s", AG_GetError());
		}

		/* Client has closed connection? */
		if (FD_ISSET(q->sock, &rdFds)) {
			ssize_t rv;

			if ((rv = read(q->sock, NULL, 0)) == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					continue;
				} else {
					WEB_LogErr("EventSource Client: %s",
					    strerror(errno));
					goto fail_sess;
				}
			} else if (rv == 0) {
				WEB_LogEvent("EventSource: Client EOF");
				break;
			}
		}
		
		/* Incoming event on Unix socket? */
		if (FD_ISSET(evSock, &rdFds)) {
			struct sockaddr_un paddr;
			socklen_t paddrLen = sizeof(paddr);
			char *cEnd = NULL;

			clntSock = accept(evSock, (struct sockaddr *)&paddr, &paddrLen);
			if (clntSock == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					continue;
				} else {
					AG_SetError("accept: %s", strerror(errno));
					goto fail_sess;
				}
			}
			for (nRead=0; nRead < sizeof(buf); ) {
				ssize_t rv;

				/* TODO: WEB_EVENT_READ_TIMEOUT */
				rv = read(clntSock, &buf[nRead], sizeof(buf)-nRead);
				WEB_LogEvent("%ld-byte message", rv);
				if (rv == -1) {
					if (errno == EINTR || errno == EAGAIN) {
						WEB_CheckSignals();
						continue;
					} else {
						AG_SetError("Read error: %s",
						    strerror(errno));
						close(clntSock);
						goto fail_sess;
					}
				} else if (rv == 0) {
					WEB_LogEvent("EOF at %u", nRead);
					close(clntSock);
					break;
				}
				nRead += rv;
				if ((cEnd = memmem(buf, nRead, "\n\n",2)))
					break;
			}
			if (cEnd == NULL) {
				WEB_LogEvent("Bad event; ignoring %u bytes", nRead);
				close(clntSock);
				continue;
			}
			if (strncmp(buf, webKillEvent, strlen(webKillEvent)) == 0) {
				WEB_LogEvent("Got kill signal");
				goto killed;
			}
			msgIdLen = snprintf(msgId, sizeof(msgId),
			    "id: %u\n", S->nEvents++);
			if (msgIdLen >= sizeof(msgId)) {
				WEB_LogErr("Chunk oversize");
			    	close(clntSock);
				goto out;
			}
#ifdef WEB_CHUNKED_EVENTS
			chunkHeadLen = snprintf(chunkHead, sizeof(chunkHead),
			    "%lx\r\n", msgIdLen + (&cEnd[2]-buf));
		    	if (chunkHeadLen >= sizeof(chunkHead)) {
				WEB_LogErr("Chunk head oversize");
			    	close(clntSock);
				goto out;
			}
			msgv[0].iov_base = chunkHead;
			msgv[0].iov_len  = chunkHeadLen;
			msgv[1].iov_base = msgId;
			msgv[1].iov_len  = msgIdLen;
			msgv[2].iov_base = buf;
			msgv[2].iov_len  = &cEnd[2] - buf;
			msgv[3].iov_base = "\r\n";
			msgv[3].iov_len  = 2;
			iovcnt = 4;
#else /* !CHUNKED_EVENTS */
			msgv[0].iov_base = msgId;
			msgv[0].iov_len  = msgIdLen;
			msgv[1].iov_base = buf;
			msgv[1].iov_len  = &cEnd[2] - buf;
			iovcnt = 2;
#endif
try_write:
			if (writev(q->sock, msgv, iovcnt) == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					goto try_write;
				}
				WEB_LogErr("writev: %s", strerror(errno));
				close(clntSock);
				goto out;
			}
			close(clntSock);
			buf[nRead] = '\0';
		} else {
			msgIdLen = snprintf(msgId, sizeof(msgId),
			    "type: ping\n"
			    "id: %u\n",
			    S->nEvents++);
			if (msgIdLen >= sizeof(msgId)) {
				WEB_LogErr("Chunk oversize");
				goto out;
			}
			WEB_LogEvent("Ping: [%u]", S->nEvents);
#ifdef WEB_CHUNKED_EVENTS
			chunkHeadLen = snprintf(chunkHead, sizeof(chunkHead),
			    "%lx\r\n", msgIdLen);
			if (chunkHeadLen >= sizeof(chunkHead)) {
				WEB_LogErr("Chunk head oversize");
				goto out;
			}
			msgv[0].iov_base = chunkHead;
			msgv[0].iov_len  = chunkHeadLen;
			msgv[1].iov_base = msgId;
			msgv[1].iov_len  = msgIdLen;
			msgv[2].iov_base = "\r\n";
			msgv[2].iov_len  = 2;
			iovcnt = 3;
#else /* !CHUNKED_EVENTS */
			msgv[0].iov_base = msgId;
			msgv[0].iov_len  = msgIdLen;
			iovcnt = 1;
#endif
try_ping:
			if (writev(q->sock, msgv, iovcnt) == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					goto try_ping;
				}
				WEB_LogErr("Events: writev: %s", strerror(errno));
				goto out;
			}
		}
		WEB_SetProcTitle("events %s (%u+%u)", username, nEventsOrig,
		    (S->nEvents - nEventsOrig));
		WEB_CheckSignals();
	}
out:
	WEB_PostEvent(NULL, NULL, NULL, "message", "logged-out:%s:0", username);
#ifdef WEB_CHUNKED_EVENTS
	WEB_SYS_Write(q->sock, "0\r\n\r\n", 5);
#endif
	WEB_SessionSave(S);
	WEB_SessionFree(S);
	close(evSock);
	unlink(sun.sun_path);
	return (0);
killed:
	WEB_PostEvent(NULL, NULL, NULL, "message", "logged-out:%s:0", username);
#ifdef WEB_CHUNKED_EVENTS
	WEB_SYS_Write(q->sock, "0\r\n\r\n", 5);
#endif
	WEB_SessionSave(S);
	WEB_SessionFree(S);
	close(evSock);
	unlink(sun.sun_path);
	status = 0;
	WEB_SYS_Write(clntSock, &status, sizeof(int));
	close(clntSock);
	return (0);
fail_sess:
	WEB_PostEvent(NULL, NULL, NULL, "message", "logged-out:%s:1", username);
#ifdef WEB_CHUNKED_EVENTS
	WEB_SYS_Write(q->sock, "0\r\n\r\n", 5);
#endif
	WEB_SessionFree(S);
fail:
	WEB_LogEvent("EventListener: %s; disconnected", AG_GetError());
	close(evSock);
	unlink(sun.sun_path);
	return (-1);
}

/*
 * A session file exists but we could not connect() to a worker process.
 * Open the session file and extract the username / password from it, so
 * that we can re-authenticate automatically.
 */
static int
GetSessionCredentials(const WEB_SessionOps *_Nonnull Sops,
    const char *_Nonnull sessID,
    char *_Nonnull  user, size_t userSize,
    char *_Nullable pass, size_t passSize)
{
	const char *userArg, *passArg;
	WEB_Session *S;

	if ((S = TryMalloc(Sops->size)) == NULL) {
		return (-1);
	}
	WEB_SessionInit(S, Sops);
	if (WEB_SessionLoad(S, sessID) == -1) {
		goto fail;
	}
	if ((userArg = GetSV(S,"user")) == NULL ||
	    (pass && (passArg = GetSV(S,"pass")) == NULL)) {
		AG_SetErrorS("Bad session data");
		goto fail;
	}
	Strlcpy(user, userArg, userSize);
	if (pass) { Strlcpy(pass, passArg, passSize); }
	WEB_SessionFree(S);
	return (0);
fail:
	WEB_SessionFree(S);
	return (-1);
}

/*
 * Spawn a new worker process. Unless pre-fork authentication is used, the
 * worker is expected to perform authentication and return a new session ID
 * on success, or an error code on failure.
 */
static int
CreateWorker(const WEB_SessionOps *_Nonnull Sops,
    WEB_Query  *_Nonnull q,
    const char *_Nonnull user,
    const char *_Nonnull pass,
    char       *_Nonnull sessID, size_t sessIdSize, Uint nRestoreAttempts,
    pid_t      *_Nonnull pid)
{
	int pp[2];
	pid_t pidNew;

	if (pipe(pp) == -1) {
		AG_SetError("pipe: %s", strerror(errno));
		return (-1);
	}
	if ((pidNew = fork()) == -1) {
		AG_SetError("fork: %s", strerror(errno));
		close(pp[0]);
		close(pp[1]);
		return (-1);
	} else if (pidNew == 0) {				/* In worker */
		if (WEB_WorkerMain(Sops, q, user, pass, sessID, pp,
		    nRestoreAttempts) != 0) {
			WEB_LogErr("Worker(%d) Failed: %s", getpid(),
			    AG_GetError());
		}
		WEB_Exit(0, NULL);
	}
	*pid = pidNew;
	close(pp[1]);

	/*
	 * Read status response. Expect a session number on success,
	 * otherwise "AUTH" or "FAIL" padded up to (WEB_SESSID_MAX-1).
	 */
	if (WEB_SYS_Read(pp[0], sessID, WEB_SESSID_MAX-1) == -1) {
		AG_SetError("Worker Status: %s", strerror(errno));
		close(pp[0]);
		return (-1);
	}
	sessID[WEB_SESSID_MAX-1] = '\0';
	if (sessID[0] == 'A') {
		AG_SetErrorS(_("Username or password is invalid"));
		write(pp[0], "0", 1);
		close(pp[0]);
		return (-1);
	} else if (!ValidSessionID(sessID)) {
		AG_SetError("Worker process failed (%s)", sessID);
		close(pp[0]);
		return (-1);
	}
	close(pp[0]);
	return (0);
}

/* Create a new session-bound Frontend<->Worker socket. */
static WEB_SessionSocket *_Nullable
CreateWorkSocket(struct sockaddr_un *_Nonnull sun, const char *_Nonnull sessID)
{
	WEB_SessionSocket *sock;
	struct stat sb;

	WEB_LogDebug("CreateWorkSocket(%s)", sessID);

	/* Check for a saved session on disk. */
	Strlcpy(sun->sun_path, WEB_PATH_SESSIONS, sizeof(sun->sun_path));
	Strlcat(sun->sun_path, sessID, sizeof(sun->sun_path));
			
	if (stat(sun->sun_path, &sb) != 0) {
		AG_SetError("%s: %s", sun->sun_path, strerror(errno));
		return (NULL);
	}
	if ((sock = TryMalloc(sizeof(WEB_SessionSocket))) == NULL) {
		return (NULL);
	}
	Strlcpy(sock->sessID, sessID, sizeof(sock->sessID));
	sock->fd = -1;
	sock->workerPID = 0;
	sock->workerIsMyChld = 0;
	TAILQ_INSERT_TAIL(&webWorkSockets, sock, sockets);
	return (sock);
}

/* Standard data buffer refill operation. */
#undef REFILL_BUFFER_MIN
#define REFILL_BUFFER_MIN(len)						\
	rv = read(sock->fd, &buf[nRead], MIN((len), sizeof(buf) - nRead)); \
	if (rv == -1) {							\
		if (errno == EINTR || errno == EAGAIN) {		\
			WEB_CheckSignals();				\
			continue;					\
		} else {						\
			break;						\
		}							\
	} else if (rv == 0) {						\
		break;							\
	}

#undef RESPAWN_WORKER
#define RESPAWN_WORKER()						\
	CloseWorkSocket(sock);						\
	sock = NULL;							\
									\
	Strlcpy(sun.sun_path, WEB_PATH_SOCKETS, sizeof(sun.sun_path));	\
	Strlcat(sun.sun_path, sessID, sizeof(sun.sun_path));		\
	Strlcat(sun.sun_path, ".sock", sizeof(sun.sun_path));		\
	unlink(sun.sun_path);						\
									\
	if (nRestoreAttempts > 0) {					\
		AG_SetError("Restore failed (%s)", AG_GetError());	\
		goto fail_auth;						\
	}								\
	if (GetSessionCredentials(Sops, sessID, user,sizeof(user),	\
	    pass,sizeof(pass)) == -1) {					\
		goto fail_auth;						\
	}								\
	nRestoreAttempts++;						\
	WEB_LogNotice("Respawning worker (%s)", AG_GetError());		\
	goto spawn_worker

/*
 * Process a Query. Return 1 (keep connection alive) or 0 (close).
 */
int
WEB_ProcessQuery(WEB_Query *q, const WEB_SessionOps *Sops, void *rdBuf,
    AG_Size rdBufLen)
{
	char head[WEB_HTTP_HEADER_MAX], *cHeadEnd, *pHead = head, *s;
	char buf[WEB_DATA_BUFSIZE];
	char sessID[WEB_SESSID_MAX];
	char user[WEB_USERNAME_MAX];
	char pass[WEB_PASSWORD_MAX];
	const WEB_CommandPreAuth *cp;
	struct sockaddr_un sun;
	socklen_t sunLen;
	const char *op, *sessArg;
	int nRestoreAttempts=0, detChunked;
	AG_Size headLen, nRead, nWrote, detContentLen;
	WEB_SessionSocket *sock;
	ssize_t rv;
	
	sessID[0] = '\0';
	user[0] = '\0';
	pass[0] = '\0';
	sun.sun_path[0] = '\0';

	if (webEventSource) {				    /* Event source */
		op = "events";
	} else {						/* Frontend */
		if (!(op = WEB_Get(q, "op", WEB_OPNAME_MAX)))
			op = "";

		for (cp = &Sops->preAuthCmds[0]; cp->name != NULL; cp++) {
			if (strcmp(cp->name, op) == 0)
				break;
		}
		if (cp->name != NULL) {                      /* Pre-auth cmd */
			WEB_BeginFrontQuery(q, op, Sops);
			WEB_SetHeaderS(q, "Accept-Ranges", "none");
			WEB_SetHeaderS(q, "Vary", "Accept-Language,"
			                          "Accept-Encoding,"
			                          "User-Agent");
			WEB_SetHeaderS(q, "Last-Modified", q->date);
			WEB_SetHeaderS(q, "Cache-Control", "no-cache, no-store, "
			                                   "must-revalidate");
			WEB_SetHeaderS(q, "Expires", "0");
			if (cp->type != NULL) {
				WEB_SetHeaderS(q, "Content-Type", cp->type);
			}
			WEB_SetHeaderS(q, "Content-Language", q->lang);
			cp->fn(q);
			WEB_FlushQuery(q);
			return WEB_KeepAlive(q);
		} else {                                /* Public module cmd */
			WEB_Module *mod = NULL;
			WEB_ModuleClass *Cmod = NULL;
			WEB_Command *cmd;
			int i;

			for (i = 0; i < webModuleCount; i++) {
				mod = webModules[i];
				Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;
				if (strncmp(Cmod->name, op, strlen(Cmod->name)) == 0)
					break;
			}
			if (i < webModuleCount) {
				for (cmd = Cmod->commands; cmd->name != NULL;
				     cmd++) {
					if (strcmp(cmd->name, op) != 0) {
						continue;
					}
					if (strchr(cmd->flags, 'P') != NULL) {
						WEB_BeginFrontQuery(q, op, Sops);
						q->mod = mod;
						WEB_CommandExec(q, op, cmd);
						WEB_FlushQuery(q);
						return WEB_KeepAlive(q);
					}
#if 1
					else {
						WEB_SetCode(q, "403 Forbidden");
						WEB_SetHeaderS(q, "Content-Language", "en");
						WEB_SetHeaderS(q, "Cache-Control", "no-cache");
						WEB_SetHeaderS(q, "Expires", "0");
						WEB_OutputError(q, "Access denied");
						WEB_FlushQuery(q);
						return WEB_KeepAlive(q);
					}
#endif
				}
			}
		}
	}

	if ((sessArg = WEB_GetCookie(q, "sess")) != NULL &&
	     ValidSessionID(sessArg)) {
		TAILQ_FOREACH(sock, &webWorkSockets, sockets) {
			if (strcmp(sock->sessID, sessArg) == 0)
				break;
		}
		if (sock == NULL) {
			if ((sock = CreateWorkSocket(&sun, sessArg)) == NULL)
				goto fail_auth;
		}
	} else {
		sock = NULL;
	}
	if (sock == NULL) {				/* Authenticate */
		const char *userArg, *passArg;
		pid_t pid;

		if ((userArg = WEB_Get(q, "username", WEB_USERNAME_MAX)) == NULL ||
		    (passArg = WEB_Get(q, "password", WEB_PASSWORD_MAX)) == NULL) {
			goto show_auth;
		}
		if ((Sops->flags & WEB_SESSION_PREFORK_AUTH) &&
		    Sops->auth != NULL &&
		    Sops->auth(NULL, userArg, passArg) == -1) {
			goto fail_auth;
		}
		Strlcpy(user, userArg, sizeof(user));
		Strlcpy(pass, passArg, sizeof(pass));
spawn_worker:
		if (CreateWorker(Sops, q, user, pass, sessID, sizeof(sessID),
		    nRestoreAttempts, &pid) == -1) {
			goto fail_auth;
		}
		if ((sock = TryMalloc(sizeof(WEB_SessionSocket))) == NULL) {
			goto fail_auth;
		}
		Strlcpy(sock->sessID, sessID, sizeof(sock->sessID));
		sock->fd = -1;
		sock->workerPID = pid;
		sock->workerIsMyChld = 1;
		TAILQ_INSERT_TAIL(&webWorkSockets, sock, sockets);
		WEB_LogInfo(
		    "%s: Spawned Worker (%s, %s, pid %d, ip %s (%s), agent \"%s\")",
		    user, sessID, q->lang, (int)pid, q->userIP, q->userHost,
		    q->userAgent);
	} else {
		Strlcpy(sessID, sock->sessID, sizeof(sessID));
	}

#ifdef WEB_DEBUG_QUERIES
	WEB_Log(WEB_LOG_QUERY, "%s(%s): /%s", webMethods[q->method].name,
	    sessID, op);
#endif
	if (sock->fd == -1) {
		if ((sock->fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			AG_SetError("socket: %s", strerror(errno));
			goto fail_auth;
		}
try_connect:
		sun.sun_family = AF_UNIX;
		Strlcpy(sun.sun_path, WEB_PATH_SOCKETS, sizeof(sun.sun_path));
		Strlcat(sun.sun_path, sessID, sizeof(sun.sun_path));
		Strlcat(sun.sun_path, ".sock", sizeof(sun.sun_path));
		sun.sun_len = strlen(sun.sun_path)+1;
		sunLen = sun.sun_len + sizeof(sun.sun_family);
		if (connect(sock->fd, (struct sockaddr *)&sun, sunLen) == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				goto try_connect;
			} else if (errno == ECONNREFUSED || errno == ENOENT) {
				/*
				 * Restart the Worker process. If re-auth is
				 * necessary, extract user/pass from session
				 * data.
				 */
				AG_SetErrorS(strerror(errno));
				RESPAWN_WORKER();
			}
			AG_SetErrorS(strerror(errno));
			WEB_LogErr("connect(%s): %s", sessID, AG_GetError());
			goto fail_auth;
		}
		/*
		 * Expect the Worker to report a valid PID on connect.
		 */
		if (WEB_SYS_Read(sock->fd, &sock->workerPID, sizeof(pid_t)) == -1) {
			WEB_LogErr("Reading workerPID: %s", AG_GetError());
			goto fail_auth;
		}
		/* if (kill(sock->workerPID, 0) != 0) {
			WEB_LogWarn("Reported Worker PID: %s", strerror(errno));
		}
		WEB_LogDebug("Opened new fd: %d (got workerPID=%d)", sock->fd,
		    sock->workerPID); */
	} else {
		/* WEB_LogDebug("Reusing existing fd: %d (with workerPID=%d)",
		    sock->fd, sock->workerPID); */
	}
/*	WEB_LogDebug("op: [%s]", op); */

	if (strcmp(op, "events") == 0) {
		/*
		 * Become an event listener. Keep returning text/event-stream
		 * until connection is closed.
		 */
		if (GetSessionCredentials(Sops, sessID, user,sizeof(user), NULL,0) == -1) {
			goto fail_auth;
		}
		if (q->method != WEB_METHOD_GET) {
			WEB_SetHeaderS(q, "Content-Type", "text/event-stream");
			WEB_SetHeaderS(q, "Cache-Control", "no-cache");
			WEB_SetHeaderS(q, "Expires", "0");
#ifdef WEB_COMPAT_NGINX
			WEB_SetHeaderS(q, "X-Accel-Buffering", "no");
#endif
#ifdef WEB_CHUNKED_EVENTS
			WEB_SetHeaderS(q, "Transfer-Encoding", "chunked");
#endif
			WEB_SetHeaderS(q, "Connection", "close");
			q->flags &= ~(WEB_QUERY_KEEPALIVE);
			WEB_FlushQuery(q);
			return (0);			/* Force close */
		}
		if (WEB_EventListener(q, Sops, sessID, user) == -1) {
			WEB_LogEvent("EventListener: %s", AG_GetError());
			WEB_BeginFrontQuery(q, "events", Sops);
			WEB_SetCode(q, "503 Service Unavailable");
			WEB_SetHeaderS(q, "Content-Language", "en");
			WEB_SetHeaderS(q, "Cache-Control", "no-cache");
			WEB_SetHeaderS(q, "Expires", "0");
			WEB_OutputError(q, AG_GetError());
			WEB_FlushQuery(q);
		}
		q->flags &= ~(WEB_QUERY_KEEPALIVE);
		WEB_SetProcTitle(NULL);
		return (0);				/* Force close */
	}

	/* Send the serialized WEB_Query data to the Worker. */
	if (WEB_QuerySave(sock->fd, q) == -1) {
		if (strcmp(AG_GetError(), "EPIPE") == 0) {
			RESPAWN_WORKER();
		} else {
			goto fail_auth;
		}
	}

	if (strcmp(op, "logout") == 0) {
		/*
		 * The Worker process should exit immediately after having
		 * read the WEB_Query.
		 */
		WEB_LogNotice("Logged out, bye!");
		goto logout;
	}

	/*
	 * If the Frontend isn't done reading all data from the request,
	 * forward the rest of it to the Worker as-is.
	 */
	if (q->contentLength > 0 && !(q->flags & WEB_QUERY_CONTENT_READ)) {
		WEB_LogDebug("Feed worker %lu bytes of %s data (got %lu)",
		    (Ulong)q->contentLength, q->contentType, (Ulong)rdBufLen);

		if (rdBufLen > 0 && WEB_SYS_Write(sock->fd, rdBuf, rdBufLen) == -1) {
			WEB_LogErr("Write to worker: %s", AG_GetError());
			goto fail_data;
		}
#ifdef WEB_DEBUG_REQUESTS
		FILE *dbgOut;
		if ((dbgOut = fopen("debug-request.txt", "w")))
			fwrite(rdBuf, 1, rdBufLen, dbgOut);
#endif
		for (nRead = rdBufLen;
		     nRead < q->contentLength;
		     ) {
			rv = read(q->sock, buf,
			    MIN(sizeof(buf), (q->contentLength-nRead)));
			if (rv == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					continue;
				} else {
					AG_SetError("Forward: %s", strerror(errno));
					goto fail_data;
				}
			}
			nRead += rv;
			if (WEB_SYS_Write(sock->fd, buf, rv) == -1) {
				WEB_LogErr("Write to worker: %s", AG_GetError());
				goto fail_data;
			}
#ifdef WEB_DEBUG_REQUESTS
			if (dbgOut) { fwrite(buf, 1, rv, dbgOut); }
#endif
		}
#ifdef WEB_DEBUG_REQUESTS
		fclose(dbgOut);
#endif
	}

	/*
	 * Analyze the HTTP headers from the Worker response. Determine if
	 * chunked transfer encoding or a Content-Length is set.
	 */
	nRead = 0;
	do {
		fd_set rdFds;
		int maxFd = 0;
		struct timeval tv;

		tv.tv_sec = WEB_WORKER_RESP_TIMEOUT;
		tv.tv_usec = 0;
		FD_ZERO(&rdFds);
		FD_SET(sock->fd, &rdFds);
		if (sock->fd > maxFd) { maxFd = sock->fd; }
read_resp_head:
		if (select(maxFd+1, &rdFds, NULL, 0, &tv) < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				goto read_resp_head;
			} else {
				AG_SetError("select: %s", strerror(errno));
				goto fail_data;
			}
		}
		if (!FD_ISSET(sock->fd, &rdFds)) {
			AG_SetErrorS("Worker response timeout; contact admin!");
			goto fail_auth;
		}
		if ((rv = read(sock->fd, &head[nRead], sizeof(head)-nRead)) == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetError("Worker: %s (in header)", strerror(errno));
				goto fail_data;
			}
		} else if (rv == 0) {
			AG_SetErrorS("Worker: EOF");
			goto fail_data;
		}
		head[nRead+rv] = '\0';
		nRead += rv;
		cHeadEnd = memmem(head, nRead, "\r\n\r\n",4);
	} while (cHeadEnd == NULL);

	if (cHeadEnd == NULL) {
		AG_SetErrorS("Worker: Bad headers");
		goto fail_data;
	}
	headLen = (&cHeadEnd[4] - head);

	/* Write the unmodified HTTP headers back to Client. */
	WEB_SYS_Write(q->sock, head, headLen);

	/* Scan for a Transfer-Encoding or Content-Length */
	*cHeadEnd = '\0';
	detChunked = 0;
	detContentLen = 0;
	while ((s = strsep(&pHead,"\n")) != NULL) {
		char *cEnd;

		if ((cEnd = strchr(s,'\r')) != NULL) {
			*cEnd = '\0';
		}
		if (strcmp(s, "Transfer-Encoding: chunked")==0) {
			detChunked = 1;
		} else if (strncmp(s, "Content-Length: ",16)==0) {
			detContentLen = (AG_Size)strtol(&s[16], NULL, 10);
		}
	}
	
	if (nRead > headLen) {
		/*
		 * We've already read some data past the end of header;
		 * copy it to the top of the buffer.
		 */
		memcpy(buf, &head[headLen], nRead-headLen);
	}
	nRead -= headLen;			/* nRead now refers to buf */

	if (detChunked) {			/* Chunked transfer encoding */
		size_t chunk;
		Uint nChunk = 0;
#ifdef WEB_DEBUG_TRANSFER
		FILE *dbgOut = fopen("debug-chunked.txt", "w");
		WEB_LogDebug("READBACK: Chunked, nRead=%lu, headLen=%lu",
		    nRead, headLen);
#endif
		do {
			char chunkHead[16];

			s = NULL;
			for (;;) {
				if ((s = memmem(buf, nRead, "\r\n",2)) != NULL) {
					break;
				}
				REFILL_BUFFER_MIN(1);	/* Min. "0\r\n\r\n" */
				nRead += rv;
			}
			if (s == NULL) {
				AG_SetError("Worker: Bad chunk near \"%s\"", buf);
				goto fail_data;
			}
			memcpy(chunkHead, buf, s-buf);		/* Read-only */
			chunkHead[s-buf] = '\0';
			if (sscanf(chunkHead, "%lx", &chunk) != 1 ||
			    chunk > UINT_MAX) {
				AG_SetError("Worker: Bad chunk (\"%s\"), "
				            "buf=[%s]", chunkHead, buf);
				goto fail_data;
			}
#ifdef WEB_DEBUG_TRANSFER
			WEB_LogDebug(
			    "Chunk #%u [\"%s\"]: Forwarding %lu bytes (nRead=%lu)",
			    nChunk, chunkHead, chunk, nRead);
#endif
			/* Write Chunk Header */
			if (WEB_SYS_Write(q->sock, buf, nRead) == -1) {
				WEB_LogErr("Client Flush: %s", AG_GetError());
			}
#ifdef WEB_DEBUG_TRANSFER
			fwrite(buf, 1, nRead, dbgOut);
			fflush(dbgOut);
#endif
			nRead = 0;

			/* Write Chunk Data */
			nWrote = 0;
			while (nWrote < chunk+2) {
				AG_Size nRemain = chunk+2 - nWrote;

				if (nRead < MIN(nRemain, sizeof(buf))) {
					REFILL_BUFFER_MIN(nRemain);
					if (rv == 0) {
						WEB_LogErr("Chunk #%u EOF", nChunk);
						break;
					}
					nRead += rv;
				}
				if (WEB_SYS_Write(q->sock, buf, nRead) == -1) {
					WEB_LogErr("Chunk #%u write: %s", nChunk,
					    AG_GetError());
				}
				nWrote += nRead;
#ifdef WEB_DEBUG_TRANSFER
				fwrite(buf, 1, nRead, dbgOut);
				fflush(dbgOut);
#endif
				nRead = 0;
			}
			if (nWrote < chunk+2) {
				AG_SetError("Chunk #%u: EOF", nChunk);
				goto fail_data;
			}
			nChunk++;
		} while (chunk != 0);			/* Terminating chunk? */
#ifdef WEB_DEBUG_TRANSFER
		fclose(dbgOut);
#endif
	} else if (detContentLen > 0) {
#ifdef WEB_DEBUG_TRANSFER
		WEB_LogDebug("READBACK: Content-Length: %lu", detContentLen);
#endif
		nWrote = 0;
		while (nWrote < detContentLen) {
			AG_Size nRemain = detContentLen-nWrote;
			if (nRead < MIN(nRemain, sizeof(buf))) {
				REFILL_BUFFER_MIN(nRemain);
				if (rv == 0) {
					break;
				}
				nRead += rv;
			}
			if (WEB_SYS_Write(q->sock, buf, nRead) == -1) {
				WEB_LogErr("Content-Len Write: %s", AG_GetError());
			}
			nWrote += nRead;
			nRead = 0;
		}
		if (nWrote < detContentLen) {
			AG_SetError("EOF at %lu (Content-Length: %lu)",
			    (Ulong)nWrote, (Ulong)detContentLen);
			goto fail_data;
		}
	}
	return WEB_KeepAlive(q);
fail_auth:
	WEB_LogS(WEB_LOG_ERR, AG_GetError());
	WEB_BeginFrontQuery(q, "login", Sops);
	WEB_SetErrorS(AG_GetError());
	Sops->loginPage(q);
	WEB_FlushQuery(q);
	if (sock) { CloseWorkSocket(sock); }
	return WEB_KeepAlive(q);
show_auth:
	WEB_BeginFrontQuery(q, "login", Sops);
	Sops->loginPage(q);
	WEB_FlushQuery(q);
	if (sock) { CloseWorkSocket(sock); }
	return WEB_KeepAlive(q);
logout:
	WEB_BeginFrontQuery(q, "logout", Sops);
	WEB_SetHeaderS(q, "Connection", "close");
	WEB_SetHeaderS(q, "Set-Cookie", "sess=; Path=/");
	WEB_OutputHTML(q, "logout");
	WEB_FlushQuery(q);
	if (sock) { CloseWorkSocket(sock); }
	q->flags &= ~(WEB_QUERY_KEEPALIVE);
	return (0);				/* Force connection close */
fail_data:
	WEB_LogS(WEB_LOG_ERR, AG_GetError());
	WEB_SetHeaderS(q, "Vary", "Accept-Language,Accept-Encoding,User-Agent");
	WEB_SetHeaderS(q, "Last-Modified", q->date);
	WEB_SetHeaderS(q, "Content-Type", "application/json; charset=utf8");
	WEB_SetHeaderS(q, "Content-Language", q->lang);
	WEB_SetHeaderS(q, "Cache-Control", "no-cache, no-store, must-revalidate");
	WEB_SetHeaderS(q, "Expires", "0");
	WEB_PutS(q, "{\"code\": -1,");
	WEB_PutJSON_NoHTML_S(q, "error", AG_GetError());
	WEB_PutS(q, "\"backend_version\": \"" VERSION "\"}\r\n");
	WEB_FlushQuery(q);
	if (sock) { CloseWorkSocket(sock); }
	return WEB_KeepAlive(q);
}

static __inline__ void
CloseFrontSocket(Uint i)
{
	if (i >= webFrontSocketCount) {
		return;
	}
	close(webFrontSockets[i]);

	if (i < webFrontSocketCount - 1) {
		memmove(&webFrontSockets[i], &webFrontSockets[i+1],
		    (webFrontSocketCount-i-1)*sizeof(int));
	}
	webFrontSocketCount--;
}

/*
 * Main routine for a Worker process:
 *
 *  1) Authenticate (if empty sessID), or load an existing session from disk.
 *  2) Write back result and session ID to parent.
 *  3) Loop reading queries (serialized WEB_Query's), processing them
 *     and forwarding the results back to parent. A worker process may also
 *     post events to any active EventSource sockets.
 */
int
WEB_WorkerMain(const WEB_SessionOps *Sops, WEB_Query *qFront,
    const char *user, const char *pass, const char *sessID,
    int pp[2], int nRestoreAttempts)
{
	char sessPath[FILENAME_MAX];
	WEB_Session *S;
	struct sigaction sa;
	struct sockaddr_un sun;
	socklen_t sunLen;
	const char *s, *lang;
	time_t tLastQuery = time(NULL);
	int fd, sockUn = -1, rv = 0, i;
	Uint nQueries = 0;

#ifdef AG_DEBUG
	if (webLangCount < 1) { AG_FatalError("webLangs < 1"); }
#endif
	lang = webLangs[0];

	Strlcpy(webWorkerSess, sessID, sizeof(webWorkerSess));
	Strlcpy(webWorkerUser, user, sizeof(webWorkerUser));

	termFlag = 0;
	chldFlag = 0;
	close(pp[0]);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	if ((S = TryMalloc(Sops->size)) == NULL) {
		rv = EX_OSERR;
		goto fail;
	}
	WEB_SessionInit(S, Sops);

	/* Authenticate */
	if (!(Sops->flags & WEB_SESSION_PREFORK_AUTH) &&
	    Sops->auth != NULL &&
	    Sops->auth(S, user, pass) == -1) {
		rv = EX_NOPERM;
		goto fail;
	}
	AG_SetErrorS("No error");

	if (*sessID != '\0') {					/* Reuse ID */
		if (WEB_SessionLoad(S, sessID) == -1) {
			rv = EX_OSFILE;
			goto fail;
		}
		if (webLanguageFn != NULL && (s = GetSV(S,"language")) &&
		    *s != '\0') {
			lang = s;
			webLanguageFn(lang, webLanguageFnArg);
		}
		for (i = 0; i < webModuleCount; i++) {
			WEB_Module *mod = webModules[i];
			WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;

			if (Cmod->workerInit &&
			    Cmod->workerInit(mod, S) != 0) {
				for (i--; i >= 0; i--) {
					WEB_Module *mod = webModules[i];
					WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;

					if (Cmod->workerDestroy != NULL)
						Cmod->workerDestroy(mod);
				}
				rv = EX_SOFTWARE;
				goto fail;
			}
		}
		if (nRestoreAttempts > 0 && Sops->sessRestored != NULL)
			Sops->sessRestored(S, user);
	} else {
regen_id:
		/* Generate a new session ID. */
		snprintf(S->id, sizeof(S->id), "%.10lu", (Ulong)arc4random());
		Strlcpy(sessPath, WEB_PATH_SESSIONS, sizeof(sessPath));
		Strlcat(sessPath, S->id, sizeof(sessPath));
		if ((fd = open(sessPath, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1) {
			if (errno == EEXIST) {
				goto regen_id;
			} else {
				AG_SetError("%s: %s", sessPath, strerror(errno));
				rv = EX_OSFILE;
				goto fail;
			}
		}
		SetSV_S(S, "user", user);
		SetSV_S(S, "pass", pass);
		SetSV_S(S, "language", qFront->lang);
		SetSV_S(S, "ip", qFront->userIP);
		SetSV_S(S, "host", qFront->userHost);
		SetSV_S(S, "agent", qFront->userAgent);

		if (Sops->sessOpen &&
		    Sops->sessOpen(S, user) == -1) {
			WEB_LogWorker("%s: %s; aborting", S->id, AG_GetError());
			close(fd);
			goto fail_close;
		}
		if (webLanguageFn != NULL && (s = GetSV(S, "language")) &&
		    *s != '\0') {
			lang = s;
			webLanguageFn(s, webLanguageFnArg);
		}
		for (i = 0; i < webModuleCount; i++) {
			WEB_Module *mod = webModules[i];
			WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;

			if (Cmod->workerInit &&
			    Cmod->workerInit(mod, S) != 0) {
				for (i--; i >= 0; i--) {
					WEB_Module *mod = webModules[i];
					WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;
					if (Cmod->workerDestroy != NULL)
						Cmod->workerDestroy(mod);
				}
				webModuleCount = 0;
				close(fd);
				goto fail_close;
			}
		}
		for (i = 0; i < webModuleCount; i++) {
			WEB_Module *mod = webModules[i];
			WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;

			if (Cmod->sessOpen &&
			    Cmod->sessOpen(mod, S) != 0) {
				WEB_LogWorker("%s: %s; aborting session", S->id,
				    AG_GetError());
				close(fd);
				goto fail_close;
			}
		}
		if (WEB_SessionSaveToFD(S, fd) == -1) {
			close(fd);
			goto fail_close;
		}
		close(fd);
	}

	WEB_SetProcTitle("worker %s", user);
	
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigPIPE;
	sigaction(SIGPIPE, &sa, NULL);
	
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigCHLD;
	sigaction(SIGCHLD, &sa, NULL);

	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SigTERM;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if ((sockUn = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		AG_SetError("socket(AF_UNIX): %s", strerror(errno));
		goto fail_close;
	}
	sun.sun_family = AF_UNIX;
	Strlcpy(sun.sun_path, WEB_PATH_SOCKETS, sizeof(sun.sun_path));
	Strlcat(sun.sun_path, S->id, sizeof(sun.sun_path));
	Strlcat(sun.sun_path, ".sock", sizeof(sun.sun_path));
	sun.sun_len = strlen(sun.sun_path)+1;
	sunLen = sun.sun_len + sizeof(sun.sun_family);
	if (bind(sockUn, (struct sockaddr *)&sun, sunLen) == -1) {
		AG_SetError("bind(%s): %s", sun.sun_path, strerror(errno));
		goto fail_close;
	}
	if (listen(sockUn, 10) == -1) {
		AG_SetError("listen: %s", strerror(errno));
		goto fail_close;
	}
	chmod(sun.sun_path, 0700);

	/*
	 * Ready to accept queries. Communicate the new session ID
	 * to the parent process.
	 */
	if (WEB_SYS_Write(pp[1], S->id, strlen(S->id)+1) == -1) {
		close(pp[1]); pp[1] = -1;
		goto fail_close;
	}
	close(pp[1]); pp[1] = -1;
	
	for (;;) {
		struct sockaddr paddr;
		socklen_t paddrLen = sizeof(paddr);
		struct timeval tv;
		int maxFd = 0;
		fd_set rdFds;
		WEB_Query q;

		tv.tv_sec = 10;			/* For Worker timeout test */
		tv.tv_usec = 0;
		FD_ZERO(&rdFds);
		FD_SET(sockUn, &rdFds);
		if (sockUn > maxFd) { maxFd = sockUn; }
		for (i = 0; i < webFrontSocketCount; i++) {
			FD_SET(webFrontSockets[i], &rdFds);
			if (webFrontSockets[i] > maxFd)
				maxFd = webFrontSockets[i];
		}

		if (select(maxFd+1, &rdFds, NULL, 0, &tv) < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetError("select: %s", strerror(errno));
				goto fail_close;
			}
		}
		if (FD_ISSET(sockUn, &rdFds)) {
			pid_t pid;
			int sNew;
		
			if ((sNew = accept(sockUn, &paddr, &paddrLen)) == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					continue;
				} else {
					AG_SetError("accept: %s", strerror(errno));
					goto fail_close;
				}
			}
			if (webFrontSocketCount+1 > WEB_MAXWORKERSOCKETS) {
				WEB_LogWorker("Too many sockets: %u > %u; "
				              "closing %d",
					      webFrontSocketCount+1,
					      WEB_MAXWORKERSOCKETS,
					      sNew);
				close(sNew);
				continue;
			}
/*			WEB_LogWorker("Adding sock%u: %d", webFrontSocketCount,
			    sNew); */
			/*
			 * Write the PID of this Worker (so the calling
			 * Frontend will know which pipe to close whenever a
			 * child is reported dead).
			 */
			pid = getpid();
			if (WEB_SYS_Write(sNew, &pid, sizeof(pid)) == -1) {
				close(sNew);
			}
			webFrontSockets[webFrontSocketCount++] = sNew;
			continue;
		}
		for (i = 0; i < webFrontSocketCount; i++) {
			if (FD_ISSET(webFrontSockets[i], &rdFds))
				break;
		}
		if (i < webFrontSocketCount) {
			char queryData[WEB_QUERY_MAX];
			Uint32 queryLen;
			WEB_Cookie *ck;
			time_t tExpire;
			struct tm *tmExpire;
			
			/* Read serialized Query from front-end. */
			if (WEB_SYS_Read(webFrontSockets[i], &queryLen,
			    sizeof(Uint32)) == -1) {
				if (strcmp(AG_GetError(), "EOF") == 0) {
					WEB_LogWorker("EOF before Query; "
					              "closing sockets[%d]", i);
					CloseFrontSocket(i);
					continue;
				} else {
					goto fail_close;
				}
			}
			if (queryLen == 0 || queryLen >= WEB_QUERY_MAX) {
				WEB_LogErr("Query (%u) too big", queryLen);
				goto fail_close;
			}
			if (WEB_SYS_Read(webFrontSockets[i], queryData,
			    queryLen) == -1) {
				if (strcmp(AG_GetError(), "EOF") == 0) {
					WEB_LogNotice("EOF mid-Query"
					              "closing sockets[%d]", i);
					CloseFrontSocket(i);
					continue;
				} else {
					goto fail_close;
				}
			}

			/* Deserialize the WEB_Query object. */
			WEB_QueryInit(&q, lang);
			if (WEB_QueryLoad(&q, queryData, queryLen) == -1)
				goto fail_close;
			
			q.sock = webFrontSockets[i];
			q.sess = S;
			tLastQuery = time(NULL);
			tExpire = tLastQuery + Sops->sessTimeout;
			tmExpire = localtime(&tExpire);
			ck = WEB_SetCookieS(&q, "sess", S->id);
			ck->path[0] = '/';
			ck->path[1] = '\0';
			strftime(ck->expires, sizeof(ck->expires),
			    "%a, %d %b %Y %H:%M:%S GMT", tmExpire);
			/* ck->flags |= WEB_COOKIE_SECURE; */
			
			/* Handle multipart/form-data POSTs generically. */
			if (q.method == WEB_METHOD_POST &&
			    strncmp(q.contentType, "multipart/form-data",19)==0 &&
			    q.contentLength > 0) {
				if (WEB_ReadFormData(&q, q.sock) == -1) {
					WEB_SetCode(&q, "400 Bad Request");
					goto fail;
				}
				q.flags |= WEB_QUERY_CONTENT_READ;
			}

			WEB_BeginWorkerQuery(&q);
			if (WEB_ExecWorkerQuery(&q) == 1) {
				/* Terminate (e.g., /logout op). */
				WEB_LogWorker("Terminating by request");
				WEB_QueryDestroy(&q);
				break;
			}
			WEB_QueryDestroy(&q);
			WEB_SetProcTitle("worker %s (%u)", user, nQueries);
			nQueries++;
		} else {
			if ((time(NULL) - tLastQuery) > Sops->workerTimeout) {
				WEB_LogWorker("Inactivity timeout (%u queries)",
				    nQueries);
				break;
			}
		}
		WEB_CheckSignals();
	}

	for (i = 0; i < webModuleCount; i++) {
		WEB_Module *mod = webModules[i];
		WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;

		if (Cmod->workerDestroy != NULL)
			Cmod->workerDestroy(mod);
	}
	if (sockUn != -1) { close(sockUn); }
	for (i = 0; i < webFrontSocketCount; i++) {
		close(webFrontSockets[i]);
	}
	unlink(sun.sun_path);
	WEB_SessionFree(S);
	return (0);
fail_close:
	for (i = 0; i < webModuleCount; i++) {
		WEB_Module *mod = webModules[i];
		WEB_ModuleClass *Cmod = (WEB_ModuleClass *)OBJECT(mod)->cls;
		if (Cmod->workerDestroy)
			Cmod->workerDestroy(mod);
	}
	rv = EX_SOFTWARE;
	if (sockUn != -1) { close(sockUn); }
	for (i = 0; i < webFrontSocketCount; i++) {
		close(webFrontSockets[i]);
	}
	unlink(sun.sun_path);
	unlink(sessPath);
fail:
	WEB_SessionFree(S);
	WEB_LogWorker("Exiting (%s)", AG_GetError());
	if (pp[1] != -1) {
		WEB_SYS_Write(pp[1], (rv == EX_NOPERM) ?
		    "AUTH      " :
		    "FAIL      ", 10);
		if (rv == EX_NOPERM) {
			char rbuf;
			WEB_SYS_Read(pp[1], &rbuf, 1);
			WEB_LogWorker("Parent responded: %c", rbuf);
		}
		close(pp[1]);
	}
	sleep(1);
	return (rv);
}

/*
 * Send a control command to a Frontend process.
 * Return 0 on success, -1 on failure and 1 if clusterID doesn't exist.
 */
int
WEB_ControlCommand(Uint clusterID, const WEB_ControlCmd *cmd)
{
	struct sockaddr_un sun;
	socklen_t sunLen;
	struct stat sb;
	int fd, status;
	
	snprintf(sun.sun_path, sizeof(sun.sun_path), "%s%u.ctrl",
	    WEB_PATH_SOCKETS, clusterID);

	if (stat(sun.sun_path, &sb) != 0) {
		return (1);
	}
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		AG_SetError("socket: %s", strerror(errno));
		return (-1);
	}
try_connect:
	sun.sun_family = AF_UNIX;
	sun.sun_len = strlen(sun.sun_path)+1;
	sunLen = sun.sun_len + sizeof(sun.sun_family);
	if (connect(fd, (struct sockaddr *)&sun, sunLen) == -1) {
		if (errno == EINTR || errno == EAGAIN) {
			WEB_CheckSignals();
			goto try_connect;
		} else {
			AG_SetErrorS(strerror(errno));
			goto fail;
		}
	}
	if (WEB_SYS_Write(fd, cmd, sizeof(WEB_ControlCmd)) == -1) {
		goto fail;
	}
	if (cmd->flags & WEB_CONTROL_CMD_SYNC) {
		if (WEB_SYS_Read(fd, &status, sizeof(status)) == -1)
			goto fail;
	} else {
		status = 0;
	}
	close(fd);
	return (status);
fail:
	close(fd);
	return (-1);
}

/* Send a control command to a Frontend process. */
int
WEB_ControlCommandS(Uint clusterID, const char *s)
{
	WEB_ControlCmd cmd;

	bzero(&cmd, sizeof(cmd));
	if (strncmp(s, "shutdown",8) == 0) {		/* shutdown [msg] */
		cmd.type = WEB_CONTROL_SHUTDOWN;
		if (isspace(s[8]) && s[9] != '\0') {
			Strlcpy(cmd.data.shutdown.reason, &s[9],
			    sizeof(cmd.data.shutdown.reason));
		}
	} else if (strcmp(s, "noop") == 0) {		/* noop */
		cmd.flags |= WEB_CONTROL_CMD_SYNC;
		cmd.type = WEB_CONTROL_NOOP;
	} else {
		AG_SetError("Bad control command: %s", s);
		return (-1);
	}
	return WEB_ControlCommand(clusterID, &cmd);
}

void
WEB_AddLanguage(const char *_Nonnull lang)
{
	if (webLangs == NULL) {
		webLangs = Malloc(2*sizeof(char *));
	} else {
		webLangs = Realloc(webLangs, (webLangCount+2)*sizeof(char *));
	}
	webLangs[webLangCount] = Strdup(lang);
	WEB_LogInfo("Registered language #%d: %s", webLangCount, webLangs[webLangCount]);
	webLangs[++webLangCount] = NULL;
}

/* Standard loop for a web application server. */
void
WEB_QueryLoop(const char *hostname, const char *port, const WEB_SessionOps *Sops)
{
	struct addrinfo hints, *res, *res0;
	char   httpSocks[WEB_MAXHTTPSOCKETS];
	Uint  nHttpSocks;
	const char *cause = "";
	struct sockaddr_un sun;
	socklen_t sunLen;
	fd_set httpSockFDs;
	int maxFd = 0;
	int i, rv, val, sock = -1;
	ssize_t rvLen;
	char *c, *cEnd, *uriEnd;
	struct stat sb;

	if (webLangCount == 0) {
		WEB_LogWarn("No languages specified, defaulting to en");
		WEB_AddLanguage("en");
	} else {
		for (i = 0; i < webLangCount; i++)
			WEB_LogInfo("Accepted language: %s", webLangs[i]);
	}

	WEB_LogNotice("Starting %s #%u%s (%s; agar %s) on %s:%s", agProgName,
	    webClusterID,
	    webEventSource ? "<Ev>" : "",
	    agProgName, VERSION, hostname, port);

	if (stat(WEB_PATH_SESSIONS,&sb) != 0 && mkdir(WEB_PATH_SESSIONS, 0700) != 0) {
		WEB_LogErr("%s: %s", WEB_PATH_SESSIONS, strerror(errno));
		return;
	}
	if (stat(WEB_PATH_SOCKETS,&sb) != 0 && mkdir(WEB_PATH_SOCKETS, 0700) != 0) {
		WEB_LogErr("%s: %s", WEB_PATH_SOCKETS, strerror(errno));
		return;
	}
	if (stat(WEB_PATH_EVENTS,&sb) != 0 && mkdir(WEB_PATH_EVENTS, 0700) != 0) {
		WEB_LogErr("%s: %s", WEB_PATH_EVENTS, strerror(errno));
		return;
	}

	/* Listen on HTTP sockets */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(hostname, port, &hints, &res0)) != 0) {
		WEB_LogErr("%s:%s: %s", hostname, port, gai_strerror(rv));
		return;
	}
	FD_ZERO(&httpSockFDs);
	for (nHttpSocks=0, res=res0;
	     res != NULL && nHttpSocks < WEB_MAXHTTPSOCKETS;
	     res = res->ai_next) {
		rv = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (rv == -1) {
			cause = "socket";
			continue;
		}
		val = 1;
		setsockopt(rv, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
		if (bind(rv, res->ai_addr, res->ai_addrlen) == -1) {
			cause = "bind";
			close(rv);
			continue;
		}
		if (listen(rv, 20) == -1) {
			cause = "listen";
			close(rv);
			continue;
		}
		httpSocks[nHttpSocks++] = rv;
		FD_SET(rv, &httpSockFDs);
		if (rv > maxFd) { maxFd = rv; }
	}
	if (nHttpSocks == 0) {
		AG_SetError("%s: %s", cause, strerror(errno));
		goto fail;
	}
	freeaddrinfo(res0);
	
	/* Listen on control socket */
	if ((webCtrlSock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		AG_SetError("socket(AF_UNIX): %s", strerror(errno));
		goto fail;
	}
	sun.sun_family = AF_UNIX;
	snprintf(sun.sun_path, sizeof(sun.sun_path), "%s%u.ctrl",
	    WEB_PATH_SOCKETS, webClusterID);
	sun.sun_len = strlen(sun.sun_path)+1;
	sunLen = sun.sun_len + sizeof(sun.sun_family);
	unlink(sun.sun_path);
	if (bind(webCtrlSock, (struct sockaddr *)&sun, sunLen) == -1) {
		AG_SetError("bind(%s): %s", sun.sun_path, strerror(errno));
		goto fail;
	}
	if (listen(webCtrlSock, 10) == -1) {
		AG_SetError("listen: %s", strerror(errno));
		goto fail;
	}
	chmod(sun.sun_path, 0700);

	for (;;) {
		char rdBuf[WEB_FRONTEND_RDBUFSIZE]; /* Must > sizeof(header) */
		char header[WEB_HTTP_HEADER_MAX];
		char uri[MAXPATHLEN];
		struct sockaddr paddr;
		socklen_t paddrLen = sizeof(paddr);
		AG_Size headerLen, rdBufLen;
		WEB_Method meth;
		fd_set readFds = httpSockFDs;

		FD_SET(webCtrlSock, &readFds);
		if (webCtrlSock > maxFd) { maxFd = webCtrlSock; }

		rv = select(maxFd+1, &readFds, NULL, NULL, NULL);
		if (rv == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetError("select: %s", strerror(errno));
				goto fail;
			}
		}
		if (FD_ISSET(webCtrlSock, &readFds)) {
			if (WEB_HandleControlCmd(webCtrlSock) == -1)
				WEB_LogErr("Control socket (in main): %s",
				    AG_GetError());
		}
		for (i = 0; i < nHttpSocks; i++) {
			if (!FD_ISSET(httpSocks[i], &readFds))
				continue;
try_accept:
			if ((sock = accept(httpSocks[i], &paddr, &paddrLen)) == -1) {
				if (errno == EINTR) {
					WEB_CheckSignals();
					goto try_accept;
				} else {
					WEB_LogErr("accept: %s", strerror(errno));
					continue;
				}
			}
			break;
		}
		if (i == nHttpSocks) {
			continue;
		}
		if (getnameinfo(&paddr, paddrLen, webPeerAddress,
		    sizeof(webPeerAddress), NULL, 0, NI_NUMERICHOST) != 0)
			webPeerAddress[0] = '\0';
read_header:
		/* Read HTTP request header */
		header[0] = '\0';
		headerLen = 0;
		rdBufLen = 0;
		for (;;) {
			/* TODO: WEB_HTTP_REQ_TIMEOUT */
			rvLen = read(sock, &header[headerLen], sizeof(header) -
			                                       headerLen - 1);
			if (rvLen == -1) {
				if (errno == EINTR || errno == EAGAIN) {
					WEB_CheckSignals();
					continue;
				} else {
					WEB_LogErr("HTTP header: %s", strerror(errno));
					goto finish;
				}
			}
			header[headerLen+rvLen] = '\0';
			headerLen += rvLen;

			if ((c = strstr(header, "\r\n\r\n")) != NULL) {
				*c = '\0';
				if (headerLen > c-header+4) {      /* Extra */
					rdBufLen = headerLen - (c-header+4);
					memcpy(rdBuf, &c[4], rdBufLen);
					headerLen = (c - &header[0]);
				}
				break;
			}
			if (rvLen == 0)
				break;
		}
		if (headerLen < WEB_HTTP_HEADER_MIN) {
			goto finish;
		}
		if ((cEnd = strchr(header,' ')) == NULL) {
			WEB_LogErr("Bad method");
			goto finish;
		}
		*cEnd = '\0';

		for (meth=0; meth < WEB_METHOD_LAST; meth++) {
			AG_Size nameLen;

			if (strcmp(header, webMethods[meth].name) != 0) {
				continue;
			}
			nameLen = strlen(webMethods[meth].name);
			if ((uriEnd = strchr(&header[nameLen+1],'\r')) == NULL) {
				WEB_LogErr("Bad request");
				goto finish;
			}
			*uriEnd = '\0';
			Strlcpy(uri, &header[nameLen+1], sizeof(uri));
			if ((cEnd = strrchr(uri,' ')) == NULL ||
			    strcasecmp(cEnd, " HTTP/1.1") != 0) {
				WEB_LogErr("Bad protocol");
				goto finish;
			}
			*cEnd = '\0';
			uriEnd += 2;			/* \r\n */
			if (uri[0] == '\0') {
				WEB_LogErr("Bad request");
				goto finish;
			}
			break;
		}
		if (meth == WEB_METHOD_LAST) {
			WEB_MethodNotAllowed(sock, uri, c, rdBuf, rdBufLen, Sops);
			goto finish;
		}
		c = uriEnd;
		if (webMethods[meth].fn(sock, uri, c, rdBuf, rdBufLen, Sops)==1) {
			webQueryCount++;
			WEB_CheckSignals();
			goto read_header;		/* Keep-alive */
		}
		WEB_LogDebug("[%s]: Closing connection", uri);
finish:
		close(sock);
		webQueryCount++;
		WEB_CheckSignals();
	}

	for (i = 0; i < nHttpSocks; i++) { close(httpSocks[i]); }
	close(webCtrlSock);
	unlink(sun.sun_path);
	return;
fail:
	for (i = 0; i < nHttpSocks; i++) { close(httpSocks[i]); }
	if (webCtrlSock != -1) {
		close(webCtrlSock);
		unlink(sun.sun_path);
	}
	WEB_LogErr("WEB_QueryLoop: %s; exiting", AG_GetError());
}

WEB_Command webModuleCommands[] = {
	/* name     fn        type         flags */
/*	{ "index",  myIndex, "text/html", "Pi" } */
	{ NULL,     NULL,   NULL,        NULL }
};
WEB_ModuleClass webModuleClass = {
	{
		"Web(Module)",
		sizeof(WEB_Module),
		{ 0, 0 },
		NULL,		/* init */
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	"myModule",		/* name */
	"x",			/* icon */
	"My Module",		/* lname */
	"Module description",	/* desc */
	NULL,			/* workerInit */
	NULL,			/* workerDestroy */
	NULL,			/* sessOpen */
	NULL,			/* sessClose */
	NULL,			/* menu */
	NULL,			/* menuSections */
	&webModuleCommands[0]
};
