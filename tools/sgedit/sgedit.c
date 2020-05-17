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
 * General-purpose editor application for FreeSG objects.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/sg.h>

#include <agar/config/have_pthreads.h>
#include <agar/config/have_clock_gettime.h>

#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <string.h>

#include "sgedit.h"

static void
Quit(AG_Event *event)
{
	AG_Terminate(0);
}

static void
PrintUsage(void)
{
	printf("%s [-3SRTEMvDs] [-d agardrv] [-t font] [-e eyesep] [file ...]\n",
	    agProgName);
}

int
main(int argc, char *argv[])
{
	char driverSpec[128];
	int i, j, debug = 0, forceScalar = 0, forceStereo = 0;
	const char *fontSpec = NULL;
	char *optArg = NULL;
	int optInd, c;
	AG_Event ev;
	char newObj = '\0';

	Strlcpy(driverSpec, "<OpenGL>", sizeof(driverSpec));

#ifdef ENABLE_NLS
	bindtextdomain("sgedit", LOCALEDIR);
	bind_textdomain_codeset("sgedit", "UTF-8");
	textdomain("sgedit");
#endif
	if (AG_InitCore("sgedit", AG_CREATE_DATADIR|AG_VERBOSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "3SRTEMvDsd:t:e:?hp:", &optArg, &optInd)) != -1) {
		switch (c) {
		case '3':
			forceStereo = 1;
			break;
		case 'S':
		case 'R':
		case 'T':
		case 'E':
		case 'M':
			newObj = c;
			break;
		case 'v':
			printf("sgedit %s\n", VERSION);
			exit(0);
		case 'D':
			debug = 1;
			break;
		case 'd':
			Strlcpy(driverSpec, optArg, sizeof(driverSpec));
			break;
		case 's':
			forceScalar = 1;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case 'e':
			sgEyeSeparation = (M_Real)strtod(optArg, NULL);
			break;
		case 'p':
			break;
		default:
			PrintUsage();
			exit(0);
		}
	}

	if (forceStereo) {
		Verbose("Forcing stereoscopic context\n");
		Strlcat(driverSpec, "(stereo)", sizeof(driverSpec));
	}
	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1) {
		AG_Verbose("%s; exiting\n", AG_GetError());
		return (-1);
	}
#if defined(HAVE_PTHREADS) && defined(HAVE_CLOCK_GETTIME)
	if (agDriverSw != NULL) {
		/*
		 * Allow AG_Delay() to work as expected when performing
		 * multithreaded offline rendering.
		 */
		Verbose("Using agTimeOps_renderer\n");
		AG_SetTimeOps(&agTimeOps_renderer);
	}
#endif
	AG_ConfigLoad();

#ifdef __APPLE__
	AG_BindGlobalKeyEv(AG_KEY_Q,	AG_KEYMOD_META, Quit);
	AG_BindGlobalKey(AG_KEY_EQUALS,	AG_KEYMOD_META,	AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_MINUS,	AG_KEYMOD_META,	AG_ZoomOut);
	AG_BindGlobalKey(AG_KEY_0,	AG_KEYMOD_META,	AG_ZoomReset);
	AG_BindGlobalKey(AG_KEY_P,	AG_KEYMOD_META, AG_ViewCapture);
#else
	AG_BindGlobalKeyEv(AG_KEY_ESCAPE, AG_KEYMOD_ANY, Quit);
	AG_BindGlobalKey(AG_KEY_EQUALS,	AG_KEYMOD_CTRL,	AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_MINUS,	AG_KEYMOD_CTRL,	AG_ZoomOut);
	AG_BindGlobalKey(AG_KEY_0,	AG_KEYMOD_CTRL,	AG_ZoomReset);
	AG_BindGlobalKey(AG_KEY_F8,	AG_KEYMOD_ANY, AG_ViewCapture);
#endif

	/* Initialize the subsystems. */
	SG_InitSubsystem();

	if (forceScalar) {
		Verbose("Forcing scalar matrix/vector operations\n");
		mVecOps2 = &mVecOps2_FPU;
		mVecOps3 = &mVecOps3_FPU;
		mVecOps4 = &mVecOps4_FPU;
		mMatOps44 = &mMatOps44_FPU;
	}

	if (newObj != '\0') {
		void *cls = NULL;

		switch (newObj) {
		case 'S':	cls = &sgClass;			break;
		case 'R':	cls = &sgScriptClass;		break;
		case 'T':	cls = &sgTextureClass;		break;
		}
		if (cls != NULL) {
			AG_EventInit(&ev);
			AG_EventPushPointer(&ev, "", cls);
			SG_GUI_NewObject(&ev);
		}
	} else if (optInd == argc) {
		AG_EventInit(&ev);
		SG_GUI_CreateNewDlg(&ev);
	}

	/* Load command line arguments. */
	for (i = optInd; i < argc; i++) {
		const AG_FileExtMapping *me = NULL;
		char *ext;

		if ((ext = strrchr(argv[i], '.')) == NULL) {
			AG_Verbose("%s: unknown file format\n", argv[i]);
			goto fail;
		}
		for (j = 0; j < sgFileExtCount; j++) {
			me = &sgFileExtMap[j];
			if (strcasecmp(ext, me->ext) == 0)
				break;
		}
		if (j == sgFileExtCount) {
			AG_Verbose("%s: unknown file format\n", argv[i]);
			goto fail;
		}
		if (SG_GUI_LoadObject(me->cls, argv[i]) == NULL) {
			AG_Verbose("%s: %s\n", argv[i], AG_GetError());
			goto fail;
		}
	}

	AG_EventLoop();
	AG_ConfigSave();
	AG_DestroyGraphics();
	AG_Destroy();
	return (0);
fail:
	AG_DestroyGraphics();
	AG_Destroy();
	return (1);
}
