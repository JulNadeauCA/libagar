/*
 * Copyright (c) 2010-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * General-purpose editor application for Agar SK(3) sketches.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/sg.h>
#include <agar/sk.h>

#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <string.h>

#include "skedit.h"

static void
Quit(AG_Event *event)
{
	AG_Terminate(0);
}

#if defined(AG_DEBUG) && defined(AG_TIMERS)
static void
DoDebugger(void)
{
	AG_Window *win;

	if ((win = AG_GuiDebugger(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}
#endif /* AG_DEBUG and AG_TIMERS */

static void
DoStyleEditor(void)
{
	AG_Window *win;

	if ((win = AG_StyleEditor(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int i, debug = 0, rv = 0;
	const char *fontSpec = NULL;
	const char *driverSpec = "<OpenGL>";
	char *optArg = NULL;
	int optInd;
	SK *sk;
	int c, openedFiles = 0;
	int forceScalar = 0;

#ifdef ENABLE_NLS
	bindtextdomain("skedit", LOCALEDIR);
	bind_textdomain_codeset("skedit", "UTF-8");
	textdomain("skedit");
#endif

	if (AG_InitCore("skedit", AG_CREATE_DATADIR|AG_VERBOSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "?vDsd:t:p:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'v':
			printf("skedit %s\n", VERSION);
			exit(0);
		case 'D':
			debug = 1;
			break;
		case 'd':
			driverSpec = optArg;
			break;
		case 's':
			forceScalar = 1;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case 'p':
			break;
		case '?':
		default:
			printf("%s [-vDs] [-d agar-driver] [-t font,pts]\n",
			    agProgName);
			exit(0);
		}
	}

	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_ConfigLoad();

	AG_BindStdGlobalKeys();
#if defined(AG_DEBUG) && defined(AG_TIMERS)
	AG_BindGlobalKey(AG_KEY_F7, AG_KEYMOD_ANY, DoDebugger);
	AG_BindGlobalKey(AG_KEY_D,  AGSI_CMD_MOD,  DoDebugger);
#endif
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_NONE, DoStyleEditor);
	AG_BindGlobalKey(AG_KEY_C,  AGSI_CMD_MOD,   DoStyleEditor);

	/* Initialize Agar-Math and Agar-SK */
	M_InitSubsystem();
	SK_InitSubsystem();
	
	if (forceScalar) {
		Verbose("Forcing scalar matrix/vector operations\n");
		mVecOps2 = &mVecOps2_FPU;
		mVecOps3 = &mVecOps3_FPU;
		mVecOps4 = &mVecOps4_FPU;
		mMatOps44 = &mMatOps44_FPU;
	}

	if (optInd == argc) {
		/* Create an initial scene. */
		if ((sk = SK_New(&skVfsRoot, NULL)) == NULL ||
		    SK_GUI_OpenObject(sk) == NULL) {
			rv = 1;
			goto out;
		}
	}

	/* Load command line arguments. */
	for (i = optInd; i < argc; i++) {
		AG_Event ev;
		char *ext;

		Verbose("Loading: %s\n", argv[i]);
		if ((ext = strrchr(argv[i], '.')) == NULL) {
			continue;
		}
		AG_EventInit(&ev);
		
		if (strcasecmp(ext, ".sk") == 0) {
			AG_EventPushPointer(&ev, "", &skClass);
		} else {
			Verbose("Ignoring argument: %s\n", argv[i]);
		}
		AG_EventPushString(&ev, "", argv[i]);
		SK_GUI_LoadObject(&ev);
	}

	AG_EventLoop();
out:
	AG_ConfigSave();
	AG_DestroyGraphics();
	AG_Destroy();
	return (rv);
}
