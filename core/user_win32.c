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
 * User information on Windows platforms.
 */

#include <agar/core/core.h>
#ifdef AG_USER

#include <agar/config/have_cygwin.h>
#include <agar/core/win32.h>
#include <agar/core/queue_close.h>
#ifndef HAVE_CYGWIN
#include <shlobj.h>
#endif
#include <agar/core/queue_close.h>
#include <agar/core/queue.h>

static int
GetUserByUID(AG_User *_Nonnull u, Uint32 uid)
{
#ifndef HAVE_CYGWIN
	char appdata[MAX_PATH];
	char tmpdir[MAX_PATH];

	/* Set home dir */
	if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
		AG_SetError("CSIDL_APPDATA: %s", AG_Strerror(GetLastError()));
		return (-1);
	}
	if ((u->home = TryStrdup(appdata)) == NULL)
		return (-1);

	/* Set temp dir */
	if (GetTempPathA(sizeof(tmpdir), tmpdir) == 0) {
		free(u->home);
		return (-1);
	}
	if ((u->tmp = TryStrdup(tmpdir)) == NULL)
		return (-1);
#endif
	u->flags |= AG_USER_NO_ACCOUNT;			/* Not a real user */
	return (0);
}

static int
GetUserByName(AG_User *_Nonnull u, const char *_Nonnull name)
{
	return GetUserByUID(u, 0);
}

static int
GetRealUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, 0);
}

static int
GetEffectiveUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, 0);
}

const AG_UserOps agUserOps_win32 = {
	"win32",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser
};
#endif /* AG_USER */
