/*	Public domain	*/

#ifndef _AGARDB_H_
#define _AGARDB_H_

#include <config/have_lldb.h>
#include <config/have_lldb_utility.h>

#include <bitset>
#include <set>
#include <string>
#include <vector>
#include <functional>

#ifdef __APPLE__
# include <LLDB/LLDB.h>
#else
# include "lldb/API/SBBreakpoint.h"
# include "lldb/API/SBCommandInterpreter.h"
# include "lldb/API/SBCommandReturnObject.h"
# include "lldb/API/SBCommunication.h"
# include "lldb/API/SBBroadcaster.h"
# include "lldb/API/SBDebugger.h"
# include "lldb/API/SBStructuredData.h"
# include "lldb/API/SBEvent.h"
# include "lldb/API/SBHostOS.h"
# include "lldb/API/SBLanguageRuntime.h"
# include "lldb/API/SBListener.h"
# include "lldb/API/SBProcess.h"
# include "lldb/API/SBStream.h"
# include "lldb/API/SBStringList.h"
# include "lldb/API/SBTarget.h"
# include "lldb/API/SBThread.h"

#ifdef HAVE_LLDB_UTILITY
# include "lldb/Utility/Stream.h"
# include "lldb/Utility/StringList.h"
# include "lldb/Utility/ArchSpec.h"
#endif

//# include "llvm/Support/ConvertUTF.h"
# include "llvm/Support/PrettyStackTrace.h"
# include "llvm/Support/Signals.h"
#endif

#include <thread>

#if !defined(__APPLE__)
#include "llvm/Support/DataTypes.h"
#endif

#include "config/enable_nls.h"
#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# define _(s) (s)
# define N_(s) (s)
#endif

#define ADB_ARCH_NAME_MAX 32

class Agardb : public lldb::SBBroadcaster {
public:
	Agardb();
	virtual ~Agardb();

	lldb::SBDebugger &GetDebugger() { return m_debugger; }

	void SetTerminalWidth(unsigned short col);

	static void Take_Screenshot(AG_Event *_Nonnull);
	static void Run_GUI_Debugger_GK(void);
	static void Run_GUI_Debugger(AG_Event *_Nonnull);
	static void Run_LLDB(lldb::SBStream &, bool);

	class GUI {
public:
		GUI();
		virtual ~GUI();

		static void Preferences(AG_Event *_Nonnull);
		static void MenuTargets(AG_Event *_Nonnull);
		static void SelectTarget(AG_Event *_Nonnull);
	};

private:
	lldb::SBDebugger m_debugger;			/* LLDB instance */
};

#endif /* _AGARDB_H_ */
