/*
 * Copyright (c) 2014-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Obtain user information from getpwnam() or getpwnam_r().
 */

#include <agar/core/core.h>
#ifdef AG_USER

#include <unistd.h>
#include <pwd.h>

#include <agar/config/have_getpwnam_r.h>
#include <agar/config/have_getenv.h>

static void
ConvertUserData(AG_User *_Nonnull u, const struct passwd *_Nonnull pw)
{
	Strlcpy(u->name, pw->pw_name, sizeof(u->name));
	u->uid = (Uint32)pw->pw_uid;
	u->gid = (Uint32)pw->pw_gid;
#if 0
	u->loginClass = TryStrdup(pw->pw_class);
	u->gecos = TryStrdup(pw->pw_gecos);
#else
	u->loginClass = NULL;
	u->gecos = NULL;
#endif
	u->home = TryStrdup(pw->pw_dir);
	u->shell = TryStrdup(pw->pw_shell);

#ifdef HAVE_GETENV
	{
		char *s;
		if ((s = getenv("TMPDIR")) != NULL && s[0] != '\0') {
			u->tmp = TryStrdup(s);
		} else {
			u->tmp = TryStrdup("/tmp");
		}
	}
#else
	u->tmp = TryStrdup("/tmp");
#endif
}

static int
GetUserByName(AG_User *_Nonnull u, const char *_Nonnull name)
{
#ifdef HAVE_GETPWNAM_R
	struct passwd pwStorage, *pwRes;
	char *buf;
	size_t bufSize;
	int rv;

	bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufSize == -1) {
		bufSize = 16384;
	}
	if ((buf = TryMalloc(bufSize)) == NULL) {
		return (-1);
	}
	rv = getpwnam_r(name, &pwStorage, buf, bufSize, &pwRes);
	if (pwRes == NULL) {
		if (rv == 0) {
			return (0);		/* User not found */
		} else {
			AG_SetError("getpwnam_r: %s", strerror(rv));
			return (-1);
		}
	}
	ConvertUserData(u, &pwStorage);
	free(buf);
#else
	struct passwd *pw = getpwnam(name);
	if (pw == NULL) {
		return (0);
	}
	ConvertUserData(u, pw);
#endif
	return (1);
}

static int
GetUserByUID(AG_User *_Nonnull u, Uint32 uid)
{
#ifdef HAVE_GETPWNAM_R
	struct passwd pwStorage, *pwRes;
	char *buf;
	long bufSize;
	int rv;

	bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufSize == -1) {
		bufSize = 16384;
	}
	if ((buf = TryMalloc((AG_Size)bufSize)) == NULL) {
		return (-1);
	}
	rv = getpwuid_r((uid_t)uid, &pwStorage, buf, bufSize, &pwRes);
	if (pwRes == NULL) {
		if (rv == 0) {
			return (0);		/* User not found */
		} else {
			AG_SetError("getpwnam_r: %s", strerror(rv));
			return (-1);
		}
	}
	ConvertUserData(u, &pwStorage);
	free(buf);
#else
	struct passwd *pw = getpwuid((uid_t)uid);
	if (pw == NULL) {
		return (0);
	}
	ConvertUserData(u, pw);
#endif
	return (1);
}

static int
GetRealUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, (Uint32)getuid());
}

static int
GetEffectiveUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, (Uint32)geteuid());
}

const AG_UserOps agUserOps_posix = {
	"posix",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser,
};
#endif /* AG_USER */
