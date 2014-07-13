/*
 * Copyright (c) 2014 Hypertriton, Inc. <http://hypertriton.com/>
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
 * User information on the Xbox console.
 */

#include <agar/core/core.h>

static int
GetUserByUID(AG_User *u, Uint32 uid)
{
	if (AG_XBOX_DriveIsMounted('T')) {
		if ((u->home = TryStrdup("T:\\")) == NULL ||
		    (u->tmp = TryStrdup("T:\\temp")) == NULL)
			return (-1);
	} else {
		if ((u->home = TryStrdup("D:\\")) == NULL ||
		    (u->tmp = TryStrdup("D:\\temp")) == NULL)
			return (-1);
	}
	u->flags |= AG_USER_NO_ACCOUNT;			/* Not a real user */
	return (0);
}

static int
GetUserByName(AG_User *u, const char *name)
{
	return GetUserByUID(u, 0);
}

static int
GetRealUser(AG_User *u)
{
	return GetUserByUID(u, 0);
}

static int
GetEffectiveUser(AG_User *u)
{
	return GetUserByUID(u, 0);
}

const AG_UserOps agUserOps_xbox = {
	"xbox",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser
};
