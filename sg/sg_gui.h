/*	Public domain	*/

#ifndef _AGAR_SG_SG_GUI_H_
#define _AGAR_SG_SG_GUI_H_

#include <agar/gui/gui.h>
#include <agar/gui/icons.h>
#include <agar/gui/surface.h>
#include <agar/gui/primitive.h>
#include <agar/gui/load_surface.h>

#include <agar/gui/checkbox.h>
#include <agar/gui/console.h>
#include <agar/gui/dir_dlg.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/font_selector.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/menu.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/objsel.h>
#include <agar/gui/pixmap.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/slider.h>
#include <agar/gui/radio.h>
#include <agar/gui/table.h>
#include <agar/gui/separator.h>

#include <agar/sg/sg_view.h>
#include <agar/sg/sg_palette_view.h>
#include <agar/sg/icons.h>

#include <agar/sg/begin.h>

__BEGIN_DECLS
extern AG_Object sgVfsRoot;			/* General-purpose VFS */

void       SG_InitGUI(void);
void       SG_DestroyGUI(void);

void       SG_FileMenu(AG_MenuItem *, void *, AG_Window *);
void       SG_EditMenu(AG_MenuItem *, void *, AG_Window *);
void       SG_ViewMenu(AG_MenuItem *, void *, AG_Window *, SG_View *);

AG_Window *SG_GUI_OpenObject(void *);
void       SG_GUI_NewObject(AG_Event *);
AG_Object *SG_GUI_LoadObject(AG_ObjectClass *, const char *);
void       SG_GUI_OpenDlg(AG_Event *);
void       SG_GUI_SaveAsDlg(AG_Event *);
void       SG_GUI_Save(AG_Event *);
void       SG_GUI_Quit(AG_Event *);
void       SG_GUI_Undo(AG_Event *);
void       SG_GUI_Redo(AG_Event *);
void       SG_GUI_EditPreferences(AG_Event *);
void       SG_GUI_SelectFontDlg(AG_Event *);
void       SG_GUI_CreateNewDlg(AG_Event *);

void       SG_GUI_PollNodes(AG_Event *);
void       SG_GUI_CreateNewView(AG_Event *);
SG_Node   *SG_GUI_CreateNode(SG *, AG_ObjectClass *);
void	   SG_GUI_DeleteNode(SG_Node *, SG_View *);
void       SG_GUI_NodePopupMenu(AG_Event *);
void       SG_GUI_EditNode(SG_Node *, AG_Widget *, SG_View *);
__END_DECLS
#include <agar/sg/close.h>

#endif /* _AGAR_SG_SG_GUI_H_ */
