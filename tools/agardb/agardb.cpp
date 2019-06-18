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
using namespace lldb_private;

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

static AG_Console  *g_console = NULL;		/* The GUI console */
static AG_Menu     *g_menu = NULL;		/* The main menu */
static AG_Textbox  *g_textbox_prompt = NULL;	/* "agardb>" prompt */

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
PrepCmd(const char *cmdsData, size_t cmdsSize, int fds[2])
{
	enum PIPES { READ, WRITE };   /* Constants 0 and 1 for READ and WRITE */
	::FILE *cmdsFile = NULL;

	fds[0] = -1;
	fds[1] = -1;
	int err = 0;
#ifdef _WIN32
	err = _pipe(fds, cmdsSize, O_BINARY);
#else
	err = pipe(fds);
#endif
	if (err == 0) {
		ssize_t nrwr;

		nrwr = write(fds[WRITE], cmdsData, cmdsSize);
		if (nrwr < 0) {
			Verbose("write(%i, %p, %" PRIu64 ") failed (errno = %i) "
			    "trying to open lldb commands pipe\n",
			    fds[WRITE], static_cast<const void *>(cmdsData),
			    static_cast<uint64_t>(cmdsSize), errno);
		} else if (static_cast<size_t>(nrwr) == cmdsSize) {
			/*
			 * Close the write end of the pipe so when we give the read end to
			 * the debugger/command interpreter it will exit when it consumes all
			 * of the data.
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
			cmdsFile = fdopen(fds[READ], "r");
			if (cmdsFile) {
				/*
				 * cmdsFile now owns the read descriptor.
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
	} else {
		Verbose("can't create pipe file descriptors for lldb commands\n");
	}

	return cmdsFile;
}

static void
PrepCmdCleanup(int fds[2])
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
 * Forward a set of commands to LLDB.
 */
void
Agardb::Forward_Commands(SBStream &cmds)
{
	lldb::SBDebugger &db = g_agardb->GetDebugger();
	const char *cmdsData = cmds.GetData();
	const size_t cmdsSize = cmds.GetSize();
	bool quit_requested = false;
	bool stopped_for_crash = false;
  	bool handle_events = true;
	bool spawn_thread = false;

	if (cmdsData && cmdsSize > 0) {
		int cmdsFds[2];
		bool success = true;
		FILE *cmdsFile;

		cmdsFile = PrepCmd(cmdsData, cmdsSize, cmdsFds);
		if (cmdsFile) {
			db.SetInputFileHandle(cmdsFile, true);
   			bool asyncSave = db.GetAsync();
			int nErrors;

			db.SetAsync(false);

			SBCommandInterpreterRunOptions options;
			options.SetStopOnError(true);
/*			if (batchMode) { options.SetStopOnCrash(true); } */

			db.RunCommandInterpreter(handle_events, spawn_thread,
			                         options, nErrors,
						 quit_requested,
						 stopped_for_crash);
			db.SetAsync(asyncSave);
		} else {
			success = false;
		}

		/* Close any pipes that we still have ownership of */
		PrepCmdCleanup(cmdsFds);

		if (!success) {
			AG_SetError("lldb command pipe error");
			FatalError();
		}
	}
}

/*
 * Debugger command entry
 */
static void
ExecCmd(AG_Event *event)
{
	AG_Textbox *tb = (AG_Textbox *)AG_SELF();
	lldb::SBDebugger &db = g_agardb->GetDebugger();
	char *s;
	SBStream cmds;
	
	if (!db.IsValid()) {
		AG_TextError("db !IsValid");
		return;
	}

	s = AG_TextboxDupString(tb);
	cmds.Printf("%s\n", s);
	free(s);

	Agardb::Forward_Commands(cmds);

	AG_TextboxClearString(tb);
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

/* Call lldb "target create"  */
static void
CreateTarget(AG_Event *event)
{
	AG_FileDlg *fdExec = (AG_FileDlg *) AG_PTR(1);
	AG_FileDlg *fdCore = (AG_FileDlg *) AG_PTR(2);
	AG_Combo *comArch  = (AG_Combo *) AG_PTR(3);
	AG_Checkbox *cbDeps = (AG_Checkbox *) AG_PTR(4);
	lldb::SBDebugger &db = g_agardb->GetDebugger();
	
	if (!db.IsValid()) {
		AG_TextError("db !IsValid");
		return;
	}

	char execFile[AG_PATHNAME_MAX];
	char coreFile[AG_PATHNAME_MAX];
	char arch[64];
	bool loadDeps;

	AG_FileDlgCopyFilename(fdExec, execFile, sizeof(execFile));
	AG_FileDlgCopyFilename(fdCore, coreFile, sizeof(coreFile));
	AG_TextboxCopyString(comArch->tbox, arch, sizeof(arch));
	if (arch[0] == '\0') {
		db.GetDefaultArchitecture(arch, sizeof(arch));
	}
	loadDeps = AG_CheckboxGetState(cbDeps);

	SBStream cmds;

	cmds.Printf("target create --arch=%s %s", arch, execFile);
//	    EscapeString(g_exec_args[0]).c_str());

	if (coreFile[0] != '\0') {
		cmds.Printf(" --core %s", EscapeString(coreFile).c_str());
	}
	cmds.Printf("\n");
#if 0
	if (nArgs > 1) {
		cmds.Printf("settings set -- target.run-args ");
		for (i = 1; i < nArgs; ++i) {
			cmds.Printf(" %s",
			    EscapeString(g_exec_args[i]).c_str();
		}
	}
#endif

//	AG_TextInfo(NULL, "Create target fdExec=%s", AGOBJECT(fdExec)->name);
	Agardb::Forward_Commands(cmds);
	AG_PostEvent(NULL, AG_ParentWindow(fdExec), "window-close", NULL);
}

static void
CreateTargetDlg(AG_Event *event)
{
	AG_CPUInfo cpu;
//	lldb::SBDebugger &db = g_agardb->GetDebugger();
	AG_Window *win;
	size_t i;
	AG_FileDlg *fdExec, *fdCore;
	AG_Combo *comArch;
	AG_Checkbox *cbDeps;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Create Debugger Target"));

	/* Executable and core files */
	fdExec = AG_FileDlgNewCompactMRU(win, "adb.mru.exec", _("Executable: "),
	    AG_FILEDLG_HFILL | AG_FILEDLG_MASK_EXT);
	{
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
		int i;

		for (i = 0 ; i < sizeof(exeFormats) / sizeof(exeFormats[0]); i++) {
			AG_FileDlgAddType(fdExec,
			    exeFormats[i].descr,
			    exeFormats[i].extns,
			    NULL, NULL);
		}
	}

	fdCore = AG_FileDlgNewCompactMRU(win, "adb.mru.exec", _("Core file: "),
	    AG_FILEDLG_HFILL | AG_FILEDLG_MASK_EXT);
	{
		AG_FileDlgAddType(fdCore, _("Core file"), ".core,<=core>",
		    NULL, NULL);
	}


	/* Target architecture */
	StringList arches;
	comArch = AG_ComboNew(win, AG_COMBO_HFILL, _("Architecture: "));
	AG_ComboSizeHint(comArch, "<unknown-mach-64>", 15);

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

	/* Dependencies */
	cbDeps = AG_CheckboxNew(win, 0, _("Load dependents"));
	AG_CheckboxToggle(cbDeps);

	AG_SeparatorNewHoriz(win);

	AG_Box *hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(hBox, 0, _("OK"), CreateTarget, "%p,%p,%p,%p,%p",
		    fdExec, fdCore, comArch, cbDeps);

		AG_ButtonNewFn(hBox, 0, _("Cancel"), AGWINCLOSE(win));
	}

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 400, -1);
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

/*
 * Our Agar-based GUI.
 */
Agardb::GUI::GUI()
{
	AG_AgarVersion av;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_MenuItem *m;

	g_agardb_gui = this;

	AG_Verbose("Creating Agar GUI\n");
	if (g_font_spec != NULL) {
		AG_TextParseFontSpec(g_font_spec);
	}
	if (AG_InitGraphics(g_driver_spec) == -1) {
		Verbose("Agar-GUI failed: %s\n", AG_GetError());
		FatalError();
	}
	AG_SetStringF(agConfig, "load-path", ".:%s", DATADIR);

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

	if ((winMain = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		AG_Verbose("GUI failed: %s\n", AG_GetError());
		return;
	}
	AG_WindowSetCaption(winMain, "agardb");
	AG_WindowSetPosition(winMain, AG_WINDOW_BC, 0);

	g_menu = AG_MenuNew(winMain, AG_MENU_HFILL);
	m = AG_MenuNode(g_menu->root, _("File"), NULL);
	{
		AG_MenuActionKb(m, _("New Target..."), agIconLoad.s,
		    AG_KEY_N, AG_KEYMOD_CTRL,
		    CreateTargetDlg, NULL);
	
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

	nb = AG_NotebookNew(winMain, AG_NOTEBOOK_EXPAND);
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
		AG_SetStyle(g_console, "font-family", "Terminal");
		AG_ConsoleMsgS(g_console, "Welcome to agardb");
		AG_SetStyle(g_console, "font-family", "Terminal");

		box = AG_BoxNewHoriz(nt, AG_BOX_HFILL);
		AG_SetStyle(box, "font-family", "Courier");
		AG_SetStyle(box, "font-weight", "bold");
		AG_SetStyle(box, "font-size", "120%");
		tb = g_textbox_prompt = AG_TextboxNew(box,
		    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
		    "agardb>");
		AG_SetEvent(tb, "textbox-return", ExecCmd, NULL);

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

	AG_WindowSetGeometryAligned(winMain, AG_WINDOW_BC, 1000, 600);
	AG_WindowShow(winMain);
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
	agDebugLvl = 0;
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
			agDebugLvl = 1;
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
	llvm::StringRef ToolName = argv[0];
	llvm::sys::PrintStackTraceOnErrorSignal(ToolName);
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
				g_exec_args.push_back(g_exec_file);
			} else if (file.ResolveExecutableLocation()) {
				char path[AG_PATHNAME_MAX];
				file.GetPath(path, sizeof(path));
				Verbose("Exec file %s (=> %s) exists\n", g_exec_file, path);
				g_exec_args.push_back(path);
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
			char archName[64];

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

		/* Commit creation of target based on command-line args. */
		Agardb::Forward_Commands(cmds);

		/* Create an Agar GUI */
		Agardb::GUI GUI;
		char pathLogOut[AG_PATHNAME_MAX];
		char pathLogErr[AG_PATHNAME_MAX];
		std::FILE *logOut, *logErr;

		/*
		 * Redirect lldb output and error to ~/.agardb/agardb.out
		 * and ~/.agardb/agardb.err for history-keeping.
		 */
		AG_GetString(agConfig, "save-path", pathLogOut, sizeof(pathLogOut));
		AG_Strlcat(pathLogOut, AG_PATHSEP, sizeof(pathLogOut));
		memcpy(pathLogErr, pathLogOut, strlen(pathLogOut)+1);
		AG_Strlcat(pathLogOut, "agardb.out", sizeof(pathLogOut));
		AG_Strlcat(pathLogErr, "agardb.err", sizeof(pathLogErr));

		if ((logOut = std::fopen(pathLogOut, "w")) == NULL) {
			Verbose("%s: %s", pathLogOut, strerror(errno));
			return (1);
		}
		::setbuf(logOut, NULL);

		if ((logErr = std::fopen(pathLogErr, "w")) == NULL) {
			Verbose("%s: %s", pathLogErr, strerror(errno));
			return (1);
		}
		::setbuf(logErr, NULL);

		db.SetOutputFileHandle(logOut, true);
		db.SetErrorFileHandle(logErr, true);

		/*
		 * Arrange for the AG_Console(3) to automatically follow the
		 * output and error log files we've just created.
		 */
		AG_ConsoleOpenFile(g_console, "lldberr", pathLogErr,
		    AG_NEWLINE_NATIVE, 0);
		AG_ConsoleOpenFile(g_console, "lldb", pathLogOut,
		    AG_NEWLINE_NATIVE, 0);

		AG_EventLoop();
	}

	SBDebugger::Terminate();
	AG_Destroy();
	return (0);
}
