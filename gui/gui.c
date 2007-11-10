/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <config/have_opengl.h>

#include "box.h"
#include "button.h"
#include "checkbox.h"
#include "combo.h"
#include "console.h"
#include "file_dlg.h"
#include "fixed.h"
#include "fspinbutton.h"
#include "fixed_plotter.h"
#include "glview.h"
#include "hsvpal.h"
#include "icon.h"
#include "label.h"
#include "menu.h"
#include "mfspinbutton.h"
#include "mpane.h"
#include "mspinbutton.h"
#include "notebook.h"
#include "numerical.h"
#include "objsel.h"
#include "pane.h"
#include "pixmap.h"
#include "radio.h"
#include "scrollbar.h"
#include "separator.h"
#include "socket.h"
#include "spinbutton.h"
#include "statusbar.h"
#include "table.h"
#include "tableview.h"
#include "textbox.h"
#include "titlebar.h"
#include "tlist.h"
#include "toolbar.h"
#include "ucombo.h"

#include "colors.h"
#include "cursors.h"
#include "primitive.h"
#include "icons.h"
#include "icons_data.h"
#include "text.h"

const void *agGUIClasses[] = {
	&agWidgetOps,
	&agWindowOps,
	&agFontOps,
	&agBoxOps,
	&agButtonOps,
	&agCheckboxOps,
	&agComboOps,
	&agConsoleOps,
	&agFileDlgOps,
	&agFixedOps,
	&agFSpinbuttonOps,
	&agFixedPlotterOps,
#ifdef HAVE_OPENGL
	&agGLViewOps,
#endif
	&agHSVPalOps,
	&agIconOps,
	&agLabelOps,
	&agMenuOps,
	&agMenuViewOps,
	&agMFSpinbuttonOps,
	&agMPaneOps,
	&agMSpinbuttonOps,
	&agNotebookOps,
	&agNotebookTabOps,
	&agNumericalOps,
	&agObjectSelectorOps,
	&agPaneOps,
	&agPixmapOps,
	&agRadioOps,
	&agScrollbarOps,
	&agSeparatorOps,
	&agSocketOps,
	&agSpinbuttonOps,
	&agStatusbarOps,
	&agTitlebarOps,
	&agTableOps,
	&agTableviewOps,
	&agTextboxOps,
	&agTlistOps,
	&agToolbarOps,
	&agUComboOps,
	NULL
};

int
AG_InitGUI(Uint flags)
{
	char path[MAXPATHLEN];
	const void **ops;
	int i, n, njoys;

	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);
	
	/* Initialize the GUI subsystems. */
	AG_ColorsInit();
	AG_InitPrimitives();
	AG_CursorsInit();
	agIcon_Init();

	/* Try to load a color scheme from the default path. */
	strlcpy(path, AG_String(agConfig, "save-path"), sizeof(path));
	strlcat(path, AG_PATHSEP, sizeof(path));
	strlcat(path, "gui-colors.acs", sizeof(path));
	(void)AG_ColorsLoad(path);
	
	/* Initialize the font engine. */
	if (AG_TextInit() == -1) {
		return (-1);
	}

	/* Initialize the input devices. */
	SDL_EnableUNICODE(agKbdUnicode);
	if (AG_Bool(agConfig, "input.joysticks")) {
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
			n = SDL_NumJoysticks();
			for (i = 0, njoys = 0; i < n; i++) {
				if (SDL_JoystickOpen(i) != NULL)
					njoys++;
			}
			if (njoys > 0)
				SDL_JoystickEventState(SDL_ENABLE);
		}
	}
	return (0);

	return (0);
}
