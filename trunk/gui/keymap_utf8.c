/*
 * Copyright (c) 2002-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Basic keyboard input processing in UTF-8 mode.
 */

#include <config/utf8.h>
#ifdef UTF8

#include <core/core.h>
#include <core/config.h>

#include "widget.h"
#include "textbox.h"
#include "keymap.h"
#include "unicode.h"

#include <ctype.h>
#include <string.h>

#ifdef THREADS
static AG_Mutex killRingLock = AG_MUTEX_INITIALIZER;
#endif
static Uint32  *killRing = NULL;
static Uint     killRingLen = 0;

static int
InsertUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 ch,
    Uint32 *ucs4, int len, int lenMax)
{
	int unicodeKbd = SDL_EnableUNICODE(-1);
	Uint32 ins[3];
	int i, nins;
	Uint32 uch = ch;

	if (!unicodeKbd) {
		uch = AG_ApplyModifiersASCII((Uint32)keysym, keymod);
	}
	if (uch == 0) { return (0); }
	if (uch == '\r') { uch = '\n'; }

	if (AG_Bool(agConfig,"input.composition")) {
		if ((nins = AG_KeyInputCompose(tbox, uch, ins)) == 0)
			return (0);
	} else {
		ins[0] = uch;
		nins = 1;
	}
	if (len+nins >= lenMax/sizeof(Uint32)) {
		return (0);
	}
	ins[nins] = '\0';

	if (tbox->pos == len) {					/* Append */
		if (unicodeKbd) {
			for (i = 0; i < nins; i++)
				ucs4[len+i] = ins[i];
		} else {
			if (keysym == 0)
				return (0);
			for (i = 0; i < nins; i++)
				ucs4[len+i] = ins[i];
		}
#if 0
		if (tbox->xMax >= tbox->wAvail) {
			AG_TextSizeUCS4(ins, &xInc, NULL);
			tbox->x += xInc;
		}
#endif
	} else {						/* Insert */
		memmove(&ucs4[tbox->pos+nins], &ucs4[tbox->pos],
		       (len - tbox->pos)*sizeof(Uint32));
		if (unicodeKbd) {
			for (i = 0; i < nins; i++)
				ucs4[tbox->pos+i] = ins[i];
		} else {
			if (keysym == 0)
				return (0);
			for (i = 0; i < nins; i++)
				ucs4[len+i] = ins[i];
		}
	}
	ucs4[len+nins] = '\0';
	tbox->pos += nins;
	return (1);
}

static int
DeleteUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 unicode,
    Uint32 *ucs4, int len, int lenMax)
{
	Uint32 *c;

	if (len == 0)
		return (0);

	switch (keysym) {
	case SDLK_BACKSPACE:
		if (tbox->pos == 0) {
			return (0);
		}
		if (tbox->pos == len) { 
			ucs4[len-1] = '\0';
			tbox->pos--;
			return (1);
		}
		tbox->pos--;
		break;
	case SDLK_DELETE:
		if (tbox->pos == len) {
			ucs4[len-1] = '\0';
			tbox->pos--;
			return (1);
		}
		break;
	default:
		break;
	}
	for (c = &ucs4[tbox->pos]; c[1] != '\0'; c++) {
		*c = c[1];
	}
	*c = '\0';
	return (1);
}

static int
CursorEndUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	tbox->pos = len;
	return (0);
}

static int
KillUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	int lenPart;

	if ((lenPart = len - tbox->pos) <= 0)
		return (0);

	AG_MutexLock(&killRingLock);
	killRing = Realloc(killRing, lenPart*4);
	memcpy(killRing, &ucs4[tbox->pos], lenPart*4);
	killRingLen = lenPart;
	AG_MutexUnlock(&killRingLock);

	ucs4[tbox->pos] = '\0';
	return (1);
}

static int
YankUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	int nCopy;

	AG_MutexLock(&killRingLock);
	if (killRing == NULL) {
		goto nochange;
	}
	if ((tbox->pos+killRingLen)*4 >= lenMax) {
		nCopy = (lenMax-4) - (tbox->pos+killRingLen)*4;
		if (nCopy < 0)
			goto nochange;
	} else {
		nCopy = killRingLen*4;
	}

	memcpy(&ucs4[tbox->pos], killRing, nCopy);
	ucs4[tbox->pos + nCopy/4] = '\0';
	tbox->pos += nCopy/4;
	AG_MutexUnlock(&killRingLock);
	return (1);
nochange:
	AG_MutexUnlock(&killRingLock);
	return (0);
}

static int
CursorRightUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	if (tbox->pos < len) {
		tbox->pos++;
	}
	return (0);
}

static int
WordBackUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	Uint32 *c;

	if (tbox->pos > 1 && ucs4[tbox->pos-1] == ' ') {
		tbox->pos -= 2;
	}
	for (c = &ucs4[tbox->pos];
	     c > &ucs4[0] && *c != ' ';
	     c--, tbox->pos--)
		;;
	if (*c == ' ') {
		tbox->pos++;
	}
	return (0);
}

static int
WordForwUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	Uint32 *c;

	if (tbox->pos == len) {
		return (1);
	}
	if (len > 1 && ucs4[tbox->pos] == ' ') {
		tbox->pos++;
	}
	for (c = &ucs4[tbox->pos];
	     *c != '\0' && *c != ' ';
	     c++, tbox->pos++)
		;;
	return (0);
}

static int
CursorHomeUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	tbox->pos = 0;
	return (0);
}

static int
CursorLeftUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, Uint32 uch,
    Uint32 *ucs4, int len, int lenMax)
{
	if (--tbox->pos < 1) {
		tbox->pos = 0;
	}
	return (0);
}

const struct ag_keycode_utf8 agKeymapUTF8[] = {
	{ SDLK_HOME,		0,		CursorHomeUTF8 },
	{ SDLK_a,		KMOD_CTRL,	CursorHomeUTF8 },
	{ SDLK_LEFT,		0,		CursorLeftUTF8 },
	{ SDLK_BACKSPACE,	0,		DeleteUTF8 },
	{ SDLK_DELETE,		0,		DeleteUTF8 },
	{ SDLK_END,		0,		CursorEndUTF8 },
	{ SDLK_e,		KMOD_CTRL,	CursorEndUTF8 },
	{ SDLK_k,		KMOD_CTRL,	KillUTF8 },
	{ SDLK_y,		KMOD_CTRL,	YankUTF8 },
	{ SDLK_b,		KMOD_ALT,	WordBackUTF8 },
	{ SDLK_f,		KMOD_ALT,	WordForwUTF8 },
	{ SDLK_RIGHT,		0,		CursorRightUTF8 },
	{ SDLK_LAST,		0,		InsertUTF8 },
};
#endif /* UTF8 */
