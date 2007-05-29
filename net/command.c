/*	$Csoft: command.c,v 1.2 2005/04/26 04:40:42 vedge Exp $	*/

/*
 * Copyright (c) 2003-2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <agar/config/network.h>
#ifdef NETWORK

#include <core/core.h>

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>

#include "net.h"

char *agnCommandErrorString = "";

char *
AGN_CommandString(AGN_Command *cmd, const char *key)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		AGN_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0)
			return (arg->value);
	}
	agnCommandErrorString = "missing string argument";
	return (NULL);
}

int
AGN_CommandInt(AGN_Command *cmd, const char *key, int *rv)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		AGN_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0) {
			*rv = atoi(arg->value);
			return (0);
		}
	}
	agnCommandErrorString = "missing int argument";
	return (-1);
}

int
AGN_CommandLong(AGN_Command *cmd, const char *key, long *lp)
{
	long lv;
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		AGN_CommandArg *arg = &cmd->args[i];

		if (strcmp(arg->key, key) == 0) {
			char *ep;
			
			errno = 0;
			lv = strtol((char *)arg->value, &ep, 10);
			if (((char *)arg->value)[0] == '\0' || *ep != '\0') {
				agnCommandErrorString = "long arg invalid";
				return (-1);
			}
			if (errno == ERANGE &&
			    (lv == LONG_MAX || lv == LONG_MIN)) {
				agnCommandErrorString = "long arg out of range";
				return (-1);
			}
			*lp = lv;
			return (0);
		}
	}
	agnCommandErrorString = "missing long argument";
	return (-1);
}

void
AGN_CommandCopyString(char *dst, AGN_Command *cmd, const char *key,
    size_t dstlen)
{
	char *src;

	src = AGN_CommandString(cmd, key);
	if (strlcpy(dst, src, dstlen) >= dstlen)
		agnCommandErrorString = "argument is too big";
}

void
AGN_InitCommand(AGN_Command *cmd)
{
	cmd->args = Malloc(sizeof(AGN_CommandArg), M_NETBUF);
	cmd->nargs = 0;
}

void	
AGN_DestroyCommand(AGN_Command *cmd)
{
	int i;

	for (i = 0; i < cmd->nargs; i++) {
		AGN_CommandArg *arg = &cmd->args[i];

		Free(arg->value, 0);
	}
	Free(cmd->args, M_NETBUF);
}

#endif /* NETWORK */
