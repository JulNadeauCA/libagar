/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/ag_network.h>
#ifdef AG_NETWORK

#include "core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "net_command.h"

char *nsCommandErrorString = "";

char *
NS_CommandString(NS_Command *cmd, const char *key)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		NS_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0)
			return (arg->value);
	}
	nsCommandErrorString = "missing string argument";
	return (NULL);
}

int
NS_CommandInt(NS_Command *cmd, const char *key, int *rv)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		NS_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0) {
			*rv = atoi(arg->value);
			return (0);
		}
	}
	nsCommandErrorString = "missing int argument";
	return (-1);
}

int
NS_CommandLong(NS_Command *cmd, const char *key, long *lp)
{
	long lv;
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		NS_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0) {
			char *ep;
			
			lv = strtol((char *)arg->value, &ep, 10);
			if (((char *)arg->value)[0] == '\0' || *ep != '\0') {
				nsCommandErrorString = "long arg invalid";
				return (-1);
			}
			*lp = lv;
			return (0);
		}
	}
	nsCommandErrorString = "missing long argument";
	return (-1);
}

void
NS_CommandCopyString(char *dst, NS_Command *cmd, const char *key,
    size_t dstlen)
{
	char *src;

	src = NS_CommandString(cmd, key);
	if (Strlcpy(dst, src, dstlen) >= dstlen)
		nsCommandErrorString = "argument is too big";
}

void
NS_InitCommand(NS_Command *cmd)
{
	cmd->args = Malloc(sizeof(NS_CommandArg));
	cmd->nargs = 0;
}

void	
NS_DestroyCommand(NS_Command *cmd)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		NS_CommandArg *arg = &cmd->args[i];

		Free(arg->value);
	}
	Free(cmd->args);
}

#endif /* AG_NETWORK */
