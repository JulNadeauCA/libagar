/*
 * Copyright (c) 2003-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Variable substitution filter for precompiled HTML documents.
 * Variable initialization and utility routines.
 */

#include <agar/core/core.h>
#include <agar/net/web.h>

#define VAR_GETTEXT_MAX 256	/* Max string length for $_(foo) */

#include <stdio.h>
#include <ctype.h>

#include <agar/config/enable_nls.h>

enum web_varsubst_mode {
	WEB_VARSUBST_NORMAL,
	WEB_VARSUBST_VAR,
	WEB_VARSUBST_ESCAPE,
	WEB_VARSUBST_TRANSLATE
};

/* Set a variable (format string). */
VAR *
WEB_VAR_Set(const char *key, const char *fmt, ...)
{
	VAR *V;

	if (key != NULL) {
		TAILQ_FOREACH(V, &webVars, vars) {
			if (strcmp(V->key, key) == 0)
				break;
		}
	} else {
		V = NULL;
	}
	if (V == NULL) {
		V = Malloc(sizeof(VAR));
		if (key != NULL) {
			Strlcpy(V->key, key, sizeof(V->key));
		} else {
			V->key[0] = '\0';
		}
		TAILQ_INSERT_HEAD(&webVars, V, vars);
	} else {
		free(V->value);
	}
	if (fmt != NULL) {
		va_list ap;
	
		va_start(ap, fmt);
		if (vasprintf(&V->value, fmt, ap) == -1) {
			V->value = NULL;
			V->len = 0;
			V->bufSize = 0;
		} else {
			V->len = strlen(V->value);
			V->bufSize = V->len+1;
		}
		va_end(ap);
	} else {
		V->value = Malloc(WEB_VAR_BUF_INIT);
		V->value[0] = '\0';
		V->len = 0;
		V->bufSize = WEB_VAR_BUF_INIT;
	}
	V->global = 0;
	return (V);
}

/* Set a variable (plain string). */
VAR *
WEB_VAR_SetS(const char *key, const char *s)
{
	VAR *V;

	if (key != NULL) {
		TAILQ_FOREACH(V, &webVars, vars) {
			if (strcmp(V->key, key) == 0)
				break;
		}
	} else {
		V = NULL;
	}
	if (V == NULL) {
		V = Malloc(sizeof(VAR));
		if (key != NULL) {
			Strlcpy(V->key, key, sizeof(V->key));
		} else {
			V->key[0] = '\0';
		}
		TAILQ_INSERT_HEAD(&webVars, V, vars);
	} else {
		free(V->value);
	}
	if (s != NULL) {
		V->value = Strdup(s);
		V->len = strlen(s);
		V->bufSize = V->len+1;
	} else {
		V->value = Malloc(WEB_VAR_BUF_INIT);
		V->value[0] = '\0';
		V->len = 0;
		V->bufSize = WEB_VAR_BUF_INIT;
	}
	V->global = 0;
	return (V);
}

/* Set a variable (plain string, use existing buffer). */
VAR *
WEB_VAR_SetS_NODUP(const char *key, char *s)
{
	VAR *V;

	if (key != NULL) {
		TAILQ_FOREACH(V, &webVars, vars) {
			if (strcmp(V->key, key) == 0)
				break;
		}
	} else {
		V = NULL;
	}
	if (V == NULL) {
		V = Malloc(sizeof(VAR));
		if (key != NULL) {
			Strlcpy(V->key, key, sizeof(V->key));
		} else {
			V->key[0] = '\0';
		}
		TAILQ_INSERT_HEAD(&webVars, V, vars);
	} else {
		free(V->value);
	}
	V->value = s;
	V->len = strlen(s);
	V->bufSize = V->len+1;
	V->global = 0;
	return (V);
}

/* Append to an existing variable. */
void
WEB_VAR_Cat(VAR *V, const char *fmt, ...)
{
	char *s;
	AG_Size len;
	va_list ap;

	va_start(ap, fmt);
	if (vasprintf(&s, fmt, ap) == -1) {
		va_end(ap);
		return;
	}
	va_end(ap);

	len = strlen(s);
	WEB_VAR_Grow(V, len+1);
	memcpy(&V->value[V->len], s, len+1);
	V->len += len;
	free(s);
}

/* Set a global variable (format string). */
VAR *
WEB_VAR_SetGlobal(const char *key, const char *fmt, ...)
{
	VAR *V;

	TAILQ_FOREACH(V, &webVars, vars) {
		if (strcmp(V->key, key) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(VAR));
		Strlcpy(V->key, key, sizeof(V->key));
		TAILQ_INSERT_HEAD(&webVars, V, vars);
	} else {
		free(V->value);
	}
	if (fmt != NULL) {
		va_list ap;
	
		va_start(ap, fmt);
		if (vasprintf(&V->value, fmt, ap) == -1) {
			V->value = NULL;
			V->len = 0;
			V->bufSize = 0;
		} else {
			V->len = strlen(V->value);
			V->bufSize = V->len+1;
		}
		va_end(ap);
	} else {
		V->value = Malloc(WEB_VAR_BUF_INIT);
		V->value[0] = '\0';
		V->len = 0;
		V->bufSize = WEB_VAR_BUF_INIT;
	}
	V->global = 1;
	return (V);
}

/* Set a global variable (plain string). */
VAR *
WEB_VAR_SetGlobalS(const char *key, const char *s)
{
	VAR *V;

	TAILQ_FOREACH(V, &webVars, vars) {
		if (strcmp(V->key, key) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(VAR));
		Strlcpy(V->key, key, sizeof(V->key));
		TAILQ_INSERT_HEAD(&webVars, V, vars);
	} else {
		free(V->value);
	}
	if (s != NULL) {
		V->value = Strdup(s);
		V->len = strlen(V->value);
		V->bufSize = V->len+1;
	} else {
		V->value = Malloc(WEB_VAR_BUF_INIT);
		V->value[0] = '\0';
		V->len = 0;
		V->bufSize = WEB_VAR_BUF_INIT;
	}
	V->global = 1;
	return (V);
}

void
WEB_VAR_Unset(const char *key)
{
	VAR *V;

	TAILQ_FOREACH(V, &webVars, vars) {
		if (strcmp(V->key, key) == 0)
			break;
	}
	if (V != NULL) {
		TAILQ_REMOVE(&webVars, V, vars);
		WEB_VAR_Free(V);
	}
}

void
WEB_VAR_Wipe(const char *key)
{
	VAR *V;

	TAILQ_FOREACH(V, &webVars, vars) {
		if (strcmp(V->key, key) == 0)
			break;
	}
	if (V != NULL)
		memset(V->value, 0, V->bufSize);
}

int
WEB_VAR_Defined(const char *key)
{
	VAR *V;

	TAILQ_FOREACH(V, &webVars, vars) {
		if (strcmp(V->key, key) == 0)
			return (1);
	}
	return (0);
}

void
WEB_VAR_Free(VAR *V)
{
	TAILQ_REMOVE(&webVars, V, vars);
	Free(V->value);
	free(V);
}

/*
 * Variable Substitution
 */

static __inline__ int
VarNameChar(const char c)
{
	return (isalnum(c) || c == '_');
}

/*
 * Perform variable substitution and translation on a whole HTML document.
 * Return results without further transformation. 
 */
void
WEB_VAR_FilterDocument(WEB_Query *q, const char *src, AG_Size srcLen)
{
	char vName[VAR_GETTEXT_MAX];
	enum web_varsubst_mode mode;
	const char *c, *s;
	char cDst, *pName;

	for (c = src; c < &src[srcLen]; ) {
		/* XXX TODO: Block copy everything up first !NORMAL */
		mode = WEB_VARSUBST_NORMAL;
		if (c[0] == '%' && c < &src[srcLen-3] &&	/* %24foo */
		    c[1] == '2' && c[2] == '4') {
			if (c[3] == '%' && &c[3] < &src[srcLen-3] &&
			    c[4] == '2' &&  c[5] == '4') {
				mode = WEB_VARSUBST_ESCAPE;
			} else {
				mode = WEB_VARSUBST_VAR;
			}
			c+=3;
			if (c[0] == '_' && c[1] == '(') {
				mode = WEB_VARSUBST_TRANSLATE;
				c+=2;
				for (pName = &vName[0];
				     c < &src[srcLen-1] && *c != ')' &&
				       isprint(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++) {
					*pName = *c;
					pName++;
				}
				c++;
			} else {
				for (pName = &vName[0];
				     c < &src[srcLen-1] && VarNameChar(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++, pName++) {
					*pName = *c;
				}
			}
			*pName = '\0';
		} else if (*c == '$') {
			mode = (c[1]=='$') ? WEB_VARSUBST_ESCAPE :
				             WEB_VARSUBST_VAR;
			c++;
			if (c[0] == '_' && c[1] == '(') {
				mode = WEB_VARSUBST_TRANSLATE;
				c+=2;
				for (pName = &vName[0];
				     c < &src[srcLen-1] && *c != ')' &&
				       isprint(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++) {
					*pName = *c;
					pName++;
				}
				c++;
			} else {
				for (pName = &vName[0];
				     c < &src[srcLen-1] && VarNameChar(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++, pName++) {
					*pName = *c;
				}
			}
			*pName = '\0';
		}

		switch (mode) {
		case WEB_VARSUBST_NORMAL:
			cDst = *(c++);
			WEB_Write(q, &cDst, 1);
			break;
		case WEB_VARSUBST_ESCAPE:
			cDst = '$';
			WEB_Write(q, &cDst, 1);
			WEB_Write(q, vName, strlen(vName));
			break;
		case WEB_VARSUBST_TRANSLATE:
#ifdef ENABLE_NLS
			s = gettext(vName);
			WEB_Write(q, s, strlen(s));
#else
			WEB_Write(q, vName, strlen(vName));
#endif
			break;
		case WEB_VARSUBST_VAR:
			if (vName[0] == '\0') {
				break;
			}
			if ((s = Get(vName)) != NULL) {
				WEB_Write(q, s, strlen(s));
			} else {
				WEB_LogErr("Uninitialized: $%s", vName);
			}
			break;
		}
	}
}

/*
 * Perform variable substitution and translation on a HTML code fragment.
 * Transform characters to make output JSON-safe for [json] mode.
 * Ignore contents outside of <body></body>.
 */
static __inline__ void
WEB_VAR_WriteJSON(WEB_Query *_Nonnull q, const char *_Nonnull s, AG_Size len)
{
	AG_Size i;
	const char *c;
	
	for (i=0, c=s; i < len; i++, c++) {
		if     (*c == '\\') { WEB_PutS(q, "\\\\"); }
		else if (*c == '"') { WEB_PutS(q, "\\\""); }
		else if (*c == '\n') { WEB_PutS(q, "\\n"); }
		else if (*c == '\t') { WEB_PutS(q, "\\t"); }
		else { WEB_PutC(q, *c); }
	}
}
void
WEB_VAR_FilterFragment(WEB_Query *q, const char *src, AG_Size srcLen)
{
	char vName[VAR_GETTEXT_MAX];
	enum web_varsubst_mode mode;
	int inBody = 0;
	const char *c, *s;
	char cDst, *pName;

	for (c = src; c < &src[srcLen]; ) {
		if (*c == '<') {
			if (strncmp(&c[1],"body>",5) == 0) {
				inBody = 1;
			} else if (strncmp(&c[1],"/body>",6) == 0) {
				break;
			}
		}
		if (!inBody) {
			continue;
		}

		/* XXX TODO: Block copy everything up first !NORMAL */
		mode = WEB_VARSUBST_NORMAL;

		if (c[0] == '%' && c < &src[srcLen-3] &&	/* %24foo */
		    c[1] == '2' && c[2] == '4') {
			if (c[3] == '%' && &c[3] < &src[srcLen-3] &&
			    c[4] == '2' &&  c[5] == '4') {
				mode = WEB_VARSUBST_ESCAPE;
			} else {
				mode = WEB_VARSUBST_VAR;
			}
			c+=3;
			if (c[0] == '_' && c[1] == '(') {
				mode = WEB_VARSUBST_TRANSLATE;
				c+=2;
				for (pName = &vName[0];
				     c < &src[srcLen-1] && *c != ')' &&
				       isprint(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++) {
					*pName = *c;
					pName++;
				}
				c++;
			} else {
				for (pName = &vName[0];
				     c < &src[srcLen-1] && VarNameChar(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++, pName++) {
					*pName = *c;
				}
			}
			*pName = '\0';
		} else if (*c == '$') {
			mode = (c[1]=='$') ? WEB_VARSUBST_ESCAPE :
				             WEB_VARSUBST_VAR;
			c++;
			if (c[0] == '_' && c[1] == '(') {
				mode = WEB_VARSUBST_TRANSLATE;
				c+=2;
				for (pName = &vName[0];
				     c < &src[srcLen-1] && *c != ')' &&
				       isprint(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++) {
					*pName = *c;
					pName++;
				}
				c++;
			} else {
				for (pName = &vName[0];
				     c < &src[srcLen-1] && VarNameChar(*c) &&
				       pName < &vName[sizeof(vName)-1];
				     c++, pName++) {
					*pName = *c;
				}
			}
			*pName = '\0';
		}
		
		switch (mode) {
		case WEB_VARSUBST_NORMAL:
			cDst = *(c++);
			WEB_VAR_WriteJSON(q, &cDst, 1);
			break;
		case WEB_VARSUBST_ESCAPE:
			cDst = '$';
			WEB_VAR_WriteJSON(q, &cDst, 1);
			WEB_VAR_WriteJSON(q, vName, strlen(vName));
			break;
		case WEB_VARSUBST_TRANSLATE:
#ifdef ENABLE_NLS
			s = gettext(vName);
			WEB_VAR_WriteJSON(q, s, strlen(s));
#else
			WEB_VAR_WriteJSON(q, vName, strlen(vName));
#endif
			break;
		case WEB_VARSUBST_VAR:
			if (vName[0] == '\0') {
				break;
			}
			if ((s = Get(vName)) != NULL) {
				WEB_VAR_WriteJSON(q, s, strlen(s));
			} else {
				WEB_LogErr("Uninitialized: $%s", vName);
			}
			break;
		}
	}
}
