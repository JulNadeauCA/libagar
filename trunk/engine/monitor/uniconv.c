/*	$Csoft: uniconv.c,v 1.2 2004/01/03 04:25:11 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/engine.h>

#ifdef DEBUG

#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>

#include <engine/unicode/unicode.h>

#include "monitor.h"

static Uint32 unitext[2] = { '\0', '\0' };
static char utf8text[256] = "";
static char bytetext[256] = "";
static Uint32 unimin = 0, unimax = 0;

static void
uniconv_update(int argc, union evarg *argv)
{
	int i;
	char *c;

	unicode_export(UNICODE_TO_UTF8, utf8text, unitext, sizeof(unitext));

	bytetext[0] = '\0';
	for (c = &utf8text[0]; *c != '\0'; c++) {
		char s[4];

		snprintf(s, sizeof(s), " %x", (unsigned char)*c);
		strlcat(bytetext, s, sizeof(bytetext));
	}
}

static void
uniconv_rangeupdate(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	Uint32 i;

	tlist_clear_items(tl);
	for (i = unimin; i < unimax; i++) {
		char text[TLIST_LABEL_MAX];
		char *c;

		if (i == 10)
			continue;

		unitext[0] = i;
		unicode_export(UNICODE_TO_UTF8, utf8text, unitext,
		    sizeof(unitext));

		bytetext[0] = '\0';
		for (c = &utf8text[0]; *c != '\0'; c++) {
			char s[4];

			snprintf(s, sizeof(s), " %x", (unsigned char)*c);
			strlcat(bytetext, s, sizeof(bytetext));
		}
		snprintf(text, sizeof(text), "%s (%s )", utf8text, bytetext);
		tlist_insert_item(tl, NULL, text, NULL);
	}
}

struct window *
uniconv_window(void)
{
	struct window *win;
	struct spinbutton *sbu;
	struct tlist *tl;
	Uint32 i;

	if ((win = window_new("uniconv")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Unicode Conversion"));
	window_set_closure(win, WINDOW_DETACH);
	
	label_polled_new(win, NULL, "%s (%s )", &utf8text, &bytetext);

	sbu = spinbutton_new(win, "Unicode: ");
	spinbutton_set_min(sbu, 0);
	spinbutton_set_max(sbu, 65537);
	widget_bind(sbu, "value", WIDGET_UINT32, &unitext[0]);
	event_new(sbu, "spinbutton-changed", uniconv_update, NULL);
	
	tl = tlist_new(win, 0);
	
	sbu = spinbutton_new(win, "Start: ");
	spinbutton_set_min(sbu, 0);
	spinbutton_set_max(sbu, 65537);
	widget_bind(sbu, "value", WIDGET_UINT32, &unimin);
	event_new(sbu, "spinbutton-changed", uniconv_rangeupdate, "%p", tl);
	
	sbu = spinbutton_new(win, "Stop: ");
	spinbutton_set_min(sbu, 0);
	spinbutton_set_max(sbu, 65537);
	widget_bind(sbu, "value", WIDGET_UINT32, &unimax);
	event_new(sbu, "spinbutton-changed", uniconv_rangeupdate, "%p", tl);

	return (win);
}

#endif	/* DEBUG */
