" Vim syntax file
" Language:     LibAgar - Agar-VG C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarvg.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_vg") || exists("c_agar_vg_typedefs")
  " vg/vg.h
  syn keyword cType VG_Vector VG_Rect VG_Color VG_IndexedColor VG_NodeOps
  syn keyword cType VG_Layer VG_Matrix VG_Node VG
  syn keyword cConstant VG_NAME_MAX VG_LAYER_NAME_MAX VG_STYLE_NAME_MAX
  syn keyword cConstant VG_TYPE_NAME_MAX VG_SYM_NAME_MAX VG_HANDLE_MAX
  syn keyword cConstant VG_ALIGN_TL VG_ALIGN_TC VG_ALIGN_TR
  syn keyword cConstant VG_ALIGN_ML VG_ALIGN_MC VG_ALIGN_MR
  syn keyword cConstant VG_ALIGN_BL VG_ALIGN_BC VG_ALIGN_BR
  " vg/vg_arc.h
  syn keyword cType VG_Arc
  " vg/vg_arc_tool.c
  syn keyword cType VG_ArcTool
  " vg/vg_circle.h
  syn keyword cType VG_Circle
  " vg/vg_circle_tool.c
  syn keyword cType VG_CircleTool
  " vg/vg_line.h
  syn keyword cType VG_Line
  syn keyword cConstant VG_LINE_SQUARE VG_LINE_BEVELED VG_LINE_ROUNDED
  syn keyword cConstant VG_LINE_MITERED
  " vg/vg_line_tool.c
  syn keyword cType VG_LineTool
  " vg/vg_math.h
  syn keyword cConstant VG_PI
  " vg/vg_point.h
  syn keyword cType VG_Point
  " vg/vg_point_tool.c
  syn keyword cType VG_PointTool
  " vg/vg_polygon.h
  syn keyword cType VG_Polygon
  " vg/vg_polygon_tool.c
  syn keyword cType VG_PolygonTool
  " vg/vg_snap.h
  syn keyword cConstant VG_FREE_POSITIONING VG_GRID VG_ENDPOINT VG_ENDPOINT_DISTANCE
  syn keyword cConstant VG_CLOSEST_POINT VG_CENTER_POINT VG_MIDDLE_POINT
  syn keyword cConstant VG_INTERSECTIONS_AUTO VG_INTERSECTIONS_MANUAL
  " vg/vg_text.h
  syn keyword cType VG_Text
  syn keyword cConstant VG_TEXT_MAX VG_TEXT_MAX_PTRS VG_FONT_FACE_MAX
  syn keyword cConstant VG_FONT_STYLE_MAX VG_FONT_SIZE_MIN VG_FONT_SIZE_MAX
  syn keyword cConstant VG_TEXT_BOLD VG_TEXT_ITALIC VG_TEXT_UNDERLINE VG_TEXT_SCALED
  " vg/vg_tool.h
  syn keyword cType VG_ToolOps VG_Tool VG_ToolCommand
  syn keyword cConstant VG_MOUSEMOTION_NOSNAP VG_BUTTONUP_NOSNAP VG_BUTTONDOWN_NOSNAP
  syn keyword cConstant VG_BUTTON_NOSNAP VG_NOSNAP VG_NOEDITCLEAR
  " vg/vg_view.h
  syn keyword cType VG_GridType VG_Grid VG_View 
  syn keyword cConstant VG_GRIDS_MAX VG_VIEW_STATUS_MAX VG_GRID_POINTS VG_GRID_LINES
  syn keyword cConstant VG_GRID_HIDE VG_GRID_UNDERSIZE
  syn keyword cConstant VG_VIEW_HFILL VG_VIEW_VFILL VG_VIEW_GRID VG_VIEW_EXTENTS
  syn keyword cConstant VG_VIEW_DISABLE_BG VG_VIEW_CONSTRUCTION VG_VIEW_EXPAND
endif
