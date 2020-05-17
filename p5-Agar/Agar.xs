/*
 * Copyright (c) 2009 Mat Sutcliffe (oktal@gmx.co.uk)
 * Copyright (c) 2008-2019 Julien Nadeau Carriere (vedge@csoft.net)
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
 * Perl interface to the Agar GUI toolkit.
 */
#ifdef PERL_CAPI
#define WIN32IO_IS_STDIO
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

MODULE = Agar	PACKAGE = Agar		PREFIX = AG_
PROTOTYPES : DISABLE

INCLUDE: Agar/Box.xs
INCLUDE: Agar/Button.xs
INCLUDE: Agar/Checkbox.xs
INCLUDE: Agar/Combo.xs
INCLUDE: Agar/Console.xs
INCLUDE: Agar/Editable.xs
INCLUDE: Agar/FileDlg.xs
INCLUDE: Agar/Fixed.xs
INCLUDE: Agar/Label.xs
INCLUDE: Agar/MPane.xs
INCLUDE: Agar/Menu.xs
INCLUDE: Agar/Notebook.xs
INCLUDE: Agar/Numerical.xs
INCLUDE: Agar/Pane.xs
INCLUDE: Agar/Pixmap.xs
INCLUDE: Agar/ProgressBar.xs
INCLUDE: Agar/Radio.xs
INCLUDE: Agar/Scrollbar.xs
INCLUDE: Agar/Scrollview.xs
INCLUDE: Agar/Separator.xs
INCLUDE: Agar/Slider.xs
INCLUDE: Agar/Textbox.xs
INCLUDE: Agar/Tlist.xs
INCLUDE: Agar/Toolbar.xs
INCLUDE: Agar/UCombo.xs

INCLUDE: Agar/PixelFormat.xs
INCLUDE: Agar/Surface.xs
INCLUDE: Agar/Text.xs
INCLUDE: Agar/Font.xs

INCLUDE: Agar/Window.xs
INCLUDE: Agar/Widget.xs
INCLUDE: Agar/Config.xs
INCLUDE: Agar/Object.xs
INCLUDE: Agar/Event.xs

MODULE = Agar	PACKAGE = Agar		PREFIX = AG_
PROTOTYPES : DISABLE

SV *
Version()
PREINIT:
	AG_AgarVersion v;
CODE:
	AG_GetVersion(&v);
	RETVAL = newSVpvf("%d.%d.%d", v.major, v.minor, v.patch);
OUTPUT:
	RETVAL

SV *
Release()
PREINIT:
	AG_AgarVersion v;
CODE:
	AG_GetVersion(&v);
	RETVAL = newSVpv(v.release,0);
OUTPUT:
	RETVAL

void
SetError(msg)
	const char *msg
CODE:
	AG_SetError("%s", msg);

SV *
GetError()
CODE:
	RETVAL = newSVpv(AG_GetError(),0);
	SvUTF8_on(RETVAL);
OUTPUT:
	RETVAL

int
InitCore(...)
PREINIT:
	const AP_FlagNames flagNames[] = {
		{ "verbose",		AG_VERBOSE },
		{ "createDataDir",	AG_CREATE_DATADIR },
		{ "softTimers",		AG_SOFT_TIMERS },
		{ NULL,			0 }
	};
	const char *progName = NULL;
	Uint flags = 0;
CODE:
	if (items == 2) {
		if (SvTYPE(SvRV(ST(1))) != SVt_PVHV) {
			Perl_croak(aTHX_ "Usage: Agar::InitCore([progName],[{opts}])");
		}
		AP_MapHashToFlags(SvRV(ST(1)), flagNames, &flags);
	} else if (items == 1) {
		progName = SvPV_nolen(ST(0));
	}
	RETVAL = (AG_InitCore(progName, flags) == 0);
OUTPUT:
	RETVAL

int
InitGraphics(...)
PREINIT:
	const char *driverSpec = NULL;
CODE:
	if (items == 1) {
		driverSpec = SvPV_nolen(ST(0));
	} else if (items > 1) {
		Perl_croak(aTHX_ "Usage: Agar::InitGraphics([driverSpec])");
	}
	RETVAL = (AG_InitGraphics(driverSpec) == 0);
OUTPUT:
	RETVAL

int
Resize(w, h)
	int w
	int h
CODE:
	RETVAL = AG_ResizeDisplay(w, h);
OUTPUT:
	RETVAL

int
SetRefreshRate(fps)
	int fps
CODE:
	RETVAL = AG_SetRefreshRate(fps);
OUTPUT:
	RETVAL

void
EventLoop()
CODE:
	AG_EventLoop();

void
Terminate(exitCode)
	int exitCode
CODE:
	AG_Terminate(exitCode);

Agar::Config
GetConfig()
CODE:
	RETVAL = agConfig;
OUTPUT:
	RETVAL

Agar::Widget
FindWidget(name)
	const char * name
CODE:
	if ((RETVAL = AG_ObjectFindS(AGOBJECT(agDriverSw), name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Widget
FindWidgetAtPoint(x, y)
	int x
	int y
CODE:
	if ((RETVAL = AG_WidgetFindPoint("*", x, y)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Widget
FindWidgetOfClassAtPoint(class, x, y)
	const char * class
	int x
	int y
PREINIT:
	char *cClass;
	int perlStyle;
CODE:
	if ((perlStyle = strnEQ(class, "Agar::", 6))) {
		cClass = savepv(class + 3);
		cClass[0] = 'A'; cClass[1] = 'G'; cClass[2] = '_';
	} else {
		cClass = (char *) class;
	}
	if ((RETVAL = AG_WidgetFindPoint(cClass, x, y)) == NULL) {
		if (perlStyle) { Safefree(cClass); }
		XSRETURN_UNDEF;
	}
	if (perlStyle) { Safefree(cClass); }
OUTPUT:
	RETVAL

void
InfoMsg(text)
	const char * text
CODE:
	AG_TextMsgS(AG_MSG_INFO, text);

void
WarningMsg(text)
	const char * text
CODE:
	AG_TextMsgS(AG_MSG_WARNING, text);

void
ErrorMsg(text)
	const char * text
CODE:
	AG_TextMsgS(AG_MSG_ERROR, text);

void
InfoMsgTimed(ms, text)
	Uint32 ms
	const char * text
CODE:
	AG_TextTmsgS(AG_MSG_INFO, ms, text);

void
WarningMsgTimed(ms, text)
	Uint32 ms
	const char * text
CODE:
	AG_TextTmsgS(AG_MSG_WARNING, ms, text);

void
ErrorMsgTimed(ms, text)
	Uint32 ms
	const char * text
CODE:
	AG_TextTmsgS(AG_MSG_ERROR, ms, text);

void
InfoMsgIgnorable(key, text)
	const char * key
	const char * text
CODE:
	AG_TextInfoS(key, text);

void
WarningMsgIgnorable(key, text)
	const char * key
	const char * text
CODE:
	AG_TextWarningS(key, text);

void
DESTROY()
CODE:
	AG_Destroy();

void
Quit()
CODE:
	AG_Quit();

void
QuitGUI()
CODE:
	AG_QuitGUI();

void
BindStdGlobalKeys()
CODE:
	AG_BindStdGlobalKeys();

