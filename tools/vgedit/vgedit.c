/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Vector graphics editor.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/vg.h>

#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <string.h>
#include <stdlib.h>

#include "vgedit.h"

AG_Object vfsRoot;

static void
CreateNewObject(AG_Event *_Nonnull event)
{
	AG_ObjectClass *cls = AG_PTR(1);
	AG_Object *obj;
	AG_Window *win;

	if ((obj = AG_ObjectNew(&vfsRoot, NULL, cls)) == NULL) {
		goto fail;
	}
	if ((win = AGOBJECT_CLASS(obj)->edit(obj)) == NULL) {
		goto fail;
	}
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_WindowSetCaptionS(win, _("Untitled"));
/*	AG_SetEvent(win, "window-close", WindowClose, "%p", obj); */
	AG_SetPointer(win, "object", obj);
	AG_PostEvent(obj, "edit-open", NULL);
	AG_WindowShow(win);
	return;
fail:
	AG_TextMsgFromError();
}

static void
CreateNewDlg(AG_Event *event)
{
	AG_Window *win;
	const AG_FileExtMapping *me;
	AG_Label *lbl;
	int i;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "vgedit");
	AG_SetStyle(win, "padding", "20");

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, "vgedit");
	AG_SetStyle(lbl, "font-size", "250%");
	AG_SetStyle(lbl, "text-color", "#aaeeff");
	AG_SetStyle(lbl, "font-weight", "bold");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);

	AG_LabelNewS(win, 0, _("Create New:"));

	for (i = 0; i < vgFileExtCount; i++) {
		me = &vgFileExtMap[i];
		AG_ButtonNewFn(win, AG_BUTTON_HFILL, me->descr,
		    CreateNewObject, "%p", me->cls);
	}
	
	AG_SeparatorNewHoriz(win);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Quit"), AGWINCLOSE(win));

	AG_WindowSetPosition(win, AG_WINDOW_MC, 0);
	AG_WindowShow(win);
}

static void
Quit(AG_Event *event)
{
	AG_Terminate(0);
}

static void
PrintUsage(void)
{
	printf("%s [-ASDv] [-d agar-driver] [-t font] [file ...]\n",
	    agProgName);
}

int
main(int argc, char *argv[])
{
	char *driverSpec = "<OpenGL>";
	int i, j, debug = 0, forceScalar = 0;
	const char *fontSpec = NULL;
	char *optArg = NULL;
	int optInd, c;
	AG_Event ev;
	char newObj = '\0';

#ifdef ENABLE_NLS
	bindtextdomain("vgedit", LOCALEDIR);
	bind_textdomain_codeset("vgedit", "UTF-8");
	textdomain("vgedit");
#endif
	if (AG_InitCore("vgedit", AG_CREATE_DATADIR|AG_VERBOSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "ASDvd:t:?hp:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'A':
		case 'S':
		case 'D':
			newObj = c;
			break;
		case 'v':
			{
				AG_AgarVersion av;

				AG_GetVersion(&av);
				printf("vgedit %s (agar %d.%d.%d)\n", VERSION,
				    av.major, av.minor, av.patch);
			}
			exit(0);
		case 'd':
			driverSpec = optArg;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case 'p':
			break;
		default:
			PrintUsage();
			exit(0);
		}
	}

	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1) {
		AG_Verbose("%s; exiting\n", AG_GetError());
		return (-1);
	}
	AG_ConfigLoad();

#ifdef __APPLE__
	AG_BindGlobalKeyEv(AG_KEY_Q,	AG_KEYMOD_META, Quit);
	AG_BindGlobalKey(AG_KEY_EQUALS,	AG_KEYMOD_META,	AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_MINUS,	AG_KEYMOD_META,	AG_ZoomOut);
	AG_BindGlobalKey(AG_KEY_0,	AG_KEYMOD_META,	AG_ZoomReset);
	AG_BindGlobalKey(AG_KEY_P,	AG_KEYMOD_META, AG_ViewCapture);
#else
	AG_BindGlobalKeyEv(AG_KEY_ESCAPE, AG_KEYMOD_ANY,  Quit);
	AG_BindGlobalKey(AG_KEY_EQUALS,   AG_KEYMOD_CTRL, AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_MINUS,    AG_KEYMOD_CTRL, AG_ZoomOut);
	AG_BindGlobalKey(AG_KEY_0,        AG_KEYMOD_CTRL, AG_ZoomReset);
	AG_BindGlobalKey(AG_KEY_F8,       AG_KEYMOD_ANY,  AG_ViewCapture);
#endif
	VG_InitSubsystem();

	/* Set up a VFS to contain all objects. */
	AG_ObjectInitStatic(&vfsRoot, NULL);
	AG_ObjectSetName(&vfsRoot, "vgedit");

	if (newObj != '\0') {
		const AG_ObjectClass *cls = NULL;
		AG_Object *obj;

		switch (newObj) {
		case 'A':	cls = &vgClass;			break;
#if 0
		case 'S':	cls = &vgClassSVG;		break;
		case 'D':	cls = &vgClassDXF;		break;
#endif
		}
		AG_EventArgs(&ev, "%p", cls);
		CreateNewObject(&ev);
	} else if (optInd == argc) {
		AG_EventInit(&ev);
		CreateNewDlg(&ev);
	}

	/* Load command line arguments. */
	for (i = optInd; i < argc; i++) {
		const char *fileShort = AG_ShortFilename(argv[i]);
		const AG_FileExtMapping *me = NULL;
		AG_Object *obj;
		AG_Window *win;
		char *ext;

		if ((ext = strrchr(argv[i], '.')) == NULL) {
			goto fail_unknown;
		}
		for (j = 0; j < vgFileExtCount; j++) {
			me = &vgFileExtMap[j];
			if (AG_Strcasecmp(ext, me->ext) == 0)
				break;
		}
		if (j == vgFileExtCount) {
			goto fail_unknown;
		}
		if ((obj = AG_ObjectNew(&vfsRoot, NULL, me->cls)) == NULL) {
			AG_Verbose("%s: %s\n", fileShort, AG_GetError());
			goto fail;
		}
		if (AG_ObjectLoadFromFile(obj, argv[i]) == -1) {
			AG_Verbose("%s: %s", fileShort, AG_GetError());
			goto fail;
		}
		AG_SetString(obj, "archive-path", argv[i]);
		AG_ObjectSetNameS(obj, fileShort);
		if ((win = AGOBJECT_CLASS(obj)->edit(obj)) == NULL) {
			goto fail;
		}
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		AG_WindowSetCaptionS(win, fileShort);
	
/*		AG_SetEvent(win, "window-close", WindowClose, "%p", obj); */
		AG_SetPointer(win, "object", obj);
		AG_PostEvent(obj, "edit-open", NULL);
		AG_WindowShow(win);
	}

	AG_EventLoop();
	AG_ConfigSave();
	AG_DestroyGraphics();
	VG_DestroySubsystem();
	AG_Destroy();
	return (0);
fail_unknown:
	AG_Verbose(_("%s: unknown file format\n"), argv[i]);
fail:
	AG_DestroyGraphics();
	AG_Destroy();
	return (1);
}
