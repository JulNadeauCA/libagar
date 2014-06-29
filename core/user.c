/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
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
