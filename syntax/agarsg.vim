" Vim syntax file
" Language:     LibAgar - Agar-SG C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarsg.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_sg") || exists("c_agar_sg_typedefs")
  " sg/sg.h
  syn keyword cType SG_GLVertex2 SG_GLVertex3 SG_Rotation SG_Action SG_NodeClass
  syn keyword cType SG_Node SG
  syn keyword cConstant SG_LEFT SG_RIGHT SG_CW SG_CCW SG_ACTION_NONE
  syn keyword cConstant SG_ACTION_MOVE SG_ACTION_MOVE_BEGIN SG_ACTION_MOVE_END
  syn keyword cConstant SG_ACTION_ZMOVE SG_ACTION_ZMOVE_BEGIN SG_ACTION_ZMOVE_END
  syn keyword cConstant SG_ACTION_ROTATE SG_ACTION_ROTATE_BEGIN SG_ACTION_ROTATE_END
  syn keyword cConstant SG_ACTION_SCALE SG_ACTION_SCALE_BEGIN SG_ACTION_SCALE_END
  syn keyword cConstant SG_ACTION_LAST SG_ACTION_SAVED SG_NODE_SELECTED 
  syn keyword cConstant SG_NODE_HIDE
  " sg/sg_camera.h
  syn keyword cType SG_CameraPolyMode SG_Camera
  syn keyword cConstant SG_CAMERA_POINTS SG_CAMERA_WIREFRAME SG_CAMERA_FLAT_SHADED
  syn keyword cConstant SG_CAMERA_SMOOTH_SHADED SG_CAMERA_DRAW
  syn keyword cConstant SG_CAMERA_ROT_I SG_CAMERA_ROT_J SG_CAMERA_ROT_K
  syn keyword cConstant SG_CAMERA_PERSPECTIVE SG_CAMERA_ORTHOGRAPHIC
  syn keyword cConstant SG_CAMERA_USER_PROJ SG_CAMERA_ROT_IGNORE 
  syn keyword cConstant SG_CAMERA_ROT_CIRCULAR SG_CAMERA_ROT_ELLIPTIC
  " sg/sg_cg_program.h
  syn keyword cType SG_CgProgram
  syn keyword cConstant SG_VERTEX_PROGRAM SG_FRAGMENT_PROGRAM
  " sg/sg_circle.h
  syn keyword cType SG_Circle
  " sg/sg_dummy.h
  syn keyword cType SG_Dummy
  " sg/sg_geom.h
  syn keyword cType SG_Geom
  syn keyword cConstant SG_GEOM_SAVED
  " sg/sg_image.h
  syn keyword cType SG_Image
  syn keyword cConstant SG_IMAGE_RECT SG_IMAGE_POLY SG_IMAGE_BILLBOARD
  syn keyword cConstant SG_IMAGE_NODUP SG_IMAGE_NODUP_ANIM SG_IMAGE_WIREFRAME
  syn keyword cConstant SG_IMAGE_HOLES SG_IMAGE_SAVE_SURFACE SG_IMAGE_NOAUTOSIZE
  syn keyword cConstant SG_IMAGE_SAVED
  " sg/sg_light.h
  syn keyword cType SG_Light
  " sg/sg_line.h
  syn keyword cType SG_Line
  " sg/sg_load_ply.h
  syn keyword cConstant SG_PLY_LOAD_VTX_NORMALS SG_PLY_LOAD_VTX_COLORS
  syn keyword cConstant SG_PLY_LOAD_TEXCOORDS SG_PLY_DUP_VERTICES
  " sg/sg_object.h
  syn keyword cType SG_Vertex SG_Edge SG_EdgeEnt SG_Facet SG_FacetEnt
  syn keyword cType SG_BSPNode SG_Object SG_ExtrudeMode
  syn keyword cConstant SG_FACET_NAME_MAX SG_EDGE_NAME_MAX SG_EDGE_SAVED
  syn keyword cConstant SG_EDGE_SELECTED SG_EDGE_HIGHLIGHTED SG_FACET_SELECTED
  syn keyword cConstant SG_FACET_HIGHLIGHTED SG_OBJECT_STATIC SG_OBJECT_NODUPVERTEX
  syn keyword cConstant SG_EXTRUDE_REGION SG_EXTRUDE_EDGES SG_EXTRUDE_VERTICES
  " sg/sg_palette.h
  syn keyword cType SG_Pigment SG_Mixture SG_Palette
  syn keyword cConstant SG_PALETTE_PIGMENTS_MAX
  " sg/sg_palette_view.h
  syn keyword cType SG_PaletteView
  syn keyword cConstant SG_PALETTE_VIEW_HFILL SG_PALETTE_VIEW_VFILL
  syn keyword cConstant SG_PALETTE_VIEW_EXPAND
  " sg/sg_plane.h
  syn keyword cType SG_Plane
  " sg/sg_point.h
  syn keyword cType SG_Point
  " sg/sg_polyball.h
  syn keyword cType SG_Polyball
  " sg/sg_polybox.h
  syn keyword cType SG_Polybox
  " sg/sg_polygon.h
  syn keyword cType SG_Polygon
  " sg/sg_program.h
  syn keyword cType SG_ProgramClass SG_Program
  " sg/sg_rectangle.h
  syn keyword cType SG_Rectangle
  " sg/sg_script.h
  syn keyword cType SG_ScriptInsn SG_ScriptFrame SG_Script
  syn keyword cConstant SG_INSN_NOOP SG_INSN_CREATE SG_INSN_DELETE SG_INSN_ACTION
  syn keyword cConstant SG_INSN_CAMACTION SG_INSN_LAST
  syn keyword cConstant SG_SCRIPT_INTERP_NONE SG_SCRIPT_INTERP_LINEAR
  syn keyword cConstant SG_SCRIPT_INSN_SUPPRESS SG_SCRIPT_INSN_SELECTED
  syn keyword cConstant SG_SCRIPT_INSN_SAVED SG_SCRIPT_SAVED
  " sg/sg_sphere.h
  syn keyword cType SG_Sphere
  " sg/sg_texture.h
  syn keyword cType SG_TextureSurface SG_TextureProgram SG_Texture
  syn keyword cConstant SG_TEXTURE_PROGS_MAX SG_TEXTURE_SURFACES_MAX
  syn keyword cConstant SG_TEXTURE_NOLIGHT SG_TEXTURE_SAVED
  " sg/sg_triangle.h
  syn keyword cType SG_Triangle
  " sg/sg_view.h
  syn keyword cType SG_ViewTexture SG_ViewList SG_ViewCamAction SG_View
  syn keyword cConstant SG_VIEW_STATUS_MAX SG_VIEW_HFILL SG_VIEW_VFILL
  syn keyword cConstant SG_VIEW_EXPAND SG_VIEW_NO_LIGHTING SG_VIEW_NO_DEPTH_TEST
  syn keyword cConstant SG_VIEW_UPDATE_PROJ SG_VIEW_PANNING SG_VIEW_CAMERA_STATUS
  syn keyword cConstant SG_VIEW_EDIT SG_VIEW_EDIT_STATUS SG_VIEW_MOVING
  syn keyword cConstant SG_VIEW_ROTATING SG_VIEW_TRANSFADE
  " sg/sg_voxel.h
  syn keyword cType SG_Voxel
  " sg/sg_widget.h
  syn keyword cType SG_Widget
  syn keyword cConstant SG_WIDGET_SQUARE SG_WIDGET_DISC
endif
