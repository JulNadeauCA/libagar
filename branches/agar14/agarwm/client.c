/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <dirent.h>
#include <signal.h>

#include "agarwm.h"

static void
Init(void *obj)
{
	WM_Client *wc = obj;

	wc->name[0] = '\0';
}

static void *
Edit(void *p)
{
	WM_Client *wc = p;
	AG_Window *win;
	AG_Textbox *tb;
	AG_Checkbox *cb;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Client: %s", AGOBJECT(wc)->name);
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_CENTER, 1);

	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Name: ");
	AG_TextboxBindUTF8(tb, wc->name, sizeof(wc->name));
	AG_WidgetFocus(tb);

	cb = AG_CheckboxNew(win, 0, "Write access");
	AG_WidgetBindFlag(cb, "state", &wc->flags, WM_CLIENT_DEBUG);
	return (win);
}

AG_ObjectClass wmClientClass = {
	"WM_Client",
	sizeof(WM_Client),
	{ 0, 0 },
	Init,
	NULL,		/* free */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	Edit
};
