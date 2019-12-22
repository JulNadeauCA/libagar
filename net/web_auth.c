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
 * Authentication and session management framework.
 */

#include <agar/core/core.h>
#include <agar/net/web.h>

#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

/* Initialize a session instance. */
void
WEB_SessionInit(WEB_Session *S, const WEB_SessionOps *Sops)
{
	S->ops = Sops;
	S->id[0] = '\0';
	S->nVars = 0;
	TAILQ_INIT(&S->vars);
	S->pp[0] = -1;
	S->pp[1] = -1;
	S->nEvents = 0;

	if (Sops->init != NULL)
		Sops->init(S);
}

/* Free a session instance and associated variables. */
void
WEB_SessionDestroy(WEB_Session *S)
{
	WEB_SessionVar *SV, *nextSV;

	if (S->ops->destroy != NULL) {
		S->ops->destroy(S);
	}
	for (SV = TAILQ_FIRST(&S->vars);
	     SV != TAILQ_END(&S->vars);
	     SV = nextSV) {
		nextSV = TAILQ_NEXT(SV, vars);
		free(SV);
	}
}

/* Terminate a session gracefully. */
void
WEB_CloseSession(WEB_Session *S)
{
	char path[FILENAME_MAX];
	int i;
	
	if (S->ops->sessClose != NULL) {
		S->ops->sessClose(S);
	}
	for (i = 0; i < webModuleCount ; i++) {
		WEB_Module *mod = webModules[i];
		WEB_ModuleClass *modC = (void *)AGOBJECT(mod)->cls;

		if (modC->sessClose != NULL)
			modC->sessClose(mod, S);
	}

	Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
	Strlcat(path, S->id, sizeof(path));
	unlink(path);

	Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
	Strlcat(path, S->id, sizeof(path));
	Strlcat(path, ".sock", sizeof(path));
	unlink(path);
	
	Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
	Strlcat(path, S->id, sizeof(path));
	Strlcat(path, ".events", sizeof(path));
	unlink(path);
}

/* Load session instance data from disk. */
int
WEB_SessionLoad(void *pSess, const char *id)
{
	char path[FILENAME_MAX];
	WEB_Session *S = pSess;
	WEB_SessionVar *SV;
	AG_DataSource *ds;
	Uint32 i, count;

	Strlcpy(S->id, id, sizeof(S->id));

	Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
	Strlcat(path, id, sizeof(path));
	if ((ds = AG_OpenFile(path, "r")) == NULL)
		return (-1);

	if ((i = AG_ReadUint32(ds)) != WEB_SESSION_DATA_MAGIC) {
		AG_SetError("Bad magic: 0x%x (%s)", i, path);
		goto fail;
	}

	if ((count = AG_ReadUint32(ds)) == 0 ||
	    count > WEB_SESSION_VARIABLES_MAX) {
		AG_SetError("Bad SV count: %u", (Uint)count);
		goto fail;
	}
	for (i=0, S->nVars=0; i < count; i++) {
		if (!(SV = TryMalloc(sizeof(WEB_SessionVar)))) {
			goto fail;
		}
		AG_CopyString(SV->key, ds, sizeof(SV->key));
		AG_CopyString(SV->value, ds, sizeof(SV->value));
		TAILQ_INSERT_TAIL(&S->vars, SV, vars);
		S->nVars++;
	}
	S->nEvents = (Uint)AG_ReadUint32(ds);

	/* Read any additional session-manager-specific data. */
	if (S->ops->load != NULL &&
	    S->ops->load(S, ds) == -1)
		goto fail;

	AG_CloseFile(ds);
	return (0);
fail:
	AG_CloseFile(ds);
	return (-1);
}

/* Save session information to disk. */
int
WEB_SessionSaveToFD(void *pSess, int fd)
{
	WEB_Session *S = pSess;
	WEB_SessionVar *SV;
	AG_DataSource *ds;
	FILE *f;
	
	if (fd != -1) {
		if (!(f = fdopen(fd, "w"))) {
			AG_SetError("fdopen");
			return (-1);
		}
		if ((ds = AG_OpenFileHandle(f)) == NULL) {
			fdclose(f,NULL);
			return (-1);
		}
	} else {
		char path[FILENAME_MAX];

		Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
		Strlcat(path, S->id, sizeof(path));
		if ((ds = AG_OpenFile(path, "w")) == NULL)
			return (-1);
	}

	AG_WriteUint32(ds, WEB_SESSION_DATA_MAGIC);
	AG_WriteUint32(ds, S->nVars);
	TAILQ_FOREACH(SV, &S->vars, vars) {
		AG_WriteString(ds, SV->key);
		AG_WriteString(ds, SV->value);
	}

	AG_WriteUint32(ds, (Uint32)S->nEvents);

	/* Write any extra session-manager specific data */
	if (S->ops->save != NULL)
		S->ops->save(S, ds);
	
	if (fd != -1) {
		AG_CloseFileHandle(ds);
	} else {
		AG_CloseFile(ds);
	}
	return (0);
}

/* Update a variable in every session opened by a given user. */
int
WEB_SetSV_ALL(const WEB_SessionOps *sessOps, const char *user, const char *key,
    const char *val)
{
	char path[FILENAME_MAX];
	struct dirent *dent;
	WEB_Session *S;
	const char *s;
	DIR *dir;

	if ((S = TryMalloc(sessOps->size)) == NULL)
		return (-1);

	if ((dir = opendir(WEB_PATH_SESSIONS)) == NULL) {
		AG_SetError("%s: %s", WEB_PATH_SESSIONS, strerror(errno));
		return (-1);
	}
	while ((dent = readdir(dir)) != NULL) {
		if (strchr(dent->d_name, '.') != NULL) {
			continue;
		}
		Strlcpy(path, WEB_PATH_SESSIONS, sizeof(path));
		Strlcat(path, dent->d_name, sizeof(path));

		WEB_SessionInit(S, sessOps);

		if (WEB_SessionLoad(S, dent->d_name) == -1)
			continue;

		if (!(s = WEB_GetSV(S,"user")) || strcmp(s,user) != 0) {
			WEB_SessionDestroy(S);
			continue;
		}
		WEB_SetSV_S(S, key, val);

		if (WEB_SessionSave(S) == -1) {
			WEB_LogErr("%s", AG_GetError());
		}
		WEB_SessionDestroy(S);
	}
	closedir(dir);
	free(S);
	return (0);
}

/* Set the value of a session variable. */
void
WEB_SetSV(void *pSess, const char *key, const char *fmt, ...)
{
	WEB_Session *S = pSess;
	WEB_SessionVar *SV;
	va_list ap;
	
	TAILQ_FOREACH(SV, &S->vars, vars) {
		if (strcmp(SV->key, key) == 0)
			break;
	}
	if (SV == NULL) {
		SV = Malloc(sizeof(WEB_SessionVar));
		Strlcpy(SV->key, key, sizeof(SV->key));
		TAILQ_INSERT_TAIL(&S->vars, SV, vars);
		S->nVars++;
	}
	va_start(ap, fmt);
	vsnprintf(SV->value, sizeof(SV->value), fmt, ap);
	va_end(ap);
}
void
WEB_SetSV_S(void *pSess, const char *key, const char *s)
{
	WEB_Session *S = pSess;
	WEB_SessionVar *SV;
	
	TAILQ_FOREACH(SV, &S->vars, vars) {
		if (strcmp(SV->key, key) == 0)
			break;
	}
	if (SV == NULL) {
		SV = Malloc(sizeof(WEB_SessionVar));
		Strlcpy(SV->key, key, sizeof(SV->key));
		TAILQ_INSERT_TAIL(&S->vars, SV, vars);
		S->nVars++;
	}
	Strlcpy(SV->value, s, sizeof(SV->value));
}
