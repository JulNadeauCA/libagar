/*
 * Copyright (c) 2012-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Interface to user accounts.
 */

#include <agar/core/core.h>
#ifdef AG_USER

const AG_UserOps *agUserOps = NULL;

/* Allocate a new user structure.  */
AG_User *
AG_UserNew(void)
{
	AG_User *u;

	if ((u = TryMalloc(sizeof(AG_User))) == NULL) {
		return (NULL);
	}
	u->name[0] = '\0';
	u->flags = 0;
	u->passwd = NULL;
	u->uid = 0;
	u->gid = 0;
	u->loginClass = NULL;
	u->gecos = NULL;
	u->home = NULL;
	u->shell = NULL;
	u->tmp = NULL;
	return (u);
}

/* Release a user structure.  */
void
AG_UserFree(AG_User *u)
{
	Free(u->passwd);
	Free(u->loginClass);
	Free(u->gecos);
	Free(u->home);
	Free(u->shell);
	Free(u->tmp);
	free(u);
}

/* Set the user database access backend. */
void
AG_SetUserOps(const AG_UserOps *ops)
{
	if (agUserOps == ops)
		return;
	if (agUserOps != NULL && agUserOps->destroy != NULL) {
		agUserOps->destroy();
	}
	agUserOps = ops;
	if (ops->init != NULL)
		ops->init();
}

/* Lookup a user account by name. */
AG_User *
AG_GetUserByName(const char *_Nonnull name)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getUserByName(u, name) != 1) {
#ifdef AG_VERBOSITY
		AG_SetError("No such user \"%s\"", name);
#else
		AG_SetErrorS("E26");
#endif
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}

/* Lookup a user account by numerical UID. */
AG_User *
AG_GetUserByUID(Uint32 uid)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getUserByUID(u, uid) != 1) {
#ifdef AG_VERBOSITY
		AG_SetError("No such user (uid %lu)", (unsigned long)uid);
#else
		AG_SetErrorS("E26");
#endif
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}

/* Return the account corresponding to the real UID of the process. */
AG_User *
AG_GetRealUser(void)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getRealUser(u) != 1) {
		AG_SetErrorV("E27", "getRealUser() failed");
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}

/* Return the account corresponding to the effective UID of the process. */
AG_User *
AG_GetEffectiveUser(void)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getEffectiveUser(u) != 1) {
		AG_SetErrorV("E28", "getEffectiveUser() failed");
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}
#endif /* AG_USER */
