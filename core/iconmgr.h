/*	$Csoft: icons.h,v 1.26 2005/09/19 07:13:36 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ICONMGR_H_
#define _AGAR_ICONMGR_H_
#include "begin_code.h"

enum {
	/* map-edition.xcf */
	OBJECT_EDITOR_ICON,
	STAMP_TOOL_ICON,
	ERASER_TOOL_ICON,
	NEW_VIEW_ICON,
	MAGNIFIER_TOOL_ICON,
	RESIZE_TOOL_ICON,
	GRID_ICON,
	PROPS_ICON,
	PROPEDIT_ICON,
	EDIT_ICON,
	RIGHT_ARROW_ICON,
	LEFT_ARROW_ICON,
	UP_ARROW_ICON,
	DOWN_ARROW_ICON,
	NODE_EDITOR_ICON,
	LAYER_EDITOR_ICON,
	SELECT_NODE_ICON,
	SELECT_REF_ICON,
	MERGE_TOOL_ICON,
	FILL_TOOL_ICON,
	MIRROR_TOOL_ICON,
	FLIP_TOOL_ICON,
	MEDIASEL_ICON,
	POSITION_TOOL_ICON,
	INVERT_TOOL_ICON,
	SETTINGS_ICON,
	CROP_TOOL_ICON,

	/* object-editor.xcf */
	OBJCREATE_ICON,
	OBJEDIT_ICON,
	OBJGENEDIT_ICON,
	OBJLOAD_ICON,
	OBJSAVE_ICON,
	OBJDUP_ICON,
	OBJMOVEUP_ICON,
	OBJMOVEDOWN_ICON,
	TRASH_ICON,
	OBJREINIT_ICON,
	CLOSE_ICON,
	FILE_ICON,
	SYMLINK_ICON,
	DIRECTORY_ICON,

	/* objects.xcf */
	OBJ_ICON,
	MAP_ICON,
	DRAWING_ICON,
	TILESET_ICON,
	PERSO_ICON,

	/* vg.xcf */
	VGORIGIN_ICON,
	VGPOINTS_ICON,
	VGLINES_ICON,
	VGTRIANGLES_ICON,
	VGCIRCLES_ICON,
	VGBEZIER_CURVE_ICON,
	VGBLOCK_ICON,
	VGTEXT_ICON,
	VGTL_ICON,
	VGTC_ICON,
	VGTR_ICON,
	VGML_ICON,
	VGMC_ICON,
	VGMR_ICON,
	VGBL_ICON,
	VGBC_ICON,
	VGBR_ICON,

	/* vg-snap.xcf */
	SNAP_FREE_ICON,
	SNAP_RINT_ICON,
	SNAP_GRID_ICON,
	SNAP_ENDPOINT_ICON,
	SNAP_ENDPOINT_D_ICON,
	SNAP_CLOSEST_ICON,
	SNAP_CENTERPT_ICON,
	SNAP_MIDDLEPT_ICON,
	SNAP_INTSECT_AUTO_ICON,
	SNAP_INTSECT_MANUAL_ICON,

	/* eda.xcf */
	EDA_CIRCUIT_ICON,
	EDA_COMPONENT_ICON,
	EDA_INSERT_COMPONENT_ICON,
	EDA_REMOVE_COMPONENT_ICON,
	EDA_NODE_ICON,
	EDA_BRANCH_TO_NODE_ICON,
	EDA_BRANCH_TO_COMPONENT_ICON,
	EDA_START_SIM_ICON,
	EDA_STOP_SIM_ICON,
	EDA_SIM_SETTINGS_ICON,
	EDA_MESH_ICON,
	EDA_SPICE_ICON,

	/* rg.xcf */
	RG_HGRADIENT_ICON,
	RG_VGRADIENT_ICON,
	RG_CGRADIENT_ICON,
	RG_TILING_ICON,
	RG_FILL_ICON,
	RG_SKETCH_PROJ_ICON,
	RG_SKETCH_ICON,
	RG_SKETCH_ATTACH_ICON,
	RG_PIXMAP_ICON,
	RG_PIXMAP_RESIZE_ICON,
	RG_PIXMAP_ATTACH_ICON,
	RG_POLYGON_ICON,
	RG_EXTRUSION_ICON,
	RG_REVOLUTION_ICON,
	RG_SWAP_ICON,
	RG_INVERT_ICON,
	RG_CONTROLS_ICON,
	RG_PROPS_ICON,
	WALKABILITY_ICON,
	CLIMBABILITY_ICON,
	JUMPABILITY_ICON,
	SLIPPAGE_ICON,

	/* anim.xcf */
	ANIM_PLAY_ICON,
	ANIM_PAUSE_ICON,
	ANIM_STOP_ICON,
	ANIM_REWIND_ICON,
	ANIM_FORWARD_ICON,
	ANIM_REWIND_F_ICON,
	ANIM_FORWARD_F_ICON,

	/* symbols.xcf */
	LEFT_BUTTON_SYMBOL,
	MID_BUTTON_SYMBOL,
	RIGHT_BUTTON_SYMBOL,
	CTRL_SYMBOL,

	/* arrow.xcf */
	GUI_ARROW_ICON,

	/* window.xcf */
	GUI_CLOSE_ICON,
	GUI_HIDE_WINDOW_ICON,
	GUI_SHOW_WINDOW_ICON,

	LAST_ICON
};

typedef struct ag_icon_mgr {
	struct ag_object obj;
	SDL_Surface **icons;
	Uint nicons;
} AG_IconMgr;

extern AG_IconMgr agIconMgr;

#ifdef DEBUG
#define AGICON(n) ((n>=0 && n<LAST_ICON) ? agIconMgr.icons[n] : NULL)
#else
#define AGICON(n) agIconMgr.icons[n]
#endif

__BEGIN_DECLS
void AG_IconMgrInit(void *, const char *);
int AG_IconMgrLoadFromDenXCF(AG_IconMgr *, const char *);
void AG_IconMgrDestroy(void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ICONMGR_H_ */
