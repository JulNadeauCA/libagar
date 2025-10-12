/*
 * Copyright (c) 2022-2023 Julien Nadeau Carriere <vedge@csoft.net>
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

#include "agarib.h"

#include <config/datadir.h>
#include <config/version.h>

/*
 * The Agar Interface Builder.
 *
 * Allows the developer to graphically construct sets of Agar GUI elements
 * and save them to a portable binary (*.ag) format which can be later read
 * by the developer's application.
 */

AG_Window *ibMain = NULL;				/* Main window */
AG_Menu *ibMainMenu = NULL;				/* Main menu */
AG_User *ibUser = NULL;					/* Effective user */
AG_Console *ibConsole = NULL;				/* Text console */
AG_Textbox *ibInput = NULL;				/* Input command */

static void
New_GUI(AG_Event *event)
{
	AG_Window *win;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_Verbose("%s\n", AG_GetError());
		return;
	}
	AG_WindowSetCaption(win, _("Untitled - Agar Interface Builder"));

	
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	AG_WindowShow(win);
}

static void
Load_GUI(AG_Event *event)
{
}

static void
Save_GUI(AG_Event *event)
{
}

static void
Save_GUI_As(AG_Event *event)
{
}

static void
Open_GUI(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *win;
	AG_FileDlg *fd;
	AG_User *u;

	if ((win = AG_WindowNew(0)) == NULL ||
	    (u = AG_GetEffectiveUser()) == NULL) {
		AG_Verbose("%s\n", AG_GetError());
		return;
	}
	AG_WindowSetCaption(win, _("Open GUI File"));
	AG_WindowSetPosition(win, AG_WINDOW_BC, 0);
	AG_WindowSetMinSize(win, 320, 240);

	fd = AG_FileDlgNew(win, AG_FILEDLG_EXPAND | AG_FILEDLG_MASK_EXT);

	Strlcpy(path, u->home, sizeof(path));
	Strlcat(path, "/GUIs", sizeof(path));
	if (AG_FileExists(path)) {
		AG_FileDlgSetDirectoryMRU(fd, "ag-guis-dir", path);
	} else {
		AG_FileDlgSetDirectoryMRU(fd, "ag-guis-dir", u->home);
	}
	AG_FileDlgAddType(fd, _("Agar GUI File"), "*.ag",
	    Load_GUI, "%p", win);

	AG_WindowShow(win);
}

#if defined(AG_DEBUG) && defined(AG_TIMERS)
static void
Run_GUI_Debugger(AG_Event *event)
{
	AG_Window *win;

	if ((win = (AG_Window *) AG_GuiDebugger(agWindowFocused)) == NULL) {
		AG_Verbose("GUI Debugger: %s\n", AG_GetError());
		return;
	}
	AG_WindowShow(win);
}
#endif /* AG_DEBUG and AG_TIMERS */

static void
PrintVersion(void)
{
	AG_AgarVersion av;
	AG_ConsoleLine *cl;
	AG_Color c;
		
	cl = AG_ConsoleMsg(ibConsole, _("Agar Interface Builder, version %s"),
	    VERSION);
	AG_ColorRGB_8(&c, 0,170,170);
	AG_ConsoleMsgColor(cl, &c);

	AG_GetVersion(&av);
	cl = AG_ConsoleMsg(ibConsole, "LibAgar version %d.%d.%d "
	    "(" AGSI_LEAGUE_SPARTAN "%s" AGSI_RST ")",
	    av.major, av.minor, av.patch,
	    av.release);
	AG_ColorRGB_8(&c, 0,170,0);
	AG_ConsoleMsgColor(cl, &c);
}

int
main(int argc, char *argv[])
{
	char *optArg = NULL, *driverSpec = NULL;
	AG_MenuItem *mi;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	Uint initFlags = AG_VERBOSE;
	int optInd, i, c;

#ifdef ENABLE_NLS
	bindtextdomain("agarib", LOCALEDIR);
	bind_textdomain_codeset("agarib", "UTF-8");
	textdomain("agarib");
#endif
	while ((c = AG_Getopt(argc, argv, "Dqd:?h", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'D':
			agDebugLvl = 1;
			break;
		case 'q':
			agDebugLvl = 0;
			initFlags &= ~(AG_VERBOSE);
			break;
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agarib [-Dq] [-d driver] [file ...]\n");
			return (1);
		}
	}
	if (AG_InitCore("agarib", initFlags) == -1) {
		return (1);
	}
	if ((ibUser = AG_GetEffectiveUser()) == NULL)
		goto fail_core;

	if (AG_InitGraphics(driverSpec) == -1)
		goto fail_core;

	if ((ibMain = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		goto fail_gfx;
	}

	AG_WindowSetCaption(ibMain, _("Agar Interface Builder"));

	ibMainMenu = (agDriverSw) ? AG_MenuNewGlobal(0) :
                                    AG_MenuNew(ibMain, AG_MENU_HFILL);

	mi = AG_MenuNode(ibMainMenu->root, ("File"), NULL);
	{
		AG_MenuItem *miTmpl;

		AG_MenuActionKb(mi, _("New GUI"),
		    agIconDoc.s,
		    AG_KEY_N, AG_KEYMOD_CTRL,
		    New_GUI, NULL);

		miTmpl = AG_MenuNode(mi, _("New GUI From Template"), agIconDoc.s);
		{
			char path[AG_FILENAME_MAX];

			AG_MenuActionKb(mi, _("Open..."),
			    agIconLoad.s,
			    AG_KEY_O, AG_KEYMOD_CTRL,
			    Open_GUI, NULL);

			Strlcpy(path, ibUser->home, sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));
			Strlcat(path, "Templates", sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));

			if (AG_FileExists(path)) {
				/* TODO */
			} else {
				AG_MenuNode(miTmpl,
				    AG_Printf(_("( Missing template directory "
				              AGSI_LEAGUE_GOTHIC "%s" AGSI_RST " )"),
				              path),
				    NULL);
			}
		}

		AG_MenuActionKb(mi, _("Save"),
		    agIconSave.s,
		    AG_KEY_S, AG_KEYMOD_CTRL,
		    Save_GUI, NULL);

		AG_MenuActionKb(mi, _("Save As..."),
		    agIconSave.s,
		    AG_KEY_S, AG_KEYMOD_SHIFT|AG_KEYMOD_CTRL,
		    Save_GUI_As, NULL);

		AG_MenuSeparator(mi);

		AG_MenuActionKb(mi, _("GUI Debugger..."), agIconMagnifier.s,
		    AG_KEY_F7, 0,
		    Run_GUI_Debugger, NULL);

		AG_MenuSeparator(mi);

		AG_MenuActionKb(mi, _("Quit"), agIconClose.s,
		    AG_KEY_Q, AG_KEYMOD_CTRL,
		    AGWINCLOSE(ibMain));		/* Close main window */
	}

	mi = AG_MenuNode(ibMainMenu->root, ("Help"), NULL);
	{
		AG_MenuActionKb(mi, _("Contents"), agIconDoc.s,
		    AG_KEY_F1, AG_KEYMOD_NONE,
		    NULL, NULL);
		AG_MenuAction(mi, _("About Agar GUI"), agIconSmallArrowRight.s,
		    AG_About, NULL);
	}

	nb = AG_NotebookNew(ibMain, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Console"), AG_BOX_VERT);
	{
		AG_Box *hb;

		ibConsole = AG_ConsoleNew(nt, AG_CONSOLE_EXPAND);

		hb = AG_BoxNewHoriz(nt, AG_BOX_HFILL);
		AG_SetFontWeight(hb, "bold");
		AG_SetFontSize(hb, "120%");
		{
			ibInput = AG_TextboxNew(hb,
			    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
			    "> ");
//			AG_SetEvent(ibInput, "textbox-return", ExecCmd, NULL);
		}

//		AG_ActionFn(ibConsole, "Help", ConsoleHelp, NULL);
//		AG_ActionOnKey(ibConsole, AG_KEY_F1, AG_KEYMOD_ANY, "Help");
//		AG_ActionOnKey(ibConsole, AG_KEY_HELP, AG_KEYMOD_ANY, "Help");

		AG_WidgetFocus(ibInput);
	}
	
	PrintVersion();


	AG_WindowShow(ibMain);
	AG_WindowSetMinSize(ibMain, 320, 200);
	AG_WindowSetGeometryAligned(ibMain, AG_WINDOW_BC, 500, 300);
	AG_EventLoop();

	AG_UserFree(ibUser);
	AG_DestroyGraphics();
	AG_Destroy();
	return (0);
fail_gfx:
	AG_DestroyGraphics();
fail_core:
	AG_Destroy();
	return (1);
}
