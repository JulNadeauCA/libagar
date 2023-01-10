" Vim syntax file
" Language:     LibAgar - Agar-SK C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarsk.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_sk") || exists("c_agar_sk_typedefs")
  " sk/sk.h
  syn keyword cType SK_Status SK_NodeOps SK_Node SK_NodePair SK_ConstraintType
  syn keyword cType SK_Constraint SK_Cluster SK_Insn SK_IntersectFn SK_GeometryFn
  syn keyword cType SK SK_Annot SK_Arc SK_Circle 
  syn keyword cType SK_Dummy SK_Group SK_Line SK_Pixmap SK_ConstraintPairFn
  syn keyword cType SK_ConstraintRingFn SK_Point SK_Polygon
  syn keyword cConstant SK_TYPE_NAME_MAX SK_NODE_NAME_MAX SK_NAME_MAX SK_STATUS_MAX
  syn keyword cConstant SK_GROUP_NAME_MAX SK_INVALID SK_WELL_CONSTRAINED
  syn keyword cConstant SK_UNDER_CONSTRAINED SK_OVER_CONSTRAINED
  syn keyword cConstant SK_NODE_SELECTED SK_NODE_MOUSEOVER SK_NODE_MOVED
  syn keyword cConstant SK_NODE_SUPCONSTRAINTS SK_NODE_FIXED SK_NODE_KNOWN
  syn keyword cConstant SK_NODE_CHECKED SK_DISTANCE SK_INCIDENT SK_ANGLE
  syn keyword cConstant SK_PERPENDICULAR SK_PARALLEL SK_TANGENT SK_CONSTRAINT_LAST
  syn keyword cConstant SK_CONSTRAINT_ANY SK_COMPOSE_PAIR SK_COMPOSE_RING
  syn keyword cConstant SK_SKIP_UNKNOWN_NODES
  " sk/sk_dimension.h
  syn keyword cType SK_Dimension SK_DimensionView
  syn keyword cConstant SK_DIMENSION_TEXT_MAX
  syn keyword cConstant SK_DIMENSION_NONE SK_DIMENSION_DISTANCE
  syn keyword cConstant SK_DIMENSION_ANGLE_ENDPOINT SK_DIMENSION_ANGLE_INTERSECT
  syn keyword cConstant SK_DIMENSION_CONJ_ANGLE
  " sk/sk_tool.h
  syn keyword cType SK_ToolOps SK_Tool SK_ToolKeyBinding SK_ToolMouseBinding
  syn keyword cConstant SK_MOUSEMOTION_NOSNAP SK_BUTTONUP_NOSNAP 
  syn keyword cConstant SK_BUTTONDOWN_NOSNAP SK_BUTTON_NOSNAP SK_NOSNAP
  " sk/sk_view.h
  syn keyword cType SK_View
  syn keyword cConstant SK_VIEW_HFILL SK_VIEW_VFILL SK_VIEW_PANNING SK_VIEW_EXPAND
  syn keyword cConstant SK_VIEW_STATUS_MAX
endif
