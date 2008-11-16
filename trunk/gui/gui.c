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
#include "editable.h"
#include "file_dlg.h"
#include "file_selector.h"
#include "fixed.h"
#include "fspinbutton.h"
#include "fixed_plotter.h"
#include "font_selector.h"
#include "glview.h"
#include "graph.h"
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
#include "progress_bar.h"
#include "radio.h"
#include "scrollbar.h"
#include "scrollview.h"
#include "separator.h"
#include "slider.h"
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

void *agGUIClasses[] = {
	&agWidgetClass,
	&agWindowClass,
	&agFontClass,
	&agBoxClass,
	&agButtonClass,
	&agCheckboxClass,
	&agComboClass,
	&agConsoleClass,
	&agEditableClass,
	&agFontSelectorClass,
	&agFileDlgClass,
	&agFileSelectorClass,
	&agFixedClass,
	&agFSpinbuttonClass,
	&agFixedPlotterClass,
	&agGraphClass,
#ifdef HAVE_OPENGL
	&agGLViewClass,
#endif
	&agHSVPalClass,
	&agIconClass,
	&agLabelClass,
	&agMenuClass,
	&agMenuViewClass,
	&agMFSpinbuttonClass,
	&agMPaneClass,
	&agMSpinbuttonClass,
	&agNotebookClass,
	&agNotebookTabClass,
	&agNumericalClass,
	&agObjectSelectorClass,
	&agPaneClass,
	&agPixmapClass,
	&agProgressBarClass,
	&agRadioClass,
	&agScrollbarClass,
	&agScrollviewClass,
	&agSeparatorClass,
	&agSliderClass,
	&agSocketClass,
	&agSpinbuttonClass,
	&agStatusbarClass,
	&agTitlebarClass,
	&agTableClass,
	&agTableviewClass,
	&agTextboxClass,
	&agTlistClass,
	&agToolbarClass,
	&agUComboClass,
	NULL
};

/* Initialize Agar-GUI */
int
AG_InitGUI(Uint flags)
{
	char path[AG_PATHNAME_MAX];
	void **ops;
	int i, n, njoys;

	/* Register the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);

	/* Initialize the GUI subsystems. */
	AG_ColorsInit();
	AG_InitPrimitives();
	AG_CursorsInit();
	agIcon_Init();

	/* Try to load a color scheme from the default path. */
	Strlcpy(path, AG_GetString(agConfig,"save-path"), sizeof(path));
	Strlcat(path, AG_PATHSEP, sizeof(path));
	Strlcat(path, "gui-colors.acs", sizeof(path));
	(void)AG_ColorsLoad(path);

	/* Initialize the font engine. */
	if (AG_TextInit() == -1)
		return (-1);

	/* Initialize the input devices. */
	if (AG_GetBool(agConfig,"input.unicode")) {
		SDL_EnableUNICODE(1);
	}
	if (AG_GetBool(agConfig,"input.joysticks")) {
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

	/* Initialize the built-in theme. */
	AG_SetStyle(agView, &agStyleDefault);

	/* Initialize the Window system. */
	AG_InitWindowSystem();
	AG_MutexInitRecursive(&agAppMenuLock);

	return (0);
}

/* Release resources allocated by Agar-GUI */
void
AG_DestroyGUI(void)
{
	void **ops;
	
	AG_MutexDestroy(&agAppMenuLock);

	/* Destroy the GUI subsystems. */
	AG_DestroyWindowSystem();
	AG_TextDestroy();
	AG_CursorsDestroy();
	AG_ColorsDestroy();

	/* Unregister the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_UnregisterClass(*ops);
}
