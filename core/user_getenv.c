/*
 * Copyright (c) 2018-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Generate user information from common environment variables. We assume
 * that $USER, $HOME, $UID, $EUID are correct. This module presents $USER
 * as the sole user which can be queried.
 *
 * Therefore unlike modules such as user_posix.c, we avoid a potentially slow
 * password-database access on startup since all Agar really needs to know for
 * its own purposes is $HOME.
 */

#include <agar/config/have_getenv.h>
#ifdef HAVE_GETENV

#include <agar/core/core.h>
#ifdef AG_USER

#include <stdlib.h>

static int
GetUserByName(AG_User *_Nonnull u, const char *_Nonnull name)
{
	char *my_name = getenv("USER");
	char *my_home = getenv("HOME");
	char *s;

	if (my_name == NULL || my_home == NULL) {
		AG_SetErrorS("$USER or $HOME is not set");
		return (-1);
	}
	if (strcmp(my_name, name) != 0) {
		return (0);
	}
	Strlcpy(u->name, my_name, sizeof(u->name));
	if ((s = getenv("EUID")) == NULL) { s = getenv("UID"); }
	u->uid = (s != NULL) ? (Uint32)strtoul(s, NULL, 10) : 0;
	u->gid = 0;
	u->loginClass = NULL;
	u->gecos = NULL;
	u->home = TryStrdup(my_home);
	u->tmp = TryStrdup((s = getenv("TMPDIR")) != NULL ? s : "/tmp");
	return (1);
}

static int
GetUserByUID(AG_User *_Nonnull u, Uint32 uid)
{
	char *s;
	Uint32 my_uid;

	if ((s = getenv("EUID")) == NULL) { s = getenv("UID"); }
	my_uid = (s != NULL) ? (Uint32)strtoul(s, NULL, 10) : 0;
	if (my_uid != uid) {
		return (0);			/* User not found */
	}
	if ((s = getenv("USER")) == NULL) {
		AG_SetErrorS("$USER is not set");
		return (-1);
	}
	return GetUserByName(u, s);
}

static int
GetRealOrEffectiveUser(AG_User *_Nonnull u)
{
	char *s;

	if ((s = getenv("USER")) == NULL) {
		AG_SetErrorS("$USER is not set");
		return (-1);
	}
	return GetUserByName(u, s);
}

const AG_UserOps agUserOps_env = {
	"env",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealOrEffectiveUser,
	GetRealOrEffectiveUser
};

#endif /* AG_USER */
#endif /* HAVE_GETENV */
