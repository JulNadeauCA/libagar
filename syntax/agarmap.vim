" Vim syntax file
" Language:     LibAgar - Agar-Map C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarmap.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_map") || exists("c_agar_map_typedefs")
  " map/insert.h
  syn keyword cType MAP_InsertTool
  " map/map.h
  syn keyword cType MAP_ItemClass MAP_Item MAP_Node MAP_Layer MAP_Camera
  syn keyword cType MAP_Change MAP_Revision MAP
  syn keyword cConstant MAP_TILESZ_DEF MAP_TILESZ_MAX MAP_WIDTH_MAX
  syn keyword cConstant MAP_HEIGHT_MAX MAP_LAYERS_MAX MAP_CAMERAS_MAX
  syn keyword cConstant MAP_LAYER_NAME_MAX MAP_CAMERA_NAME_MAX
  syn keyword cConstant MAP_NODE_ITEMS_MAX MAP_OBJECT_ID_MAX
  syn keyword cConstant MAP_ITEM_TILE MAP_ITEM_IMG MAP_ITEM_LINK MAP_ITEM_PRIVATE
  syn keyword cConstant MAP_ITEM_LAST
  syn keyword cConstant MAP_ITEM_BLOCK MAP_ITEM_CLIMBABLE MAP_ITEM_SLIPPERY
  syn keyword cConstant MAP_ITEM_JUMPABLE MAP_ITEM_MOUSEOVER MAP_ITEM_SELECTED
  syn keyword cConstant MAP_ITEM_VALID MAP_NODE_SELECTED MAP_NODE_VALID
  syn keyword cConstant MAP_UPPER_LEFT MAP_MIDDLE_LEFT MAP_LOWER_LEFT
  syn keyword cConstant MAP_UPPER_RIGHT MAP_MIDDLE_RIGHT MAP_LOWER_RIGHT
  syn keyword cConstant MAP_CENTER MAP_LOWER_CENTER MAP_UPPER_CENTER
  syn keyword cConstant MAP_CHANGE_NODECHG MAP_CHANGE_LAYERADD MAP_CHANGE_LAYERDEL
  syn keyword cConstant MAP_CHANGE_LAST MAP_SAVE_CAM0POS MAP_SAVE_CAM0ZOOM
  syn keyword cConstant MAP_SAVED_FLAGS
  " map/map_img.h
  syn keyword cType MAP_Img
  " map/map_link.h
  syn keyword cType MAP_Link
  syn keyword cConstant MAP_LINK_ID_MAX
  " map/map_math.h
  syn keyword cConstant MAP_PI
  " map/map_object.h
  syn keyword cType MAP_Location MAP_Object MAP_ObjectView MAP_ObjectClass
  syn keyword cType MAP_Tile 
  syn keyword cConstant MAP_OBJECT_LOCATION_SELECTED MAP_OBJECT_LOCATION_VALID
  syn keyword cConstant MAP_OBJECT_VALID MAP_OBJECT_ATTACHED MAP_OBJECT_SELECTED
  syn keyword cConstant MAP_OBJECT_TOP MAP_OBJECT_BOTTOM MAP_OBJECT_FRONT
  syn keyword cConstant MAP_OBJECT_LEFT MAP_OBJECT_RIGHT MAP_OBJECT_BACK
  syn keyword cConstant MAP_OBJECT_ISOMETRIC MAP_OBJECT_DIMETRIC MAP_OBJECT_TRIMETRIC
  " map/mapview.h
  syn keyword cType MAP_ViewDrawCb MAP_View
  syn keyword cConstant MAP_VIEW_EDITION MAP_VIEW_EDIT_ATTRS MAP_VIEW_EDIT_ORIGIN
  syn keyword cConstant MAP_VIEW_PLAY MAP_VIEW_MODE_LAST
  syn keyword cConstant MAP_VIEW_EDIT MAP_VIEW_GRID MAP_VIEW_ENTER MAP_VIEW_NO_CURSOR
  syn keyword cConstant MAP_VIEW_NO_BMPSCALE MAP_VIEW_NO_BG MAP_VIEW_NO_NODESEL
  syn keyword cConstant MAP_VIEW_SET_ATTRS MAP_VIEW_SHOW_ORIGIN MAP_VIEW_NO_SCROLLBARS
  syn keyword cConstant MAP_VIEW_SELECTION_MOVING MAP_VIEW_SELECTION_MOVED
  " map/nodesel.h
  syn keyword cType MAP_NodeselTool
  " map/rg_feature.h
  syn keyword cType RG_FeatureOps RG_FeatureSketch RG_FeaturePixmap RG_Feature
  syn keyword cConstant RG_FEATURE_NAME_MAX RG_FEATURE_TYPE_MAX RG_FEATURE_AUTOREDRAW
  " map/rg_fill.h
  syn keyword cConstant RG_FILL_SOLID RG_FILL_HGRADIENT RG_FILL_VGRADIENT
  syn keyword cConstant RG_FILL_CGRADIENT RG_FILL_PATTERN
  " map/rg_math.h
  syn keyword cConstant RG_PI
  " map/rg_pixmap.h
  syn keyword cType RG_PixmapMod RG_PixmapUndoBlk RG_Brush RG_Pixmap 
  syn keyword cConstant RG_PIXMAP_NAME_MAX RG_PIXMAP_PIXEL_REPLACE
  syn keyword cConstant RG_PIXMAP_OVERLAY_ALPHA RG_PIXMAP_AVERAGE_ALPHA
  syn keyword cConstant RG_PIXMAP_DEST_ALPHA RG_PIXMAP_NO_BLENDING
  syn keyword cConstant RG_PIXMAP_BRUSH_MONO RG_PIXMAP_BRUSH_RGB
  syn keyword cConstant RG_PIXMAP_BRUSH_ONESHOT
  " map/rg_prim.h
  syn keyword cConstant RG_PRIM_OVERLAY_ALPHA RG_PRIM_AVERAGE_ALPHA
  syn keyword cConstant RG_PRIM_SRC_ALPHA RG_PRIM_DST_ALPHA
  " map/rg_sketch.h
  syn keyword cType RG_SketchMod RG_SketchUndoBlk RG_Sketch
  syn keyword cConstant RG_SKETCH_NAME_MAX RG_SKETCH_VERTEX_DISPLACEMENT
  " map/rg_texsel.h
  syn keyword cType RG_TextureSelector
  " map/rg_texture.h
  syn keyword cType RG_Texture
  syn keyword cConstant RG_TEXTURE_NAME_MAX RG_TEXTURE_REPEAT RG_TEXTURE_CLAMP
  syn keyword cConstant RG_TEXTURE_CLAMP_TO_EDGE RG_TEXTURE_CLAMP_TO_BORDER
  " map/rg_tile.h
  syn keyword cType RG_TileElement RG_Tile RG_TileVariant
  syn keyword cConstant RG_TILE_NAME_MAX RG_TILE_CLASS_MAX RG_TILE_ELEMENT_NAME_MAX
  syn keyword cConstant RG_TILE_SIZE_MIN RG_TILE_SIZE_MAX RG_TILE_FEATURE
  syn keyword cConstant RG_TILE_PIXMAP RG_TILE_SKETCH RG_SNAP_NONE RG_SNAP_TO_GRID
  syn keyword cConstant RG_TILE_SRCCOLORKEY RG_TILE_SRCALPHA RG_TILE_DIRTY
  syn keyword cConstant RG_TILE_SQUARE_ENDPOINT RG_TILE_ROUNDED_ENDPOINT
  syn keyword cConstant RG_TILE_BLOCK RG_TILE_CLIMBABLE RG_TILE_SLIPPERY
  syn keyword cConstant RG_TILE_JUMPABLE
  " map/rg_tileset.h
  syn keyword cType RG_Tileset
  syn keyword cConstant RG_TILESZ RG_TEMPLATE_NAME_MAX RG_TILESET_NAME_MAX
  syn keyword cConstant RG_TILE_ID_MAX RG_TILE_ID_MINREUSE
  " map/rg_tileview.h
  syn keyword cType RG_TileviewHandle RG_TileviewCtrl RG_TileviewToolOps
  syn keyword cType RG_TileviewBitmapToolOps RG_TileviewSketchToolOps
  syn keyword cType RG_TileviewTool RG_Tileview
  syn keyword cConstant RG_TILEVIEW_MIN_W RG_TILEVIEW_MIN_H
  syn keyword cConstant RG_TILEVIEW_POINT RG_TILEVIEW_RECTANGLE RG_TILEVIEW_RDIMENSIONS
  syn keyword cConstant RG_TILEVIEW_CIRCLE RG_TILEVIEW_VERTEX RG_TILEVIEW_INT_VAL
  syn keyword cConstant RG_TILEVIEW_INT_PTR RG_TILEVIEW_UINT_VAL RG_TILEVIEW_UINT_PTR
  syn keyword cConstant RG_TILEVIEW_FLOAT_VAL RG_TILEVIEW_FLOAT_PTR RG_TILEVIEW_DOUBLE_VAL
  syn keyword cConstant RG_TILEVIEW_DOUBLE_PTR RG_TILEVIEW_TILE_TOOL
  syn keyword cConstant RG_TILEVIEW_FEATURE_TOOL RG_TILEVIEW_SKETCH_TOOL
  syn keyword cConstant RG_TILEVIEW_PIXMAP_TOOL
  syn keyword cConstant RG_TILEVIEW_TILE_EDIT RG_TILEVIEW_FEATURE_EDIT
  syn keyword cConstant RG_TILEVIEW_SKETCH_EDIT RG_TILEVIEW_PIXMAP_EDIT
  syn keyword cConstant RG_TILEVIEW_ATTRIB_EDIT RG_TILEVIEW_LAYERS_EDIT
  syn keyword cConstant RG_TILEVIEW_NO_SCROLLING RG_TILEVIEW_HIDE_CONTROLS
  syn keyword cConstant RG_TILEVIEW_NO_TILING RG_TILEVIEW_NO_EXTENT
  syn keyword cConstant RG_TILEVIEW_NO_GRID RG_TILEVIEW_SET_ATTRIBS
  syn keyword cConstant RG_TILEVIEW_READONLY RG_TVPIXMAP_IDLE RG_TVPIXMAP_FREEHAND
  syn keyword cConstant RG_TVPIXMAP_ORTHOGONAL RG_TVPIXMAP_VERTICAL
  syn keyword cConstant RG_TVPIXMAP_HORIZONTAL RG_TVPIXMAP_DIAGONAL
  " map/rg_transform.h
  syn keyword cType RG_Transform
  syn keyword cConstant RG_TRANSFORM_MAX_ARGS RG_TRANSFORM_CHAIN_MAX
  syn keyword cConstant RG_TRANSFORM_MIRROR RG_TRANSFORM_FLIP RG_TRANSFORM_ROTATE
  syn keyword cConstant RG_TRANSFORM_RGB_INVERT
  " map/tool.h
  syn keyword cType MAP_ToolOps MAP_Tool MAP_ToolKeyBinding MAP_ToolMouseBinding
  syn keyword cConstant MAP_TOOL_STATUS_MAX MAP_TOOL_HIDDEN
endif

" 
" Agar-Micro (https://libagar.org/micro.html)
" 
if !exists("c_no_agar_micro") || exists("c_agar_micro_typedefs")
  syn keyword cType MA_Color MA_ColorOffset MA_Component MA_ComponentOffset
  syn keyword cType MA_Pixel MA_Pt MA_Rect MA_Rect2 MA_ClipRect MA_Driver
  syn keyword cType MA_DriverClass MA_SizeReq MA_SizeAlloc MA_Surface
  syn keyword cType MA_Widget MA_WidgetVec MA_FlagDescr MA_WidgetClass
  syn keyword cType MA_SizeSpec MA_DriverDUMMY MA_Window MA_PixelFormat
  syn keyword cType MA_WindowVec MA_WindowQ MA_PixelPacking MA_DriverC64
endif
