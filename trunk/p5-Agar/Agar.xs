/*
 * Copyright (c) 2008-2009 Julien Nadeau (vedge@hypertriton.com)
 * Copyright (c) 2009 Mat Sutcliffe (oktal@gmx.co.uk)
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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

extern XS(boot_Agar__Object);
extern XS(boot_Agar__Widget);
extern XS(boot_Agar__Event);
extern XS(boot_Agar__PixelFormat);
extern XS(boot_Agar__Surface);
extern XS(boot_Agar__Window);
extern XS(boot_Agar__Config);
extern XS(boot_Agar__Font);
extern XS(boot_Agar__Colors);
extern XS(boot_Agar__Text);
extern XS(boot_Agar__Box);
extern XS(boot_Agar__Button);
extern XS(boot_Agar__Checkbox);
extern XS(boot_Agar__Combo);
extern XS(boot_Agar__Console);
extern XS(boot_Agar__Editable);
extern XS(boot_Agar__FileDlg);
extern XS(boot_Agar__Fixed);
extern XS(boot_Agar__Label);
extern XS(boot_Agar__Menu);
extern XS(boot_Agar__MPane);
extern XS(boot_Agar__Notebook);
extern XS(boot_Agar__Numerical);
extern XS(boot_Agar__Pane);
extern XS(boot_Agar__Pixmap);
extern XS(boot_Agar__ProgressBar);
extern XS(boot_Agar__Radio);
extern XS(boot_Agar__Scrollbar);
extern XS(boot_Agar__Scrollview);
extern XS(boot_Agar__Separator);
extern XS(boot_Agar__Slider);
extern XS(boot_Agar__Textbox);
extern XS(boot_Agar__Tlist);
extern XS(boot_Agar__Toolbar);
extern XS(boot_Agar__UCombo);

MODULE = Agar		PACKAGE = Agar		PREFIX = AG_
PROTOTYPES: DISABLE

BOOT:
boot_Agar__Object(aTHX_ cv);
boot_Agar__Widget(aTHX_ cv);
boot_Agar__Event(aTHX_ cv);
boot_Agar__PixelFormat(aTHX_ cv);
boot_Agar__Surface(aTHX_ cv);
boot_Agar__Window(aTHX_ cv);
boot_Agar__Config(aTHX_ cv);
boot_Agar__Font(aTHX_ cv);
boot_Agar__Colors(aTHX_ cv);
boot_Agar__Text(aTHX_ cv);
boot_Agar__Box(aTHX_ cv);
boot_Agar__Button(aTHX_ cv);
boot_Agar__Checkbox(aTHX_ cv);
boot_Agar__Combo(aTHX_ cv);
boot_Agar__Console(aTHX_ cv);
boot_Agar__Editable(aTHX_ cv);
boot_Agar__FileDlg(aTHX_ cv);
boot_Agar__Fixed(aTHX_ cv);
boot_Agar__Label(aTHX_ cv);
boot_Agar__Menu(aTHX_ cv);
boot_Agar__MPane(aTHX_ cv);
boot_Agar__Notebook(aTHX_ cv);
boot_Agar__Numerical(aTHX_ cv);
boot_Agar__Pane(aTHX_ cv);
boot_Agar__Pixmap(aTHX_ cv);
boot_Agar__ProgressBar(aTHX_ cv);
boot_Agar__Radio(aTHX_ cv);
boot_Agar__Scrollbar(aTHX_ cv);
boot_Agar__Scrollview(aTHX_ cv);
boot_Agar__Separator(aTHX_ cv);
boot_Agar__Slider(aTHX_ cv);
boot_Agar__Textbox(aTHX_ cv);
boot_Agar__Tlist(aTHX_ cv);
boot_Agar__Toolbar(aTHX_ cv);
boot_Agar__UCombo(aTHX_ cv);

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
InitCore(progName, ...)
	const char *progName
PREINIT:
	const AP_FlagNames flagNames[] = {
		{ "verbose", AG_VERBOSE },
		{ NULL,      0 }
	};
	Uint flags = 0;
CODE:
	if (items == 2) {
		if (SvTYPE(SvRV(ST(1))) != SVt_PVHV) {
			Perl_croak(aTHX_ "Usage: Agar::InitCore(progName,[{opts}])");
		}
		AP_MapHashToFlags(SvRV(ST(1)), flagNames, &flags);
	} else if (items != 1) {
		Perl_croak(aTHX_ "Usage: Agar::InitCore(progName,[{opts}])");
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
	if ((RETVAL = AG_WidgetFind(agDriverSw, name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Object
FindObject(name)
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
PromptMsg(text, coderef)
	const char * text
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AG_TextPromptString(text, AP_EventHandlerDecRef, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: Agar::PromptText(text,coderef)");
	}

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

