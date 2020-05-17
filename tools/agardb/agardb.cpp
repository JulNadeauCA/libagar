/*
 * Copyright (c) 2019-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * agardb: The Agar debugger.
 *
 * A general-purpose debugger built on top of LLDB (http://lldb.llvm.org/)
 * with an Agar GUI and added features for debugging programs which are built
 * against an instrumented Agar library.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include "config/datadir.h"
#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>

#include <atomic>
#include <csignal>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <string>
#include <cstdio>

#include "agardb.h"

using namespace lldb;
#ifdef HAVE_LLDB_UTILITY
using namespace lldb_private;
#endif

static Agardb      *g_agardb = NULL;
static Agardb::GUI *g_agardb_gui = NULL;
static const char  *g_font_spec = NULL;		/* Default font (-t) */
static const char  *g_driver_spec = NULL;	/* AG_Driver(3) spec (-d) */
static const char  *g_default_arch = NULL;	/* Architecture (-a) */
static const char  *g_core_file = NULL;		/* Core dump to load (-c) */
static std::string  g_process_name;		/* Process to attach by name */
static int          g_process_wait_for = 0;	/* --waitfor */
static int          g_set_use_color = 1;	/* Use color */
static char        *g_exec_file = NULL;		/* Executable (-f) */
static std::vector<std::string> g_exec_args;	/* Arguments to executable */

static AG_Window   *g_main_window = NULL;	/* The main window */
static AG_Console  *g_console = NULL;		/* The GUI console */
static AG_Menu     *g_menu = NULL;		/* The main menu */
static AG_Textbox  *g_textbox_prompt = NULL;	/* "agardb>" prompt */

static const struct {
	const char *cfgKey;
	const char *descr;
} g_interp_opts[] = {
	{ "lldb.stopOnCont",   N_("Stop on Continue") },
	{ "lldb.stopOnError",  N_("Stop on Error") },
	{ "lldb.stopOnCrash",  N_("Stop on Crash") },
	{ "lldb.echoCommands", N_("Echo Commands") },
	{ "lldb.echoCommCmds", N_("Echo Comment Commands") },
	{ "lldb.printResults", N_("Print Results") }
};

//static bool g_old_stdin_termios_is_valid = false;
//static struct termios g_old_stdin_termios;

/*
 * Our LLDB debugger instance.
 */
Agardb::Agardb() :
    SBBroadcaster("Agardb"),
    m_debugger(SBDebugger::Create(false))
{
	/*
	 * We want to be able to handle CTRL+D in the terminal to
	 * have it terminate certain input.
	 */
	m_debugger.SetCloseInputOnEOF(false);
	g_agardb = this;
}
Agardb::~Agardb() {
	g_agardb = NULL;
}

static void
FatalError(void)
{
	Verbose("Fatal error, exiting\n");
	SBDebugger::Terminate();
	AG_Destroy();
	exit(1);
}

/* Prepare commands for lldb sourcing */
static ::FILE *
Open_LLDB_Pipe(const char *cmdsData, size_t cmdsSize, int fds[2])
{
	enum PIPES { READ, WRITE };   /* Constants 0 and 1 for READ and WRITE */
	::FILE *cmdsPipe = NULL;
	ssize_t nrwr;

	fds[0] = -1;
	fds[1] = -1;
	int err = 0;
#ifdef _WIN32
	err = _pipe(fds, cmdsSize, O_BINARY);
#else
	err = pipe(fds);
#endif
	if (err != 0) {
		AG_SetError("LLDB command pipe: %s", AG_Strerror(errno));
		return (NULL);
	}
	nrwr = write(fds[WRITE], cmdsData, cmdsSize);
	if (nrwr < 0) {
		Verbose("write(%i, %p, %" PRIu64 ") failed (errno = %i) "
		        "trying to open lldb commands pipe\n",
		    fds[WRITE], static_cast<const void *>(cmdsData),
		    static_cast<uint64_t>(cmdsSize), errno);
	} else if (static_cast<size_t>(nrwr) == cmdsSize) {
		/*
		 * Close the write end of the pipe so when we give the read
		 * end to the debugger/command interpreter, it will exit on EOF.
		 */
#ifdef _WIN32
		_close(fds[WRITE]);
#else
		close(fds[WRITE]);
#endif
		fds[WRITE] = -1;

		/*
		 * Now open the read file descriptor in a FILE * that
		 * we can give to the debugger as an input handle.
		 */
		if ((cmdsPipe = fdopen(fds[READ], "r")) != NULL) {
			/*
			 * cmdsPipe now owns the read descriptor.
			 * Hand ownership of the FILE * over to the
			 * debugger.
			 */
			fds[READ] = -1;
		} else {
			Verbose("fdopen(%i, \"r\") failed (errno = %i) when "
			        "trying to open lldb commands pipe\n",
			    fds[READ], errno);
		}
	}
	return (cmdsPipe);
}

static void
Close_LLDB_Pipe(int fds[2])
{
	enum PIPES { READ, WRITE };

	if (fds[WRITE] != -1) {
#ifdef _WIN32
		_close(fds[WRITE]);
#else
		close(fds[WRITE]);
#endif
		fds[WRITE] = -1;
	}

	if (fds[READ] != -1) {
#ifdef _WIN32
		_close(fds[READ]);
#else
		close(fds[READ]);
#endif
		fds[READ] = -1;
	}
}

/*
 * "agar version": Print the libagar version number.
 */
class AgarVersionCommand : public lldb::SBCommandPluginInterface {
public:
	virtual bool DoExecute(lldb::SBDebugger debugger, char **command,
	                       lldb::SBCommandReturnObject &result) {
		AG_AgarVersion av;

		AG_GetVersion(&av);
		result.Printf("agardb is running agar %d.%d.%d (\"%s\")\n",
		    av.major, av.minor, av.patch, av.release);
#if 0
		if (command == NULL) {
			return (false);
		}
		const char *arg = *command;

		while (arg != NULL) {
			result.Printf("%s", arg);
			arg = *(++command);
		}
#endif
		return (true);
	}
};

/*
 * "agar debug": Print or set the Agar debug level.
 */
class AgarDebugCommand : public lldb::SBCommandPluginInterface {
public:
	virtual bool DoExecute(lldb::SBDebugger debugger, char **command,
	                       lldb::SBCommandReturnObject &result) {
		if (command == NULL || *command == NULL) {
			result.Printf("agDebugLvl: %d\n", agDebugLvl);
			return (true);
		} else {
			int newLvl = atoi(*command);
			result.Printf("agDebugLvl %d -> %d\n", agDebugLvl, newLvl);
			agDebugLvl = newLvl;
		}
		return (true);
	}
};

/*
 * "agar driver": Query the available AG_Driver classes.
 */
class AgarDriverCommand : public lldb::SBCommandPluginInterface {
public:
	virtual bool DoExecute(lldb::SBDebugger debugger, char **command,
	                       lldb::SBCommandReturnObject &result)
	{
		const AG_DriverClass *drvClass = AGWIDGET(g_main_window)->drvOps;

		if (command != NULL && (*command) != NULL) {
			AG_DriverClass **pd;
		
			if (strcasecmp(*command, "list") == 0) {
				for (pd = &agDriverList[0]; *pd != NULL; pd++) {
					AG_DriverClass *dc = *pd;

					result.Printf("%p: %s ( %s(3) )\n",
					    *pd, dc->name, AGCLASS(dc)->name);
				}
				return (true);
			}
			for (pd = &agDriverList[0]; *pd != NULL; pd++) {
				AG_DriverClass *dc = *pd;

				if (strcasecmp(dc->name, *command) == 0 ||
				    strcasecmp(AGCLASS(dc)->name, *command) == 0)
					break;
			}
			if (*pd != NULL) {
				drvClass = *pd;
				result.Printf(
				    _("The %s(3) driver is available in this build.\n"),
				    AGCLASS(drvClass)->name);
			} else {
				result.Printf("No such driver \"%s\"\n", *command);
				return (false);
			}
		} else {
			result.Printf(_("agardb is running %s(3)\n"), AGCLASS(drvClass)->name);
		}
		result.Printf("[%s]\n", AGCLASS(drvClass)->name);
		result.Printf(_("Driver Class: %s\n"), AGCLASS(drvClass)->name);
		result.Printf(_("Rendering method: %s\n"), _(agDriverTypeNames[drvClass->type]));
		result.Printf(_("Windowing method: %s\n"), _(agDriverWmTypeNames[drvClass->wm]));
		result.Printf(_("Capabilities: %s%s%s\n"),
		    (drvClass->flags & AG_DRIVER_OPENGL) ? "GL " : "",
		    (drvClass->flags & AG_DRIVER_SDL) ? "SDL " : "",
		    (drvClass->flags & AG_DRIVER_TEXTURES) ? "TEXTURES " : "");

		return (true);
	}
};

/*
 * Forward a set of commands to LLDB.
 */
void
Agardb::Run_LLDB(SBStream &cmds, bool async)
{
	lldb::SBDebugger &db = g_agardb->GetDebugger();
	const char *cmdsData = cmds.GetData();
	const size_t cmdsSize = cmds.GetSize();
  	bool doHandleEvents = true;
	bool quitRequested = false;
	bool stoppedForCrash = false;
	int cmdsFds[2];
	FILE *cmdsPipe;

	SBHostOS::ThreadCreated("<lldb.driver.command-thread>");

	if (!cmdsData || cmdsSize == 0) {
		return;
	}
	if ((cmdsPipe = Open_LLDB_Pipe(cmdsData, cmdsSize, cmdsFds)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	db.SetInputFileHandle(cmdsPipe, true);
	AG_Config *cfg = agConfig;
	SBCommandInterpreterRunOptions ro;
   	bool asyncSave = db.GetAsync();
	int nErrors;

	db.SetAsync(async);

	ro.SetStopOnContinue      (AG_GetBool(cfg, "lldb.stopOnCont"));
	ro.SetStopOnError         (AG_GetBool(cfg, "lldb.stopOnError"));
	ro.SetStopOnCrash         (AG_GetBool(cfg, "lldb.stopOnCrash"));
	ro.SetEchoCommands        (AG_GetBool(cfg, "lldb.echoCommands"));
	ro.SetEchoCommentCommands (AG_GetBool(cfg, "lldb.echoCommCmds"));
	ro.SetPrintResults        (AG_GetBool(cfg, "lldb.printResults"));

	lldb::SBCommandInterpreter interpreter = db.GetCommandInterpreter();
	lldb::SBCommand agarCmds = interpreter.AddMultiwordCommand("agar", NULL);
	agarCmds.AddCommand("debug", new AgarDebugCommand(),
	                    _("set agardb debug level"));
	agarCmds.AddCommand("driver", new AgarDriverCommand(),
	                    _("print agar driver information"));
	agarCmds.AddCommand("version", new AgarVersionCommand(),
	                    _("print libagar version"));

	db.RunCommandInterpreter(doHandleEvents, async, ro, nErrors,
	                         quitRequested, stoppedForCrash);

	db.SetAsync(asyncSave);

	/* Close any pipes that we still have ownership of */
	Close_LLDB_Pipe(cmdsFds);
}

static void
PrintVersion(void)
{
	AG_AgarVersion av;
	AG_ConsoleLine *cl;
	AG_Color c;
		
	cl = AG_ConsoleMsg(g_console, "agardb version %s", VERSION);
	AG_ColorRGB_8(&c, 0,170,170);
	AG_ConsoleMsgColor(cl, &c);

	AG_GetVersion(&av);
	cl = AG_ConsoleMsg(g_console, "agar version %d.%d.%d",
	    av.major, av.minor, av.patch);
	AG_ColorRGB_8(&c, 0,170,0);
	AG_ConsoleMsgColor(cl, &c);
}

/*
 * Debugger command entry
 */
static void *ExecCmdThread(void *);

static void
ExecCmd(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_Thread th;

	AG_ThreadCreate(&th, ExecCmdThread, tb);
}
static void *
ExecCmdThread(void *arg)
{
	AG_Textbox *tb = AGTEXTBOX(arg);
	char *s;
	SBStream cmds;
	
	AG_OBJECT_ISA(tb, "AG_Widget:AG_Textbox:*");

	s = AG_TextboxDupString(tb);
	if (strcmp(s, "version") == 0)
		PrintVersion();

	cmds.Printf("%s\n", s);
	free(s);

	Agardb::Run_LLDB(cmds, 0);
	AG_TextboxClearString(tb);
	return (NULL);
}

static void
ConsoleHelp(AG_Event *event)
{
	SBStream cmds;

	cmds.Printf("help\n");
	Agardb::Run_LLDB(cmds, 0);
}

static void
QuitGUI(AG_Event *event)
{
	AG_QuitGUI();
}

static std::string
EscapeString(std::string arg)
{
	std::string::size_type pos = 0;

	while ((pos = arg.find_first_of("\"\\", pos)) != std::string::npos) {
		arg.insert(pos, 1, '\\');
		pos += 2;
	}
	return '"' + arg + '"';
}

/*
 * Create a new debugger target.
 */
static void
CreateTarget(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_PTR(1);
	char execFile[AG_PATHNAME_MAX];
	char coreFile[AG_PATHNAME_MAX];
	char symFile[AG_PATHNAME_MAX];
	char args[AG_ARG_MAX];
	char arch[ADB_ARCH_NAME_MAX];
	lldb::SBDebugger &db = g_agardb->GetDebugger();

	AG_FileDlgCopyFilename(AGFILEDLG( AG_GetPointer(win,"fdExec") ), execFile, sizeof(execFile));
	AG_FileDlgCopyFilename(AGFILEDLG( AG_GetPointer(win,"fdCore") ), coreFile, sizeof(coreFile));
	AG_FileDlgCopyFilename(AGFILEDLG( AG_GetPointer(win,"fdSyms") ), symFile, sizeof(symFile));
	AG_TextboxCopyString  (AGTEXTBOX( AG_GetPointer(win,"tbArgs") ), args, sizeof(args));
#ifdef HAVE_LLDB_UTILITY
	AG_TextboxCopyString  (AGCOMBO  ( AG_GetPointer(win,"comArch") )->tbox, arch, sizeof(arch));
#else
	arch[0] = '\0';
#endif
	if (arch[0] == '\0')
		db.GetDefaultArchitecture(arch, sizeof(arch));

	SBStream cmds;
	cmds.Printf("target create");
	if (!AG_CheckboxGetState(AGCHECKBOX( AG_GetPointer(win,"cbDeps") ))) {
		cmds.Printf(" --no-dependents=true");
	}
	cmds.Printf(" --arch=%s %s", arch, execFile);
	if (coreFile[0] != '\0') { cmds.Printf(" --core %s", EscapeString(coreFile).c_str()); }
	if (symFile[0] != '\0') { cmds.Printf(" --symfile %s", EscapeString(symFile).c_str()); }
	cmds.Printf("\n");
	if (args[0] != '\0')
		cmds.Printf("settings set -- target.run-args %s\n", args);

	Agardb::Run_LLDB(cmds, 0);
	AG_PostEvent(win, "window-close", NULL);
}

static void
MoveFocus(AG_Event *event)
{
	AG_WidgetFocus(AG_WIDGET_PTR(1));
}

static void
CreateTargetDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fdExec, *fdCore, *fdSyms;
	AG_Checkbox *cbDeps;
	AG_Textbox *tbArgs;
	AG_Size i;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("New Target"));
	AG_SetStyle(win, "padding", "5");

	AG_SpacerNewHoriz(win);

	fdExec = AG_FileDlgNewCompactMRU(win, "adb.mru.exec", _("Executable: "),
	                                 AG_FILEDLG_HFILL | AG_FILEDLG_MASK_EXT);
	AG_SetPointer(win, "fdExec", fdExec);

	const struct {
		const char *descr;
		const char *extns;
	} exeFormats[] = {
		{ N_("Executable"),		"<-x>" },
		{ N_("Binary Executable"),	".bin,<=a.out>" },
		{ N_("DOS/Windows Executable"),	".exe,.com" },
		{ N_("OSX Application"),	".app,.osx" },
#ifdef __WIN32__
		{ N_("Control Panel Extension"),".cpl" },
		{ N_("Installer Package"),	".msi" },
		{ N_("Screensaver Executable"),	".scr" },
#endif
	};
	for (i = 0 ; i < sizeof(exeFormats) / sizeof(exeFormats[0]); i++) {
		AG_FileDlgAddType(fdExec,
		    exeFormats[i].descr,
		    exeFormats[i].extns, NULL, NULL);
	}

	/* Arguments to the program */
	tbArgs = AG_TextboxNewS(win, AG_TEXTBOX_HFILL, _("Arguments: "));
	AG_SetStyle(tbArgs, "font-style", "courier-prime");
	AG_SetPointer(win, "tbArgs", tbArgs);

	AG_SeparatorNewHoriz(win);

	/* Core dump file */
	fdCore = AG_FileDlgNewCompactMRU(win, "adb.mru.exec", _("Core File: "),
	                                 AG_FILEDLG_HFILL | AG_FILEDLG_MASK_EXT);
	AG_FileDlgAddType(fdCore, _("Core file"), ".core,<=core>", NULL, NULL);
	AG_SetPointer(win, "fdCore", fdCore);

	/* Stand-alone symbols file */
	fdSyms = AG_FileDlgNewCompactMRU(win, "adb.mru.exec", _("Symbols File: "),
	                                 AG_FILEDLG_HFILL | AG_FILEDLG_MASK_EXT);
	AG_SetPointer(win, "fdSyms", fdSyms);
	AG_FileDlgAddType(fdSyms, _("ELF symbols file"), ".debug", NULL, NULL);
	AG_FileDlgAddType(fdSyms, _("MacOS symbols file"), ".dSYM", NULL, NULL);
	AG_FileDlgAddType(fdSyms, _("Windows symbols file"), ".pdb", NULL, NULL);

	AG_SeparatorNewHoriz(win);

	/* Target architecture */
#ifdef HAVE_LLDB_UTILITY
	AG_CPUInfo cpu;
	AG_Combo *comArch;
	StringList arches;

	comArch = AG_ComboNew(win, AG_COMBO_HFILL, _("Architecture: "));
	AG_ComboSizeHint(comArch, "<unknown-mach-64>", 15);
	AG_SetPointer(win, "comArch", comArch);

	ArchSpec::ListSupportedArchNames(arches);
	for (i = 0; i < arches.GetSize(); i++) {
		AG_TlistAddS(comArch->list, agIconGear.s,
		    arches.GetStringAtIndex(i));
	}
	AG_GetCPUInfo(&cpu);
	if (strcmp(cpu.arch, "amd64") == 0) {
		AG_ComboSelectText(comArch, "x86_64");
	} else {
		AG_ComboSelectText(comArch, cpu.arch);
	}
#endif /* HAVE_LLDB_UTILITY */

	AG_SeparatorNewHoriz(win);

	/* Dependencies */
	cbDeps = AG_CheckboxNew(win, 0, _("Load dependencies"));
	AG_CheckboxToggle(cbDeps);
	AG_SetPointer(win, "cbDeps", cbDeps);

	AG_Box *hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	AG_Button *btnOK;

	btnOK = AG_ButtonNewFn(hBox, 0, _("OK"), CreateTarget, "%p", win);
	AG_ButtonNewFn(hBox, 0, _("Cancel"), AGWINCLOSE(win));

	AG_WidgetFocus(fdExec->textbox);
	AG_SetEvent(fdExec->textbox, "textbox-return", MoveFocus, "%p", tbArgs);
	AG_SetEvent(tbArgs,          "textbox-return", MoveFocus, "%p", fdCore->textbox);
	AG_SetEvent(fdCore->textbox, "textbox-return", MoveFocus, "%p", fdSyms->textbox);
	AG_SetEvent(fdSyms->textbox, "textbox-return", MoveFocus, "%p", btnOK);

	AG_WindowShow(win);
}

void
Agardb::Take_Screenshot(AG_Event *event)
{
	AG_ViewCapture();
}

#if defined(AG_DEBUG) && defined(AG_TIMERS)
void
Agardb::Run_GUI_Debugger_GK(void)
{
	AG_Window *win = (AG_Window *) AG_GuiDebugger(agWindowFocused);
	if (win != NULL)
		AG_WindowShow(win);
}
void
Agardb::Run_GUI_Debugger(AG_Event *event)
{
	Agardb::Run_GUI_Debugger_GK();
}
#endif /* DEBUG and TIMERS */

/* Debugger preferences */
void
Agardb::GUI::Preferences(AG_Event *event)
{
	AG_Window *win = AG_WindowNew(0);
	AG_Notebook *nb;
	AG_NotebookTab *nt;

	AG_WindowSetCaption(win, _("Debugger Preferences"));
	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);

	nt = AG_NotebookAdd(nb, _("Interpreter Options"), AG_BOX_VERT);
	{
		for (int i = 0; i < sizeof(g_interp_opts) / sizeof(g_interp_opts[0]); i++) {
			AG_Checkbox *cb;

			cb = AG_CheckboxNewS(nt, 0, _(g_interp_opts[i].descr));
			AG_BindVariable(cb, "state",
			                agConfig, g_interp_opts[i].cfgKey);
		}
	}

	AG_WindowShow(win);
}

/* "Targets / Select Target" */
void
Agardb::GUI::SelectTarget(AG_Event *event)
{
	int i = AG_INT(1);

	AG_TextTmsg(AG_MSG_INFO, 1250, _("Selected target # %d"), i);
}

/* Generate the "Targets" menu */
void
Agardb::GUI::MenuTargets(AG_Event *event)
{
	AG_MenuItem *mi = AG_MENU_ITEM_PTR(1);
	lldb::SBDebugger &db = g_agardb->GetDebugger();
	const Uint32 numTargets = db.GetNumTargets();
	Uint32 i;

	if (numTargets == 0) {
		(AG_MenuNode(mi,_("(no targets)"),NULL))->state = 0;
		return;
	}
	for (i = 0; i < numTargets; i++) {
		lldb::SBTarget dbTarget = db.GetTargetAtIndex(i);

		if (!dbTarget.IsValid()) {
			AG_MenuNode(mi,_("(invalid target)"),NULL);
			continue;
		}
		lldb::SBFileSpec exe = dbTarget.GetExecutable();
		if (exe.IsValid() && exe.Exists()) {
			AG_MenuAction(mi,
			    AG_Printf("%d. %s (%s)", i, exe.GetFilename(),
			        dbTarget.GetTriple()),
			    agIconDocImport.s,
			    Agardb::GUI::SelectTarget, "%i", i);
		} else {
			AG_MenuAction(mi,
			    AG_Printf("Target #%d (%s)", i, dbTarget.GetTriple()),
			    agIconDoc.s,
			    Agardb::GUI::SelectTarget, "%i", i);
		}
	}
}

/*
 * Create Agar GUI.
 */
Agardb::GUI::GUI()
{
	AG_AgarVersion av;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_MenuItem *m;

	g_agardb_gui = this;

	if (g_font_spec != NULL) {
		AG_TextParseFontSpec(g_font_spec);
	}
	if (AG_InitGraphics(g_driver_spec) == -1) {
		Verbose("Agar-GUI failed: %s\n", AG_GetError());
		FatalError();
	}
	AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, DATADIR);
	AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, ".");

	AG_BindStdGlobalKeys();
#ifdef __APPLE__
# if defined(AG_DEBUG) && defined(AG_TIMERS)
	AG_BindGlobalKey(AG_KEY_D, AG_KEYMOD_META, Agardb::Run_GUI_Debugger_GK);
# endif
	AG_BindGlobalKey(AG_KEY_C, AG_KEYMOD_META, AG_ViewCapture);
#else
# if defined(AG_DEBUG) && defined(AG_TIMERS)
	AG_BindGlobalKey(AG_KEY_F7,  AG_KEYMOD_ANY,                  Run_GUI_Debugger_GK);
	AG_BindGlobalKey(AG_KEY_F12, AG_KEYMOD_ANY,                  Run_GUI_Debugger_GK);
/*	AG_BindGlobalKey(AG_KEY_K,   AG_KEYMOD_CTRL|AG_KEYMOD_SHIFT, Run_GUI_Debugger_GK); */
# endif
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
#endif

	if ((g_main_window = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		AG_Verbose("GUI failed: %s\n", AG_GetError());
		return;
	}
	AG_WindowSetCaption(g_main_window, "agardb");
	AG_WindowSetPosition(g_main_window, AG_WINDOW_BC, 0);

	g_menu = AG_MenuNew(g_main_window, AG_MENU_HFILL);
	m = AG_MenuNode(g_menu->root, _("File"), NULL);
	{
		AG_MenuActionKb(m, _("New Target..."), agIconLoad.s,
		    AG_KEY_N, AG_KEYMOD_CTRL,
		    CreateTargetDlg, NULL);
		
		AG_MenuDynamicItem(m, _("Targets"), NULL,
		    Agardb::GUI::MenuTargets, NULL);
	
		AG_MenuSeparator(m);
#if 0	
		AG_MenuActionKb(m, _("Take Screenshot"), agIconDoc.s,
		    AG_KEY_F8, 0,
		    Agardb::Take_Screenshot, NULL);
#endif
		AG_MenuActionKb(m, _("GUI Debugger..."), agIconMagnifier.s,
		    AG_KEY_F7, 0,
		    Agardb::Run_GUI_Debugger, NULL);
		
		AG_MenuSeparator(m);

		AG_MenuActionKb(m, _("Quit"), agIconClose.s,
		    AG_KEY_Q, AG_KEYMOD_CTRL,
		    QuitGUI, NULL);
	}

	m = AG_MenuNode(g_menu->root, _("Edit"), NULL);
	{
		AG_MenuAction(m, _("Preferences..."), agIconGear.s,
		    Agardb::GUI::Preferences, NULL);
	}

	nb = AG_NotebookNew(g_main_window, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Inspector"), AG_BOX_HORIZ);
	{
		AG_LabelNewS(nt, 0,
		    "TODO\n"
		    "Display the hierarchy of Windows and Widgets of an\n"
		    "instrumented Agar. Provide inspection of VFS and Objects,\n"
		    "Variables, Events and Timers. Also Surfaces, Cursors and\n"
		    "class-specific items.\n"
		    "\n"
		    "Stylesheet editor with Pick an Element (Ctrl+Shift+C)\n"
		    "feature, filters and import/export ability. Fonts, Icons,\n"
		    "Surfaces/Animations (bitmap editor). Ability to capture\n"
		    "and transmit widget renderings to the debugger.");
	}
	nt = AG_NotebookAdd(nb, _("Console"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0,
		    "TODO\n"
		    "Remotely access the AG_Debug() and AG_Verbose() output"
		    "generated by the debugged program.");
	}
	nt = AG_NotebookAdd(nb, _("Debugger"), AG_BOX_VERT);
	AG_NotebookSelect(nb, nt);
	{
		AG_Box *box;
		AG_Textbox *tb;

		g_console = AG_ConsoleNew(nt, AG_CONSOLE_EXPAND);
		PrintVersion();

		box = AG_BoxNewHoriz(nt, AG_BOX_HFILL);
		AG_SetStyle(box, "font-weight", "bold");
		AG_SetStyle(box, "font-size", "120%");
		tb = g_textbox_prompt = AG_TextboxNew(box,
		    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
		    "(lldb)");
		AG_SetEvent(tb, "textbox-return", ExecCmd, NULL);
	
		AG_ActionFn(g_console, "Help", ConsoleHelp, NULL);
		AG_ActionOnKey(g_console, AG_KEY_F1, AG_KEYMOD_ANY, "Help");
		AG_ActionOnKey(g_console, AG_KEY_HELP, AG_KEYMOD_ANY, "Help");

		AG_WidgetFocus(tb);

		AG_SeparatorNewHoriz(nt);

		AG_LabelNewS(nt, 0,
		    "TODO\n"
		    "Show a Tlist of source files, object files and libraries.\n"
		    "\n"
		    "Command-line interface to LLDB. Process control (run, stop).\n"
		    "Attach to processes by PID or by name. Process monitor.\n"
		    "Backtraces. Watch expressions. Breakpoints...");
	}
	nt = AG_NotebookAdd(nb, _("Style Editor"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0, "TODO: A simple editor for Agar stylings");
	}
	nt = AG_NotebookAdd(nb, _("Memory"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0, "TODO: A memory usage monitor");
	}
#if 0
	/*
	 * Additional tabs by library or program-specific add-ons here.
	 */
	TAILQ_FOREACH(addon, &agardb_addons, addons) {
		nt = AG_NotebookAdd(nb, addon->title, AG_BOX_VERT);
		addon->edit(nt);
	}
#endif
	nt = AG_NotebookAdd(nb, _("About agardb"), AG_BOX_VERT);
	{
		AG_SetStyle(nt, "font-size", "150%");
		AG_GetVersion(&av);
		AG_LabelNew(nt, 0,
		    "agardb version %s\n"
		    "agar version %d.%d.%d (\"%s\")\n"
		    "%s", VERSION,
		    av.major, av.minor, av.patch, av.release,
		    SBDebugger::GetVersionString());
	}

	AG_WindowSetGeometryAligned(g_main_window, AG_WINDOW_BC, 1000, 600);
	AG_WindowShow(g_main_window);
}

Agardb::GUI::~GUI()
{
	AG_ConfigSave();
	AG_DestroyGraphics();
	g_agardb_gui = NULL;
}

/* Handle terminal window resize */
void
Agardb::SetTerminalWidth(unsigned short col)
{
	GetDebugger().SetTerminalWidth(col);
}

static void
PrintUsage(void)
{
	printf("%s [-hvxXD] [-a architecture] [-c core-file] "
	       "[-f exec-file] [-n process-name] [-p process-pid] "
	       "[-d agar-driver] "
	       "[-t font] [-f file]\n", agProgName);
}

static void
Handle_SigINT(int signo)
{
	static std::atomic_flag g_interrupt_sent = ATOMIC_FLAG_INIT;

	if (g_agardb != NULL) {
		if (!g_interrupt_sent.test_and_set()) {
			lldb::SBDebugger &db = g_agardb->GetDebugger();

			db.DispatchInputInterrupt();
			g_interrupt_sent.clear();
			return;
		}
	}
	_exit(signo);
}

static void
Handle_SigWINCH(int signo)
{
	struct winsize window_size;

	if (isatty(STDIN_FILENO) &&
	    ::ioctl(STDIN_FILENO, TIOCGWINSZ, &window_size) == 0) {
		if ((window_size.ws_col > 0) && g_agardb != NULL) {
			g_agardb->SetTerminalWidth(window_size.ws_col);
		}
	}
}

static void
Handle_SigTSTP(int signo)
{
	if (g_agardb != NULL)
		g_agardb->GetDebugger().SaveInputTerminalState();

	signal(signo, SIG_DFL);
	kill(getpid(), signo);
	signal(signo, Handle_SigTSTP);
}

static void
Handle_SigCONT(int signo)
{
	if (g_agardb != NULL)
		g_agardb->GetDebugger().RestoreInputTerminalState();

	signal(signo, SIG_DFL);
	kill(getpid(), signo);
	signal(signo, Handle_SigCONT);
}

#if 0
static void
ResetStdinTermios(void)
{
	if (g_old_stdin_termios_is_valid) {
		g_old_stdin_termios_is_valid = false;
		::tcsetattr(STDIN_FILENO, TCSANOW, &g_old_stdin_termios);
	}
}
#endif

int
main(int argc, char *const argv[])
{
	char savePath[AG_PATHNAME_MAX];
	char *optArg = NULL;
	int optInd, i, c;
	lldb::pid_t processPID = LLDB_INVALID_PROCESS_ID;

#ifdef ENABLE_NLS
	bindtextdomain("agardb", LOCALEDIR);
	bind_textdomain_codeset("agardb", "UTF-8");
	textdomain("agardb");
#endif
	if (AG_InitCore("agardb", AG_VERBOSE | AG_CREATE_DATADIR) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
#ifdef AG_DEBUG
	agDebugLvl = 1;
#endif
	g_process_name.erase();
	while ((c = AG_Getopt(argc, argv, "a:c:f:n:p:d:t:xDXvh?", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'a':
			g_default_arch = optArg;
			break;
		case 'c':
			g_core_file = optArg;
			break;
		case 'f':
			g_exec_file = optArg;
			break;
		case 'n':
			g_process_name = optArg;
			break;
		case 'w':
			g_process_wait_for = 1;
			break;
		case 'p':
			char *ep;
			processPID = strtol(optArg, &ep, 0);
			if (ep == optArg || *ep != '\0') {
				fprintf(stderr, "Bad Process ID: %s\n", optArg);
				AG_Destroy();
				return (1);
			}
			break;
		case 'd':
			g_driver_spec = optArg;
			break;
		case 't':
			g_font_spec = optArg;
			break;
		case 'x':
			/* is the default */
			break;
		case 'D':
#ifdef AG_DEBUG
			agDebugLvl = 0;
#endif
			break;
		case 'X':
			g_set_use_color = 0;
			break;
		case 'v':
			printf("agardb %s\n", VERSION);
			AG_Destroy();
			return (0);
		case 'h':
		case '?':
		default:
			PrintUsage();
			AG_Destroy();
			return (1);
		}
	}
	if (g_process_name.empty() && processPID == LLDB_INVALID_PROCESS_ID) {
		if (optInd < argc && g_exec_file == NULL) { /* Equivalent to -f */
			g_exec_file = argv[optInd];
			optInd++;
		}
		for (i = optInd; i < argc; i++) {    /* Args for the program */
			if (argv[i] != NULL)
				g_exec_args.push_back(argv[i]);
		}
	}
	
	/*
	 * LLDB Initialization
	 */
#ifdef HAVE_LLDB_UTILITY
	llvm::StringRef ToolName = argv[0];
	llvm::sys::PrintStackTraceOnErrorSignal(ToolName);
#endif
	llvm::PrettyStackTraceProgram X(argc, argv);
	SBDebugger::Initialize();
  	SBHostOS::ThreadCreated("<lldb.driver.main-thread>");
	signal(SIGINT, Handle_SigINT);
#if !defined(_MSC_VER)
	signal(SIGPIPE, SIG_IGN);
	signal(SIGWINCH, Handle_SigWINCH);
	signal(SIGTSTP, Handle_SigTSTP);
	signal(SIGCONT, Handle_SigCONT);
#endif
	{
		Agardb agardb;			/* Enter debugger */
		lldb::SBDebugger db;
		SBStream cmds;

		db = agardb.GetDebugger();
		if (db.IsValid()) {
			AG_Verbose("LLDB: %s OK\n", SBDebugger::GetVersionString());
		} else {
			AG_Verbose("LLDB: Initialization failed!\n");
			AG_Destroy();
			return (1);
		}

		/* Do not parse .lldbinit files automatically */
		db.SkipLLDBInitFiles(true);
		db.SkipAppInitFiles(true);

		/*
		 * Check command-line arguments
		 */
		if (g_default_arch) {
			if (db.SetDefaultArchitecture(g_default_arch) == 0) {
				Verbose("No such architecture \"%s\"\n", g_default_arch);
				FatalError();
			}
		}
		if (g_core_file != NULL) {
			SBFileSpec file(g_core_file);
			if (!file.Exists()) {
				Verbose("No such core (-c) file: %s\n", g_core_file);
				FatalError();
			}
			Verbose("Core file %s exists\n", g_core_file);
		}
		if (g_exec_file != NULL) {
			SBFileSpec file(g_exec_file);
			if (file.Exists()) {
				Verbose("Exec file %s exists\n", g_exec_file);
				g_exec_args.insert(g_exec_args.begin(),
				                   g_exec_file);
			} else if (file.ResolveExecutableLocation()) {
				char path[AG_PATHNAME_MAX];
				file.GetPath(path, sizeof(path));
				Verbose("Executable %s exists (path: %s)\n",
				    g_exec_file, path);
				g_exec_args.insert(g_exec_args.begin(),
				                   path);
			} else {
				Verbose("Exec: No such file: %s\n", g_exec_file);
				FatalError();
			}
		}
#if 0
		/* Terminal Initialization */
		if (::tcgetattr(STDIN_FILENO, &g_old_stdin_termios) == 0) {
			g_old_stdin_termios_is_valid = true;
			atexit(ResetStdinTermios);
		}
#ifndef _MSC_VER
		::setbuf(stdin, NULL);
#endif
		::setbuf(stdout, NULL);

		/* I/O Initialization */
		db.SetErrorFileHandle(stderr, false);
		db.SetOutputFileHandle(stdout, false);
		db.SetInputFileHandle(stdin, false);     /* Not yet */
		/* db.SetUseExternalEditor(useExternalEditor); */
 
 		/* Terminal Sizing */
		struct winsize window_size;
		if (isatty(STDIN_FILENO) &&
		    ::ioctl(STDIN_FILENO, TIOCGWINSZ, &window_size) == 0) {
			if (window_size.ws_col > 0)
				db.SetTerminalWidth(window_size.ws_col);
		}
#endif
		const int nArgs = g_exec_args.size();

		if (nArgs > 0) {
			char archName[ADB_ARCH_NAME_MAX];

			if (db.GetDefaultArchitecture(archName, sizeof(archName))) {
				cmds.Printf("target create --arch=%s %s",
				    archName,
				    EscapeString(g_exec_args[0]).c_str());
			} else {
				cmds.Printf("target create %s",
				    EscapeString(g_exec_args[0]).c_str());
			}
			if (g_core_file != NULL) {
				cmds.Printf(" --core %s", EscapeString(g_core_file).c_str());
			}
    			cmds.Printf("\n");

			if (nArgs > 1) {
      				cmds.Printf("settings set -- target.run-args ");
				for (i = 1; i < nArgs; ++i) {
					cmds.Printf(" %s",
					    EscapeString(g_exec_args[i]).c_str());
				}
				cmds.Printf("\n");
			}
		} else if (g_core_file != NULL) {		/* and nArgs=0 */
			cmds.Printf("target create --core %s\n",
                           EscapeString(g_core_file).c_str());
		} else if (!g_process_name.empty()) {
			cmds.Printf("process attach --name %s",
                           EscapeString(g_process_name).c_str());
			if (g_process_wait_for) {
				cmds.Printf(" --waitfor");
			}
			cmds.Printf("\n");
		} else if (processPID != LLDB_INVALID_PROCESS_ID) {
			cmds.Printf("process attach --pid %" PRIu64 "\n",
			    processPID);
		}

		/* Load settings from ~/.agardb. */
		(void)AG_ConfigLoad();
	
		/*
		 * Inherit command interpreter option defaults from
		 * SBCommandInterpreterRunOptions().
		 */
		AG_Config *cfg = agConfig;
		const SBCommandInterpreterRunOptions ro;

		if (!AG_Defined(cfg, "lldb.stopOnCont"))
			AG_SetBool(cfg,"lldb.stopOnCont", ro.GetStopOnContinue());
		if (!AG_Defined(cfg, "lldb.stopOnError"))
			AG_SetBool(cfg,"lldb.stopOnError", ro.GetStopOnError());
		if (!AG_Defined(cfg, "lldb.stopOnCrash"))
			AG_SetBool(cfg,"lldb.stopOnCrash", ro.GetStopOnCrash());
		if (!AG_Defined(cfg, "lldb.echoCommands"))
			AG_SetBool(cfg,"lldb.echoCommands", ro.GetEchoCommands());
		if (!AG_Defined(cfg, "lldb.echoCommCmds"))
			AG_SetBool(cfg,"lldb.echoCommCmds", ro.GetEchoCommentCommands());
		if (!AG_Defined(cfg, "lldb.printResults"))
			AG_SetBool(cfg,"lldb.printResults", ro.GetPrintResults());

		/* Commit creation of target based on command-line args. */
		Agardb::Run_LLDB(cmds, 0);

		/*
		 * Create Agar GUI.
		 */
		Agardb::GUI GUI;

		/*
		 * Redirect lldb output and error to ~/.agardb/agardb.out.<pid>
		 * and ~/.agardb/agardb.err.<pid> for history-keeping.
		 */
		char pathOut[AG_PATHNAME_MAX];
		char pathErr[AG_PATHNAME_MAX];
		std::FILE *logOut, *logErr;

		AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, savePath, sizeof(savePath));
		Strlcat(savePath, AG_PATHSEP, sizeof(savePath));
		Snprintf(pathOut, sizeof(pathOut), "%sagardb.out.%d", savePath, getpid());
		Snprintf(pathErr, sizeof(pathErr), "%sagardb.err.%d", savePath, getpid());

		if ((logOut = std::fopen(pathOut, "w")) == NULL) {
			Verbose("%s: %s", pathOut, strerror(errno));
			return (1);
		}
		::setbuf(logOut, NULL);

		if ((logErr = std::fopen(pathErr, "w")) == NULL) {
			Verbose("%s: %s", pathErr, strerror(errno));
			return (1);
		}
		::setbuf(logErr, NULL);

		db.SetOutputFileHandle(logOut, true);
		db.SetErrorFileHandle(logErr, true);

		/*
		 * Arrange for AG_Console(3) to follow the output and error
		 * log files we've just created.
		 */
		AG_ConsoleOpenFile(g_console, "lldberr", pathErr, AG_NEWLINE_NATIVE, 0);
		AG_ConsoleOpenFile(g_console, "lldb", pathOut, AG_NEWLINE_NATIVE, 0);

		AG_EventLoop();
	}

	/*
	 * Save the contents of the output and error buffers to a .log file.
	 */
	char pathPID[AG_PATHNAME_MAX];
	char pathLog[AG_PATHNAME_MAX];

	AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, savePath, sizeof(savePath));
	Strlcat(savePath, AG_PATHSEP, sizeof(savePath));
	Snprintf(pathPID, sizeof(pathPID), "%sagardb.out.%d", savePath, getpid());
	Snprintf(pathLog, sizeof(pathLog), "%sagardb.out.log", savePath);
	rename(pathPID, pathLog);
	Snprintf(pathPID, sizeof(pathPID), "%sagardb.err.%d", savePath, getpid());
	Snprintf(pathLog, sizeof(pathLog), "%sagardb.err.log", savePath);
	rename(pathPID, pathLog);

	SBDebugger::Terminate();
	AG_Destroy();
	return (0);
}
