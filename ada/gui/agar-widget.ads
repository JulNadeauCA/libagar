------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                          A G A R  . W I D G E T                          --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with System;
with Agar.Object;
with Agar.Event;
with Agar.Surface;
with Agar.Keyboard;
with Agar.Mouse;

--
-- Base class for all Agar GUI widgets.
--

package Agar.Widget is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package OBJ renames Agar.Object;
  package EV renames Agar.Event;
  package SU renames Agar.Surface;
  package KBD renames Agar.Keyboard;
  package MSE renames Agar.Mouse;

  use type C.int;
  use type C.unsigned;
  use type C.C_float;
  
  type Window;
  type Window_Access is access all Window with Convention => C;
  subtype Window_not_null_Access is not null Window_Access;
  
  type Widget;
  type Widget_Access is access all Widget with Convention => C;
  subtype Widget_not_null_Access is not null Widget_Access;
  
  -----------------------------
  -- Widget Size Requisition --
  -----------------------------
  type SizeReq is limited record
    W : C.int;				-- Width (px)
    H : C.int;				-- Height (px)
  end record
    with Convention => C;

  type SizeReq_Access is access all SizeReq with Convention => C;
  subtype SizeReq_not_null_Access is not null SizeReq_Access;
  
  ----------------------------
  -- Widget Size Allocation --
  ----------------------------
  type SizeAlloc is limited record
    W,H : C.int;			-- Size (px)
    X,Y : C.int;			-- Position in parent
  end record
    with Convention => C;

  type SizeAlloc_Access is access all SizeAlloc with Convention => C;
  subtype SizeAlloc_not_null_Access is not null SizeAlloc_Access;
  
  ----------------------
  -- Low-Level Driver --
  ----------------------
  type Driver_Type_t is
    (FRAME_BUFFER,                               -- By video memory access
     VECTOR);                                    -- By vector drawing commands
  for Driver_Type_t'Size use C.int'Size;

  type Driver_WM_Type_t is
    (WM_SINGLE,                                  -- Single-window (built-in WM)
     WM_MULTIPLE);                               -- Multi-windows
  for Driver_WM_Type_t'Size use C.int'Size;
  
  type Driver_Event;
  type Driver_Event_Access is access all Driver_Event with Convention => C;
  subtype Driver_Event_not_null_Access is not null Driver_Event_Access;
  
  ------------------
  -- Mouse Cursor --
  ------------------
  type Cursor;
  type Cursor_Access is access all Cursor with Convention => C;
  subtype Cursor_not_null_Access is not null Cursor_Access;

  type Cursor_Data_Access is access all Unsigned_8 with Convention => C;
  type Cursor_Entry is limited record
    Next : Cursor_Access;
    Prev : access Cursor_Access;
  end record
    with Convention => C;

  type Cursor_List is limited record
    First : Cursor_Access;
    Last  : access Cursor_Access;
  end record
    with Convention => C;

  type Cursor is limited record
    W,H             : C.unsigned;
    Bitmap          : Cursor_Data_Access;
    Mask            : Cursor_Data_Access;
    Hot_X, Hot_Y    : C.int;
    Driver_Data     : System.Address;
    Entry_in_Driver : Cursor_Entry;
  end record
    with Convention => C;
  
  -----------------------------------------------------
  -- Widget Color Palette (4 states x 8 = 32 colors) --
  -----------------------------------------------------
  type Widget_State is
    (DEFAULT_STATE,			-- Not focused (default state)
     DISABLED_STATE,			-- Disabled (#disabled)
     FOCUSED_STATE,			-- Holds focus (#focused)
     HOVER_STATE);                      -- Cursor is over (#hover)
  for Widget_State'Size use C.int'Size;

  type Widget_Color is
    (FG_COLOR,            -- Foreground primary      ("color")
     BG_COLOR,            -- Background primary      ("background-color")
     TEXT_COLOR,          -- Text and vector icons   ("text-color")
     LINE_COLOR,          -- Lines and filled shapes ("line-color")
     HIGH_COLOR,          -- Top and left shading    ("high-color")
     LOW_COLOR,           -- Bottom/right shading    ("low-color")
     SELECTION_COLOR,     -- Selection primary       ("selection-color")
     UNUSED_COLOR);       -- Currently unused

  -- TODO
  type Widget_Palette is array (1 .. $SIZEOF_AG_WidgetPalette)
    of aliased Unsigned_8 with Convention => C;
  for Widget_Palette'Size use $SIZEOF_AG_WidgetPalette * System.Storage_Unit;
 
  -----------------------------
  -- GL & Texture Management --
  -----------------------------
#if HAVE_OPENGL
  type Widget_GL_Projection_Matrix is array (1 .. 16) of aliased C.C_float with Convention => C;
  type Widget_GL_Modelview_Matrix is array (1 .. 16) of aliased C.C_float with Convention => C;
  type Widget_GL_Context is limited record
    Projection : Widget_GL_Projection_Matrix;
    Modelview  : Widget_GL_Modelview_Matrix;
  end record
    with Convention => C;
  type Widget_GL_Context_Access is access all Widget_GL_Context with Convention => C;
#end if;
  subtype Surface_Handle is C.int;
  subtype Texture_Handle is C.unsigned;
  type Texture_Handle_Access is access all Texture_Handle with Convention => C;
  subtype Texture_Handle_not_null_Access is not null Texture_Handle_Access;

  type Texture_Coord is limited record
    X,Y : C.C_float;
    W,H : C.C_float;
  end record
    with Convention => C;

  type Texture_Coord_Access is access all Texture_Coord with Convention => C;
  subtype Texture_Coord_not_null_Access is not null Texture_Coord_Access;
  
  -------------------------------
  -- Low Level Driver Instance --
  -------------------------------
  type Driver is limited record
    Super         : aliased OBJ.Object;          -- [Object -> Driver]
    Instance_ID   : C.unsigned;                  -- Driver instance ID
    Flags         : C.unsigned;                  -- Flags (below)
    Ref_Surface   : SU.Surface_Access;           -- Standard surface format
    Video_Format  : SU.Pixel_Format;             -- Video format (FB modes only)
    Keyboard_Dev  : KBD.Keyboard_Device_Access;  -- Keyboard device
    Mouse_Dev     : MSE.Mouse_Device_Access;     -- Mouse device
    Glyph_Cache   : System.Address;              -- Cached AG_Glyph store
    GL_Context    : System.Address;              -- TODO: AG_GL_Context
    Active_Cursor : Cursor_Access;               -- Effective cursor
    Cursors       : Cursor_List;                 -- All registered cursors
    Cursor_Count  : C.unsigned;                  -- Total cursor count
    C_Pad1        : Interfaces.Unsigned_32;
  end record
    with Convention => C;
  
  -- Flags --
  DRIVER_WINDOW_BG : constant C.unsigned := 16#02#; -- Managed window background
 
  type Driver_Access is access all Driver with Convention => C;
  subtype Driver_not_null_Access is not null Driver_Access;
  
  ------------------------
  -- Generic Driver Ops --
  ------------------------
  type Open_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Spec   : CS.chars_ptr) return C.int with Convention => C;

  type Close_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Get_Display_Size_Func_Access is access function
    (W,H : access C.unsigned) return C.int with Convention => C;

  --------------------------------
  -- Low-Level Event Processing --
  --------------------------------
  type Begin_Event_Processing_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Pending_Events_Func_Access is access function
    (Driver : Driver_not_null_Access) return C.int with Convention => C;

  type Get_Next_Event_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Event  : Driver_Event_not_null_Access) return C.int with Convention => C;

  type Process_Event_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Event  : Driver_Event_not_null_Access) return C.int with Convention => C;

  type Generic_Event_Loop_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type End_Event_Processing_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Terminate_Func_Access is access procedure with Convention => C;

  -------------------
  -- Rendering Ops --
  -------------------
  type Begin_Rendering_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Render_Window_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type End_Rendering_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Fill_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Update_Region_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access) with Convention => C;

  type Upload_Texture_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Texture  : Texture_Handle_not_null_Access;
     Surface  : SU.Surface_not_null_Access;
     TexCoord : Texture_Coord_Access) with Convention => C;

  type Update_Texture_Func_Access is access function
    (Driver   : Driver_not_null_Access;
     Texture  : C.unsigned;
     Surface  : SU.Surface_not_null_Access;
     TexCoord : Texture_Coord_Access) return C.int with Convention => C;

  type Delete_Texture_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Texture  : C.unsigned) with Convention => C;

  type Set_Refresh_Rate_Func_Access is access function
    (Driver : Driver_not_null_Access;
     FPS    : C.int) return C.int with Convention => C;

  ---------------------------
  -- Clipping and Blending --
  ---------------------------
  type Push_Clip_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access) with Convention => C;

  type Pop_Clip_Rect_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Push_Blending_Mode_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Pop_Blending_Mode_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  ----------------------
  -- Hardware Cursors --
  ----------------------
  type Create_Cursor_Func_Access is access function
    (Driver        : Driver_not_null_Access;
     W,H           : C.unsigned;
     Data, Bitmask : SU.Pixel_Access;
     Hot_X,Hot_Y   : C.int) return Cursor_Access with Convention => C;

  type Free_Cursor_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Cursor : Cursor_not_null_Access) with Convention => C;

  type Set_Cursor_Func_Access is access function
    (Driver : Driver_not_null_Access;
     Cursor : Cursor_not_null_Access) return C.int with Convention => C;

  type Unset_Cursor_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;

  type Get_Cursor_Visibility_Func_Access is access function
    (Driver : Driver_not_null_Access) return C.int with Convention => C;

  type Set_Cursor_Visibility_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Enable : C.int) with Convention => C;

  -------------------------
  -- Surfaces / Textures --
  -------------------------
  type Blit_Surface_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : SU.Surface_not_null_Access;
     X,Y     : C.int) with Convention => C;

  type Blit_Surface_From_Func_Access is access procedure
    (Driver   : Driver_not_null_Access;
     Widget   : Widget_not_null_Access;
     Source   : Widget_not_null_Access;
     Surface  : C.int;
     Src_Rect : SU.Rect_Access;
     X,Y      : C.int) with Convention => C;

  type Blit_Surface_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : SU.Surface_not_null_Access;
     W,H     : C.C_float) with Convention => C;

  type Blit_Surface_From_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : C.int;
     W,H     : C.C_float) with Convention => C;

  type Blit_Surface_Flipped_GL_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : C.int;
     W,H     : C.C_float) with Convention => C;

  type Backup_Surfaces_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Widget : Widget_not_null_Access) with Convention => C;

  type Restore_Surfaces_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Widget : Widget_not_null_Access) with Convention => C;

  type Render_to_Surface_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Widget  : Widget_not_null_Access;
     Surface : access SU.Surface_Access) with Convention => C;

  -------------------
  -- Rendering Ops --
  -------------------
  type Put_Pixel_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Color   : SU.Color_not_null_Access) with Convention => C;

  type Put_Pixel_32_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Value   : Interfaces.Unsigned_32) with Convention => C;

  type Put_Pixel_RGB8_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     R,G,B   : Unsigned_8) with Convention => C;

#if AG_MODEL = AG_LARGE
  type Put_Pixel_64_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     Value   : Interfaces.Unsigned_64) with Convention => C;

  type Put_Pixel_RGB16_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     X,Y     : C.int;
     R,G,B   : Interfaces.Unsigned_16) with Convention => C;
#end if;

  type Blend_Pixel_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X,Y       : C.int;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Draw_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X1,Y1  : C.int;
     X2,Y2  : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Horizontal_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X1,X2  : C.int;
     Y      : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Vertical_Line_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X      : C.int;
     Y1,Y2  : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Blended_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;
  
  type Draw_Wide_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Width     : C.C_float) with Convention => C;

  type Draw_Wide_Sti16_Line_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     X1,Y1     : C.int;
     X2,Y2     : C.int;
     Color     : SU.Color_not_null_Access;
     Width     : C.C_float;
     Stipple   : Unsigned_16) with Convention => C;

  type Draw_Triangle_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     V1,V2,V3  : SU.AG_Pt_not_null_Access;
     Color     : SU.Color_not_null_Access) with Convention => C;

  type Draw_Polygon_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Points : SU.AG_Pt_not_null_Access;
     Count  : C.unsigned;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Polygon_Sti32_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Points  : SU.AG_Pt_not_null_Access;
     Count   : C.unsigned;
     Color   : SU.Color_not_null_Access;
     Stipple : System.Address) with Convention => C;

  type Draw_Arrow_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Angle   : C.C_float;
     X,Y     : C.int;
     H       : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Box_Rounded_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Rect    : SU.Rect_not_null_Access;
     Z       : C.int;
     Radius  : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Box_Rounded_Top_Func_Access is access procedure
    (Driver  : Driver_not_null_Access;
     Rect    : SU.Rect_not_null_Access;
     Z       : C.int;
     Radius  : C.int;
     Color_A : SU.Color_not_null_Access;
     Color_B : SU.Color_not_null_Access) with Convention => C;

  type Draw_Circle_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X,Y    : C.int;
     Radius : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Circle_Filled_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     X,Y    : C.int;
     Radius : C.int;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Rect_Filled_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Rect   : SU.Rect_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  type Draw_Rect_Blended_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Rect      : SU.Rect_not_null_Access;
     Color     : SU.Color_not_null_Access;
     Source_Fn : SU.Alpha_Func;
     Dest_Fn   : SU.Alpha_Func) with Convention => C;

  type Draw_Rect_Dithered_Func_Access is access procedure
    (Driver    : Driver_not_null_Access;
     Rect      : SU.Rect_not_null_Access;
     Color     : SU.Color_not_null_Access) with Convention => C;

  type Update_Glyph_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Glyph  : System.Address) with Convention => C;

  type Draw_Glyph_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Glyph  : System.Address;
     X,Y    : C.int) with Convention => C;

  type Delete_List_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     List   : C.unsigned) with Convention => C;

  --------------------------
  -- Generic Driver Class --
  --------------------------
  type Driver_Class is limited record
    Super       : aliased OBJ.Class;                  -- [Driver]
    Name        : CS.chars_ptr;
    Driver_Type : Driver_Type_t;
    WM_Type     : Driver_WM_Type_t;
#if HAVE_64BIT
    Flags       : Interfaces.Unsigned_64;             -- DRIVER_* flags (below)
#else
    Flags       : C.unsigned;
#end if;
    -- Initialization --
    Open_Func        : Open_Func_Access;
    Close_Func       : Close_Func_Access;
    Get_Display_Size : Get_Display_Size_Func_Access;

    -- Low-level Events --
    Begin_Event_Processing : Begin_Event_Processing_Func_Access;
    Pending_Events         : Pending_Events_Func_Access;
    Get_Next_Event         : Get_Next_Event_Func_Access;
    Process_Event          : Process_Event_Func_Access;
    Generic_Event_Loop     : Generic_Event_Loop_Func_Access;
    End_Event_Processing   : End_Event_Processing_Func_Access;
    Terminate_Func         : Terminate_Func_Access;

    -- Rendering Ops --
    Begin_Rendering  : Begin_Rendering_Func_Access;
    Render_Window    : Render_Window_Func_Access;
    End_Rendering    : End_Rendering_Func_Access;
    Fill_Rect        : Fill_Rect_Func_Access;
    Update_Region    : Update_Region_Func_Access;
    Upload_Texture   : Upload_Texture_Func_Access;
    Update_Texture   : Update_Texture_Func_Access;
    Delete_Texture   : Delete_Texture_Func_Access;
    Set_Refresh_Rate : Set_Refresh_Rate_Func_Access;

    -- Clipping and Blending --
    Push_Clip_Rect     : Push_Clip_Rect_Func_Access;
    Pop_Clip_Rect      : Pop_Clip_Rect_Func_Access;
    Push_Blending_Mode : Push_Blending_Mode_Func_Access;
    Pop_Blending_Mode  : Pop_Blending_Mode_Func_Access;

    -- Hardware Cursors --
    Create_Cursor         : Create_Cursor_Func_Access;
    Free_Cursor           : Free_Cursor_Func_Access;
    Set_Cursor            : Set_Cursor_Func_Access;
    Unset_Cursor          : Unset_Cursor_Func_Access;
    Get_Cursor_Visibility : Get_Cursor_Visibility_Func_Access;
    Set_Cursor_Visibility : Set_Cursor_Visibility_Func_Access;

    -- Surface / Textures --
    Blit_Surface            : Blit_Surface_Func_Access;
    Blit_Surface_From       : Blit_Surface_From_Func_Access;
    Blit_Surface_GL         : Blit_Surface_GL_Func_Access;
    Blit_Surface_From_GL    : Blit_Surface_From_GL_Func_Access;
    Blit_Surface_Flipped_GL : Blit_Surface_Flipped_GL_Func_Access;
    Backup_Surfaces         : Backup_Surfaces_Func_Access;
    Restore_Surfaces        : Restore_Surfaces_Func_Access;
    Render_to_Surface       : Render_to_Surface_Func_Access;

    -- Rendering Ops --
    Put_Pixel            : Put_Pixel_Func_Access;
    Put_Pixel_32         : Put_Pixel_32_Func_Access;
    Put_Pixel_RGB8       : Put_Pixel_RGB8_Func_Access;
#if AG_MODEL = AG_LARGE
    Put_Pixel_64         : Put_Pixel_64_Func_Access;
    Put_Pixel_RGB16      : Put_Pixel_RGB16_Func_Access;
#end if;
    Blend_Pixel          : Blend_Pixel_Func_Access;
    Draw_Line            : Draw_Line_Func_Access;
    Draw_Horizonal_Line  : Draw_Horizontal_Line_Func_Access;
    Draw_Vertical_Line   : Draw_Vertical_Line_Func_Access;
    Draw_Blended_Line    : Draw_Blended_Line_Func_Access;
    Draw_Wide_Line       : Draw_Wide_Line_Func_Access;
    Draw_Wide_Sti16_Line : Draw_Wide_Sti16_Line_Func_Access;
    Draw_Triangle        : Draw_Triangle_Func_Access;
    Draw_Polygon         : Draw_Polygon_Func_Access;
    Draw_Polygon_Sti32   : Draw_Polygon_Sti32_Func_Access;
    Draw_Arrow           : Draw_Arrow_Func_Access;
    Draw_Box_Rounded     : Draw_Box_Rounded_Func_Access;
    Draw_Box_Rounded_Top : Draw_Box_Rounded_Top_Func_Access;
    Draw_Circle          : Draw_Circle_Func_Access;
    Draw_Circle_Filled   : Draw_Circle_Filled_Func_Access;
    Draw_Rect_Filled     : Draw_Rect_Filled_Func_Access;
    Draw_Rect_Blended    : Draw_Rect_Blended_Func_Access;
    Draw_Rect_Dithered   : Draw_Rect_Dithered_Func_Access;
    Update_Glyph         : Update_Glyph_Func_Access;
    Draw_Glyph           : Draw_Glyph_Func_Access;
    Delete_List          : Delete_List_Func_Access;
  end record
    with Convention => C;

  -- Flags --
  DRIVER_OPENGL   : constant C.unsigned := 16#01#;  -- OpenGL supported
  DRIVER_SDL      : constant C.unsigned := 16#02#;  -- SDL 1.x calls supported
  DRIVER_TEXTURES : constant C.unsigned := 16#04#;  -- Texture mgmt supported

  type Driver_Class_Access is access all Driver_Class with Convention => C;
  subtype Driver_Class_not_null_Access is not null Driver_Class_Access;
  
  -----------------------------
  -- Multi-Window Driver Ops --
  -----------------------------

  type Open_Window_Func_Access is access function
    (Window         : Window_not_null_Access;
     Geometry       : SU.Rect_not_null_Access;
     Bits_per_Pixel : C.int;
     Flags          : C.unsigned) return C.int with Convention => C;

  -- Flags --
  DRIVER_MW_ANYPOS       : constant C.unsigned := 16#01#;  -- Autoposition window
  DRIVER_MW_ANYPOS_AVAIL : constant C.unsigned := 16#02#;  -- Autopos supported

  type Close_Window_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type Map_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Unmap_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Raise_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Lower_Window_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Reparent_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     Parent : Window_not_null_Access;
     X,Y    : C.int) return C.int with Convention => C;

  type Get_Input_Focus_Func_Access is access function 
    (Window : access Window_not_null_Access) return C.int with Convention => C;

  type Set_Input_Focus_Func_Access is access function 
    (Window : Window_not_null_Access) return C.int with Convention => C;

  type Move_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     X,Y    : C.int) return C.int with Convention => C;

  type Resize_Window_Func_Access is access function 
    (Window : Window_not_null_Access;
     W,H    : C.unsigned) return C.int with Convention => C;

  type Move_Resize_Window_Func_Access is access function 
    (Window   : Window_not_null_Access;
     Geometry : SizeAlloc_Access) return C.int with Convention => C;

  type Pre_Resize_Callback_Func_Access is access procedure
    (Window : Window_not_null_Access) with Convention => C;

  type Post_Resize_Callback_Func_Access is access procedure
    (Window   : Window_not_null_Access;
     Geometry : SizeAlloc_Access) with Convention => C;

  type Capture_Window_Func_Access is access function
    (Window  : Window_not_null_Access;
     Surface : access SU.Surface_Access) return C.int with Convention => C;

  type Set_Border_Width_Func_Access is access function
    (Window : Window_not_null_Access;
     W      : C.unsigned) return C.int with Convention => C;

  type Set_Window_Caption_Func_Access is access function
    (Window  : Window_not_null_Access;
     Caption : CS.chars_ptr) return C.int with Convention => C;

  type Set_Transient_For_Func_Access is access procedure
    (Window, Parent : Window_not_null_Access) with Convention => C;

  type Set_Opacity_Func_Access is access function
    (Window  : Window_not_null_Access;
     Opacity : C.C_float) return C.int with Convention => C;

  type Tweak_Alignment_Func_Access is access procedure
    (Window       : Window_not_null_Access;
     Geometry     : SizeAlloc_Access;
     Max_W, Max_H : C.unsigned) with Convention => C;

  -------------------------------
  -- Multi-Window Driver Class --
  -------------------------------
  type Driver_MW_Class is limited record
    Super                : aliased Driver_Class;       -- [Driver -> DriverMW]
    Open_Window          : Open_Window_Func_Access;
    Close_Window         : Close_Window_Func_Access;
    Map_Window           : Map_Window_Func_Access;
    Unmap_Window         : Unmap_Window_Func_Access;
    Raise_Window         : Raise_Window_Func_Access;
    Lower_Window         : Lower_Window_Func_Access;
    Reparent_Window      : Reparent_Window_Func_Access;
    Get_Input_Focus      : Get_Input_Focus_Func_Access;
    Set_Input_Focus      : Set_Input_Focus_Func_Access;
    Move_Window          : Move_Window_Func_Access;
    Resize_Window        : Resize_Window_Func_Access;
    Move_Resize_Window   : Move_Resize_Window_Func_Access;
    Pre_Resize_Callback  : Pre_Resize_Callback_Func_Access;
    Post_Resize_Callback : Post_Resize_Callback_Func_Access;
    Capture_Window       : Capture_Window_Func_Access;
    Set_Border_Width     : Set_Border_Width_Func_Access;
    Set_Window_Caption   : Set_Window_Caption_Func_Access;
    Set_Transient_For    : Set_Transient_For_Func_Access;
    Set_Opacity          : Set_Opacity_Func_Access;
    Tweak_Alignment      : Tweak_Alignment_Func_Access;
  end record
    with Convention => C;

  type Driver_MW_Class_Access is access all Driver_MW_Class with Convention => C;
  subtype Driver_MW_Class_not_null_Access is not null Driver_MW_Class_Access;
 
  ----------------------------------
  -- Multi-Window Driver Instance --
  ----------------------------------
  type Driver_MW is limited record
    Super  : aliased Driver;                    -- [Driver -> DriverMW]
    Window : Window_Access;                     -- Back reference to window
    Flags  : C.unsigned;                        -- DRIVER_MW_* flags (below)
    C_Pad1 : Interfaces.Unsigned_32;
  end record
    with Convention => C;
  
  -- Flags --
  DRIVER_MW_OPEN : constant C.unsigned := 16#01#;  -- Enable rendering

  type Driver_MW_Access is access all Driver_MW with Convention => C;
  subtype Driver_MW_not_null_Access is not null Driver_MW_Access;
  
  -----------------------
  -- Single-Window Ops --
  -----------------------

  type Open_Video_Func_Access is access function
    (Driver         : Driver_not_null_Access;
     W,H            : C.unsigned;
     Bits_per_Pixel : C.int;
     Flags          : C.unsigned) return C.int with Convention => C;

  type Open_Video_Context_Func_Access is access function
    (Driver  : Driver_not_null_Access;
     Context : System.Address;
     Flags   : C.unsigned) return C.int with Convention => C;

  type Set_Video_Context_Func_Access is access function
    (Driver  : Driver_not_null_Access;
     Context : System.Address) return C.int with Convention => C;

  type Close_Video_Func_Access is access procedure
    (Driver : Driver_not_null_Access) with Convention => C;
  
  type Video_Resize_Func_Access is access function
    (Driver : Driver_not_null_Access;
     W,H    : C.unsigned) return C.int with Convention => C;

  type Video_Capture_Func_Access is access function
    (Driver : Driver_not_null_Access) return SU.Surface_Access with Convention => C;
  
  type Video_Clear_Func_Access is access procedure
    (Driver : Driver_not_null_Access;
     Color  : SU.Color_not_null_Access) with Convention => C;

  --------------------------------
  -- Single-Window Driver Class --
  --------------------------------
  type Driver_SW_Class is limited record
    Super              : aliased Driver_Class;    -- [Driver -> DriverSW]
#if HAVE_64BIT
    Flags              : Interfaces.Unsigned_64;  -- DRIVER_SW_* flags (below)
#else
    Flags              : C.unsigned;              -- DRIVER_SW_* flags (below)
#end if;
    Open_Video         : Open_Video_Func_Access;
    Open_Video_Context : Open_Video_Context_Func_Access;
    Set_Video_Context  : Set_Video_Context_Func_Access;
    Close_Video        : Close_Video_Func_Access;
    Video_Resize       : Video_Resize_Func_Access;
    Video_Capture      : Video_Capture_Func_Access;
    Video_Clear        : Video_Clear_Func_Access;
  end record
    with Convention => C;

  type Driver_SW_Class_Access is access all Driver_SW_Class with Convention => C;
  subtype Driver_SW_Class_not_null_Access is not null Driver_SW_Class_Access;
 
  -- Flags --
  DRIVER_SW_OVERLAY    : constant C.unsigned := 16#01#;  -- Overlay mode
  DRIVER_SW_BGPOPUP    : constant C.unsigned := 16#02#;  -- BG popup menu
  DRIVER_SW_FULLSCREEN : constant C.unsigned := 16#04#;  -- Fullscreen mode
  DRIVER_SW_REDRAW     : constant C.unsigned := 16#08#;  -- Redraw queued
  
  type Window_Operation_t is
    (NONE,
     MOVE,
     LEFT_RESIZE,
     RIGHT_RESIZE,
     HORIZ_RESIZE);
  for Window_Operation_t'Size use C.int'Size;

  -----------------------------------
  -- Single-Window Driver Instance --
  -----------------------------------
  type Driver_SW is limited record
    Super          : aliased Driver;              -- [Driver -> DriverSW]
    W,H            : C.unsigned;                  -- Resolution
    Bits_per_Pixel : C.unsigned;                  -- Depth
    Flags          : C.unsigned;                  -- Flags (none currently)

    Selected_Window      : Window_Access;         -- Window being manipulated
    Last_Key_Down_Window : Window_Access;         -- Window with last kbd event
    Modal_Window_Stack   : System.Address;        -- Modal window list TODO

    Window_Operation     : Window_Operation_t;    -- Window op in progress
    Icon_W, Icon_H       : C.int;                 -- Window icon sizes
    Nominal_FPS          : C.unsigned;            -- Nominal frames/second
    Current_FPS          : C.int;                 -- Last calculated FPS
    BG_Color             : SU.AG_Color;           -- Background color
    Refresh_Last         : C.unsigned;            -- Refresh rate timestamp
#if AG_MODEL = AG_MEDIUM
    C_Pad1               : Interfaces.Unsigned_32;
#end if;
    BG_Popup_Menu        : System.Address;        -- Background popup (TODO)
  end record
    with Convention => C;

  type Driver_SW_Access is access all Driver_SW with Convention => C;
  subtype Driver_SW_not_null_Access is not null Driver_SW_Access;

  ------------------------
  -- Driver Input Event --
  ------------------------
  type Driver_Event_Type is
    (UNKNOWN,
     MOUSE_MOTION,
     MOUSE_BUTTON_DOWN,
     MOUSE_BUTTON_UP,
     MOUSE_ENTER,
     MOUSE_LEAVE,
     FOCUS_IN,
     FOCUS_OUT,
     KEY_DOWN,
     KEY_UP,
     EXPOSE,
     VIDEO_RESIZE,
     CLOSE);
  for Driver_Event_Type'Size use C.int'Size;

  type Driver_Event_Entry is limited record
    Next : Driver_Event_Access;
    Prev : access Driver_Event_Access;
  end record
    with Convention => C;

  type Driver_Event
    (Which : Driver_Event_Type := Driver_Event_Type'First) is
  record
    Event_Type     : Driver_Event_Type;               -- Type of event
    C_Pad1         : Interfaces.Unsigned_32;
    Window         : Window_Access;                   -- For multi-window mode
    Entry_in_Queue : Driver_Event_Entry;              -- Entry in event queue

    case Which is
      when MOUSE_MOTION =>
        X,Y           : C.int;                        -- Mouse coordinates
      when MOUSE_BUTTON_DOWN | MOUSE_BUTTON_UP =>
        Button        : MSE.Mouse_Button;             -- Mouse button index
      when KEY_DOWN | KEY_UP =>
        Keysym        : Agar.Keyboard.Key_Sym;        -- Virtual keysym
        Unicode       : Interfaces.Unsigned_32;       -- Translated Unicode
      when VIDEO_RESIZE =>
        View_X,View_Y : C.int;                        -- New view coordinates
	View_W,View_H : C.int;                        -- New resolution
      when others =>
        null;
    end case;
  end record
    with Convention => C;
  pragma Unchecked_Union (Driver_Event);

  ---------------------
  -- Widget Instance --
  ---------------------
  type Widget_Private_t is array (1 .. $SIZEOF_AG_WidgetPvt) of
    aliased Unsigned_8 with Convention => C;
  for Widget_Private_t'Size use $SIZEOF_AG_WidgetPvt * System.Storage_Unit;

  type Widget_Surface_Flags_Access is access all Unsigned_8 with Convention => C;
  type Widget is limited record
    Super            : aliased OBJ.Object;       -- [Object -> Widget]
    Flags            : C.unsigned;               -- WIDGET_* Flags (below)
    X,Y              : C.int;                    -- Coordinates in parent widget
    W,H              : C.int;                    -- Allocated size in pixels

    Rect             : SU.AG_Rect;               -- Rectangle at 0,0 (cached)
    View_Rect        : SU.AG_Rect2;              -- Display coordinates (cached)
    Sensitivity_Rect : SU.AG_Rect2;              -- Cursor sensitivity rectangle

    Surface_Count    : C.unsigned;                  -- Mapped surface count
    Surfaces         : access SU.Surface_Access;    -- Mapped surfaces
    Surface_Flags    : Widget_Surface_Flags_Access; -- Mapped surface flags

    Textures         : Texture_Handle_Access;    -- Mapped texture handles
    Texcoords        : Texture_Coord_Access;     -- Mapped texture coords

    Forward_Focus_To : Widget_Access;            -- Forward focus to widget
    Parent_Window    : Window_Access;            -- Parent window (if any)
    Driver           : Driver_Access;            -- Parent driver instance
    Driver_Ops       : Driver_Class_Access;      -- Parent driver class

    Stylesheet       : System.Address;           -- TODO Alternate CSS stylesheet
    State            : Widget_State;             -- Style-effecting state
    Margin_Top       : Interfaces.Unsigned_8;    -- Margin top (px)
    Margin_Right     : Interfaces.Unsigned_8;    -- Margin right (px)
    Margin_Bottom    : Interfaces.Unsigned_8;    -- Margin bottom (px)
    Margin_Left      : Interfaces.Unsigned_8;    -- Margin left (px)

    Padding_Top      : C.unsigned;               -- Padding top (px)
    Padding_Right    : C.unsigned;               -- Padding right (px)
    Padding_Bottom   : C.unsigned;               -- Padding bottom (px)
    Padding_Left     : C.unsigned;               -- Padding left (px)

    Spacing_Top      : C.unsigned;               -- Spacing top (px)
    Spacing_Right    : C.unsigned;               -- Spacing right (px)
    Spacing_Bottom   : C.unsigned;               -- Spacing bottom (px)
    Spacing_Left     : C.unsigned;               -- Spacing left (px)

    Font             : System.Address;           -- Active font (TODO)
    Palette          : Widget_Palette;           -- Color palette

#if HAVE_OPENGL
    GL_Context       : Widget_GL_Context_Access; -- Context for USE_OPENGL
#end if;
    Actions_Data     : System.Address;           -- TODO
    Actions_Length   : C.int;                    -- TODO
    Actions_Capacity : C.int;                    -- TODO
    Private_Data     : Widget_Private_t;
  end record
    with Convention => C;
  
  -- Flags --
  WIDGET_FOCUSABLE            : constant C.unsigned := 16#00_0001#; -- Can grab focus
  WIDGET_FOCUSED              : constant C.unsigned := 16#00_0002#; -- Holds focus (read-only)
  WIDGET_UNFOCUSED_MOTION     : constant C.unsigned := 16#00_0004#; -- Receive mousemotion regardless of focus
  WIDGET_UNFOCUSED_BUTTONUP   : constant C.unsigned := 16#00_0008#; -- Receive buttonup regardless of focus
  WIDGET_UNFOCUSED_BUTTONDOWN : constant C.unsigned := 16#00_0010#; -- Receive buttondown regardless of focus
  WIDGET_VISIBLE              : constant C.unsigned := 16#00_0020#; -- Is visible (read-only)
  WIDGET_H_FILL               : constant C.unsigned := 16#00_0040#; -- Expand to fill width
  WIDGET_V_FILL               : constant C.unsigned := 16#00_0080#; -- Expand to fill height
  WIDGET_USE_OPENGL           : constant C.unsigned := 16#00_0100#; -- Set up separate GL context
  WIDGET_HIDE                 : constant C.unsigned := 16#00_0200#; -- Don't draw
  WIDGET_DISABLED             : constant C.unsigned := 16#00_0400#; -- Disabled state, ignore input
  WIDGET_MOUSEOVER            : constant C.unsigned := 16#00_0800#; -- Mouseover state (read-only)
  WIDGET_CATCH_TAB            : constant C.unsigned := 16#00_1000#; -- Receive events for focus-cycling key
  WIDGET_GL_RESHAPE           : constant C.unsigned := 16#00_2000#; -- Pending reshape
  WIDGET_UNDERSIZE            : constant C.unsigned := 16#00_4000#; -- Too small to draw
  WIDGET_NO_SPACING           : constant C.unsigned := 16#00_8000#; -- Ignore box model
  WIDGET_UNFOCUSED_KEYDOWN    : constant C.unsigned := 16#01_0000#; -- Receive keydowns regardless of focus
  WIDGET_UNFOCUSED_KEYUP      : constant C.unsigned := 16#02_0000#; -- Receive keyups regardless of focus
  WIDGET_UPDATE_WINDOW        : constant C.unsigned := 16#10_0000#; -- Request Window_Update ASAP
  WIDGET_QUEUE_SURFACE_BACKUP : constant C.unsigned := 16#20_0000#; -- Backup surfaces ASAP
  WIDGET_USE_TEXT             : constant C.unsigned := 16#40_0000#; -- Use font engine
  WIDGET_USE_MOUSEOVER        : constant C.unsigned := 16#80_0000#; -- Generate mouseover events
  WIDGET_EXPAND               : constant C.unsigned := WIDGET_H_FILL or
                                                       WIDGET_V_FILL;
  -- Surface flags --
  WIDGET_SURFACE_NODUP : constant C.unsigned := 16#01#; -- Don't free on cleanup
  WIDGET_SURFACE_REGEN : constant C.unsigned := 16#02#; -- Regen texture ASAP
 
  ------------------------------
  -- Boolean Flag Description --
  ------------------------------
  type Flag_Descr is limited record
#if HAVE_64BIT
    Bitmask   : Interfaces.Unsigned_64; -- Bitmask
#else
    Bitmask   : C.unsigned;             -- Bitmask
#end if;
    Text      : CS.chars_ptr;           -- Description (UTF-8)
    Writeable : C.int;                  -- User-editable
    C_Pad1    : Interfaces.Unsigned_32;
  end record
    with Convention => C;
  type Flag_Descr_Access is access all Flag_Descr with Convention => C;
  subtype Flag_Descr_not_null_Access is not null Flag_Descr_Access;
  
  --------------------
  -- Widget Actions --
  --------------------
  type Action_Type is
    (ACTION_FN,                 -- Call subroutine
     ACTION_SET_INT,            -- Set an integer 0 or 1
     ACTION_TOGGLE_INT,         -- Toggle an integer
     ACTION_SET_FLAG,           -- Set bit(s) 0 or 1
     ACTION_TOGGLE_FLAG);       -- Toggle bit(s)
  for Action_Type'Size use C.int'Size;

  type Action_Event_Type is
    (ACTION_BUTTON_DOWN,        -- Button pressed
     ACTION_BUTTON_UP,          -- Button released
     ACTION_KEY_DOWN,           -- Key pressed (once)
     ACTION_KEY_UP,             -- Key released
     ACTION_KEY_REPEAT);        -- Key pressed (with key repeat)
  for Action_Event_Type'Size use C.int'Size;

  type Action is limited record
    Act_Type     : Action_Type;             -- Type of action
    C_Pad1       : Interfaces.Unsigned_32;
    Widget       : Widget_Access;           -- Back pointer to widget
    Func         : EV.Event_Access;         -- Callback routine
    Set_Target   : System.Address;          -- Target for SET_{INT,FLAG}
    Set_Value    : C.int;                   -- Value for SET_INT
    Flag_Bitmask : C.unsigned;              -- Bitmask for {SET,TOGGLE}_FLAG
  end record
    with Convention => C;
  type Action_Access is access all Action with Convention => C;
  subtype Action_not_null_Access is not null Action_Access;

  type Action_Tie is array (1 .. $SIZEOF_AG_ActionTie)
    of aliased Unsigned_8 with Convention => C;
  for Action_Tie'Size use $SIZEOF_AG_ActionTie * System.Storage_Unit;
  type Action_Tie_Access is access all Action_Tie with Convention => C;
  subtype Action_Tie_not_null_Access is not null Action_Tie_Access;
  
  type Redraw_Tie is array (1 .. $SIZEOF_AG_RedrawTie)
    of aliased Unsigned_8 with Convention => C;
  for Redraw_Tie'Size use $SIZEOF_AG_RedrawTie * System.Storage_Unit;
  type Redraw_Tie_Access is access all Redraw_Tie with Convention => C;
  subtype Redraw_Tie_not_null_Access is not null Redraw_Tie_Access;
 
  ------------------------
  -- Cursor-Change Area --
  ------------------------
  type Cursor_Area is array (1 .. $SIZEOF_AG_CursorArea)
    of aliased Unsigned_8 with Convention => C;
  for Cursor_Area'Size use $SIZEOF_AG_CursorArea * System.Storage_Unit;
  type Cursor_Area_Access is access all Cursor_Area with Convention => C;
  subtype Cursor_Area_not_null_Access is not null Cursor_Area_Access;
  
  ------------
  -- Window --
  ------------
  CAPTION_MAX : constant Natural := $AG_WINDOW_CAPTION_MAX;

  type WM_Function is
    (WM_NORMAL,            -- Normal top-level window
     WM_DESKTOP,           -- Desktop feature (e.g., full-screen)
     WM_DOCK,              -- Dock or panel feature
     WM_TOOLBAR,           -- Toolbar torn off from main window
     WM_MENU,              -- Pinnable menu window
     WM_UTILITY,           -- Persistent utility window (palette, toolbox)
     WM_SPLASH,            -- Introductory screen
     WM_DIALOG,            -- Dialog window
     WM_DROPDOWN_MENU,     -- Menubar-triggered drop-down menu
     WM_POPUP_MENU,        -- Contextual popup menu
     WM_TOOLTIP,           -- Mouse hover triggered tooltip
     WM_NOTIFICATION,      -- Notification bubble
     WM_COMBO,             -- Combo-box triggered window
     WM_DND);              -- Draggable object

  for WM_Function'Size use C.int'Size;
 
  type Window_Alignment is
    (NO_ALIGNMENT,
     TOP_LEFT,      TOP_CENTER,      TOP_RIGHT,
     MIDDLE_LEFT,   MIDDLE_CENTER,   MIDDLE_RIGHT,
     BOTTOM_LEFT,   BOTTOM_CENTER,   BOTTOM_RIGHT,
     LAST_ALIGNMENT);

  for Window_Alignment'Size use C.int'Size;

  type Window_Caption is array (1 .. CAPTION_MAX) of
    aliased C.char with Convention => C;
  type Window_Private_t is array (1 .. $SIZEOF_AG_WindowPvt) of
    aliased Unsigned_8 with Convention => C;
  for Window_Private_t'Size use $SIZEOF_AG_WindowPvt * System.Storage_Unit;

  type Entry_in_User_t is limited record
    Next : Window_Access;
    Prev : access Window_Access;
  end record
    with Convention => C;

  type Window is limited record
    Super     : aliased Widget;           -- [Widget -> Window]

    Flags     : C.unsigned;               -- WINDOW_* flags (below)
    Caption   : Window_Caption;           -- Window title
    Visible   : C.int;                    -- Visibility flag
    Dirty     : C.int;                    -- Redraw flag
    Alignment        : Window_Alignment;  -- Initial position

    Title_Bar : System.Address;           -- TODO AG_Titlebar
    Icon      : System.Address;           -- TODO AG_Icon

    Spacing          : C.int;             -- Container spacing
    Padding_Top      : C.int;             -- Container padding (top)
    Padding_Bottom   : C.int;             -- Container padding (bottom)
    Padding_Left     : C.int;             -- Container padding (left)
    Padding_Right    : C.int;             -- Container padding (right)
    Req_W, Req_H     : C.int;             -- Requested window size
    Min_W, Min_H     : C.int;             -- Minimum window size
    Bottom_Border_W  : C.int;             -- Bottom border width (px)
    Side_Borders_W   : C.int;             -- Side borders width (px)
    Resize_Control_W : C.int;             -- Resize control width (px)

    Rect       : SU.AG_Rect;              -- Effective view rectangle
    Rect_Saved : SU.AG_Rect;              -- For post-{min,max}imize restore

    Min_Size_Pct         : C.int;         -- Size in % for MINSIZE_IS_PCT
    Focused_Widget_Count : C.int;         -- Number of focused widgets
    Excl_Motion_Widget   : Widget_Access; -- Process mousemotion exclusively
    Window_Function      : WM_Function;   -- High-level WM function
    Zoom_Pct             : C.int;         -- Effective zoom level in %

    Parent_Window        : Window_Access;   -- Parent window
    Transient_For_Window : Window_Access;   -- Is transient for that window
    Pinned_To_Window     : Window_Access;   -- Is pinned to that window

    Entry_in_User : Entry_in_User_t;        -- In optional user linked list

    Private_Data : Window_Private_t;
  end record
    with Convention => C;
  
  -- Flags --
  WINDOW_MODAL            : constant C.unsigned := 16#0000_0001#; -- App modal
  WINDOW_MAXIMIZED        : constant C.unsigned := 16#0000_0002#; -- Maximized
  WINDOW_MINIMIZED        : constant C.unsigned := 16#0000_0004#; -- Minimized
  WINDOW_KEEP_ABOVE       : constant C.unsigned := 16#0000_0008#; -- Keep above others
  WINDOW_KEEP_BELOW       : constant C.unsigned := 16#0000_0010#; -- Keep below others
  WINDOW_DENY_FOCUS       : constant C.unsigned := 16#0000_0020#; -- Reject focus
  WINDOW_NO_TITLE         : constant C.unsigned := 16#0000_0040#; -- No titlebar
  WINDOW_NO_BORDERS       : constant C.unsigned := 16#0000_0080#; -- No borders
  WINDOW_NO_H_RESIZE      : constant C.unsigned := 16#0000_0100#; -- No horiz resize
  WINDOW_NO_V_RESIZE      : constant C.unsigned := 16#0000_0200#; -- No vert resize
  WINDOW_NO_CLOSE         : constant C.unsigned := 16#0000_0400#; -- No close button
  WINDOW_NO_MINIMIZE      : constant C.unsigned := 16#0000_0800#; -- No minimize button
  WINDOW_NO_MAXIMIZE      : constant C.unsigned := 16#0000_1000#; -- No maximize button
  WINDOW_TILEABLE         : constant C.unsigned := 16#0000_2000#; -- WM can tile
  WINDOW_MINSIZE_IS_PCT   : constant C.unsigned := 16#0000_4000#; -- Min size is in %
  WINDOW_NO_BACKGROUND    : constant C.unsigned := 16#0000_8000#; -- No bg fill
  WINDOW_MAIN             : constant C.unsigned := 16#0001_0000#; -- Exit when closed
  WINDOW_FOCUS_ON_ATTACH  : constant C.unsigned := 16#0002_0000#; -- Focus after attach
  WINDOW_H_MAXIMIZE       : constant C.unsigned := 16#0004_0000#; -- Keep maximized horizontally
  WINDOW_V_MAXIMIZE       : constant C.unsigned := 16#0008_0000#; -- Keep maximized vertically
  WINDOW_NO_MOVE          : constant C.unsigned := 16#0010_0000#; -- Disable movement
  WINDOW_NO_CLIPPING      : constant C.unsigned := 16#0020_0000#; -- No clipping rectangle over window
  WINDOW_MODKEY_EVENTS    : constant C.unsigned := 16#0040_0000#; -- Modifier keys generate keyup/keydown
  WINDOW_DETACHING        : constant C.unsigned := 16#0080_0000#; -- Detach in progress (read-only)
  WINDOW_NO_CURSOR_CHANGE : constant C.unsigned := 16#0400_0000#; -- Disable cursor updates
  WINDOW_FADE_IN          : constant C.unsigned := 16#0800_0000#; -- Fade in (if supported)
  WINDOW_FADE_OUT         : constant C.unsigned := 16#1000_0000#; -- Fade out (if supported)
  WINDOW_NO_RESIZE        : constant C.unsigned := WINDOW_NO_H_RESIZE or
                                                   WINDOW_NO_V_RESIZE;
  WINDOW_NO_BUTTONS       : constant C.unsigned := WINDOW_NO_CLOSE or
                                                   WINDOW_NO_MINIMIZE or
						   WINDOW_NO_MAXIMIZE;
  WINDOW_PLAIN            : constant C.unsigned := WINDOW_NO_TITLE or
                                                   WINDOW_NO_BORDERS;

  --
  -- Return the first visible widget intersecting a point or enclosing a
  -- rectangle (in view coordinates). Scan all drivers and return first match.
  --
  --function Find_At_Point
  --  (Class : in String;
  --   X,Y   : in Natural) return Widget_Access;
  --function Find_Enclosing_Rect
  --  (Class : in String;
  --   X,Y   : in Natural;
  --   W,H   : in Positive) return Widget_Access;
 
  --
  -- Create / destroy a low-level driver instance.
  -- 
  function Open_Driver (Class : Driver_Class_not_null_Access) return Driver_Access
    with Import, Convention => C, Link_Name => "AG_DriverOpen";
  procedure Close_Driver (Driver : Driver_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_DriverClose";

  --
  -- Return the driver instance for the given numerical ID.
  --
  function Get_Driver (Driver_ID : C.unsigned) return Driver_Access
    with Import, Convention => C, Link_Name => "ag_get_driver_by_id";

  --
  -- Dump video memory to a jpeg file in ~/.<progname>/screenshot/.
  --
  procedure Capture_Screenshot
    with Import, Convention => C, Link_Name => "AG_ViewCapture";

  --
  -- Render a widget to the display.
  -- Context: Low-level rendering (i.e., AG_Driver(3) code)
  --
  procedure Draw (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetDraw";
 
  --
  -- Request a widget's preferred initial size.
  -- Context: Container widget's Size_Request or Size_Allocate operation.
  --
  procedure Size_Req
    (Widget : in Widget_not_null_Access;
     Size   : in SizeReq_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetSizeReq";
  
  --
  -- Allocate an effective widget size and position.
  -- Context: Container widget's Size_Request or Size_Allocate operation.
  --
  procedure Size_Alloc
    (Widget : in Widget_not_null_Access;
     Size   : in SizeAlloc_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetSizeAlloc";

  --
  -- Set whether to accept (or deny) focused state.
  --
  procedure Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean);
  function Set_Focusable
    (Widget : in Widget_not_null_Access;
     Enable : in Boolean) return Boolean;
 
  --
  -- Arrange for focus state to be forwarded automatically to the given
  -- Target widget (or null = disable focus forwarding).
  --
  procedure Forward_Focus
    (Widget : in Widget_not_null_Access;
     Target : in Widget_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetForwardFocus";

  --
  -- Focus on the widget (and implicitely its parents up to and including
  -- the parent window).
  --
  procedure Focus
    (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetFocus";
  function Focus
    (Widget : in Widget_not_null_Access) return Boolean;
 
  --
  -- Remove focus state from the widget (and its children implicitely).
  --
  procedure Unfocus
    (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetUnfocus";
 
  --
  -- Search for a focused widget under Root (which can also be a Window).
  --
  function Find_Focused_Widget
    (Root : in Widget_not_null_Access) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindFocused";
 
  --
  -- Return the topmost visible widget intersecting a display-coordinate point.
  --
  function Find_Widget_At_Point
    (Class : String;
     X,Y   : Natural) return Widget_Access;

  --
  -- Return topmost visible widget enclosing a display-coordinate rectangle.
  --
  function Find_Widget_Enclosing_Rect
    (Class         : String;
     X,Y           : Natural;
     Width, Height : Positive) return Widget_Access;

  --
  -- Update the effective view coordinates of a widget and its descendants.
  --
  procedure Update_Coordinates
    (Widget : in Widget_not_null_Access;
     X      : in Natural;
     Y      : in Natural);

  --
  -- Attach a Surface to a Widget such that:
  --
  --   1) It is freed automatically when the widget is destroyed.
  --   2) A hardware texture is generated automatically for it
  --      (where supported by the graphics backend).
  --
  -- Returned handle is unique to the Widget (and is index into its internal
  -- Surfaces, Surface_Flags, Textures and Texcoords arrays).
  -- 
  function Map_Surface
    (Widget  : in Widget_not_null_Access;
     Surface : in SU.Surface_not_null_Access) return Surface_Handle
    with Import, Convention => C, Link_Name => "AG_WidgetMapSurface";

  --
  -- Delete and replace any Surface corresponding to the given handle (as
  -- returned by Map_Surface). The previous surface (if any) is freed, and
  -- any associated hardware texture is regenerated.
  --
  -- Passing Surface => null is equivalent to calling Unmap_Surface.
  --
  procedure Replace_Surface
    (Widget  : in Widget_not_null_Access;
     Handle  : in Surface_Handle;
     Surface : in SU.Surface_Access := null)
    with Import, Convention => C, Link_Name => "AG_WidgetReplaceSurface";
 
  --
  -- Free any Surface mapped to the given handle (as returned by Map_Surface).
  -- Delete any hardware texture associated with the surface.
  --
  procedure Unmap_Surface
    (Widget : in Widget_not_null_Access;
     Handle : in Surface_Handle);

  --
  -- Blit the surface (or render the hardware texture) at Source:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  -- Source may be different from Widget (i.e., Widgets may render other
  -- widgets' surfaces) as long as both widgets are in the same Window.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Source   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0);

  --
  -- Blit the surface (or render the hardware texture) at Widget:[Handle],
  -- at target coordinates X,Y relative to Widget.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access := null;
     X,Y      : in Natural := 0);

  --
  -- Blit a Surface not managed by the Widget. This method is inefficient
  -- (no hardware acceleration) and should be avoided.
  --
  procedure Blit_Surface
    (Widget   : in Widget_not_null_Access;
     Surface  : in SU.Surface_not_null_Access;
     X,Y      : in Natural := 0);

#if HAVE_OPENGL
  --
  -- Coordinate-free variants of Blit_Surface for OpenGL-only widgets.
  -- Rely on GL transformations instead of coordinates.
  --
  procedure Blit_Surface_GL
    (Widget        : in Widget_not_null_Access;
     Handle        : in Surface_Handle;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitSurfaceGL";

  procedure Blit_Surface_GL
    (Widget        : in Widget_not_null_Access;
     Surface       : in SU.Surface_not_null_Access;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitGL";

  procedure Blit_Surface_GL_Flipped
    (Widget        : in Widget_not_null_Access;
     Surface       : in SU.Surface_not_null_Access;
     Width, Height : in C.C_float := 1.0)
    with Import, Convention => C, Link_Name => "AG_WidgetBlitSurfaceFlippedGL";

  --
  -- Destroy all GL resources associated with a widget and its children
  -- (but in a way that allows us to regenerate the GL context later).
  --
  procedure Free_GL_Resources (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetFreeResourcesGL";
  
  --
  -- Regenerate GL resources associated with a widget after loss of GL context.
  --
  procedure Regen_GL_Resources (Widget : in Widget_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_WidgetRegenResourcesGL";
#end if;

  --
  -- Test whether widget is sensitive to view coordinates X,Y.
  --
  function Is_Sensitive
    (Widget : in Widget_not_null_Access;
     X,Y    : in Natural) return Boolean;

  --
  -- Create a new mouse instance under a Driver.
  --
  function New_Mouse
    (Driver : in Driver_not_null_Access;
     Descr  : in String) return MSE.Mouse_Device_not_null_Access;

  --
  -- Change the cursor if its coordinates overlap a registered cursor area.
  -- Generally called from window/driver code following a mouse motion event.
  --
  procedure Mouse_Cursor_Update
    (Window : in Window_not_null_Access;
     X,Y    : in Natural);

  --
  -- Handle a mouse motion.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Motion
    (Window    : in Window_not_null_Access;
     X,Y       : in Natural;
     Xrel,Yrel : in Integer;
     Buttons   : in MSE.Mouse_Button);

  --
  -- Handle a mouse button press / release.
  -- Called from Driver code (agDrivers must be locked).
  --
  procedure Process_Mouse_Button_Up
    (Window : in Window_not_null_Access;
     X,Y    : in Natural;
     Button : in MSE.Mouse_Button);
  procedure Process_Mouse_Button_Down
    (Window : in Window_not_null_Access;
     X,Y    : in Natural;
     Button : in MSE.Mouse_Button);

  --
  -- Clear the internal cache of rendered glyphs.
  --
  procedure Clear_Glyph_Cache
    (Driver : in Driver_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_TextClearGlyphCache";
  
  private
  
  function AG_WidgetSetFocusable
    (Widget : in Widget_not_null_Access;
     Enable : in C.int) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetSetFocusable";

  function AG_WidgetFocus
    (Widget : in Widget_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetFocus";

  function AG_WidgetFindPoint
    (Class : in CS.chars_ptr;
     X,Y   : in C.int) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindPoint";
  
  function AG_WidgetFindRect
    (Class  : in CS.chars_ptr;
     X,Y    : in C.int;
     Width  : in C.int;
     Height : in C.int) return Widget_Access
    with Import, Convention => C, Link_Name => "AG_WidgetFindRect";
  
  procedure AG_WidgetUpdateCoords
    (Widget : in Widget_not_null_Access;
     X,Y    : in C.int)
    with Import, Convention => C, Link_Name => "AG_WidgetUpdateCoords";
 
  procedure AG_WidgetBlitFrom
    (Widget   : in Widget_not_null_Access;
     Source   : in Widget_not_null_Access;
     Handle   : in Surface_Handle;
     Src_Rect : in SU.Rect_Access;
     X,Y      : in C.int)
    with Import, Convention => C, Link_Name => "ag_widget_blit_from";
  
  procedure AG_WidgetBlit
    (Widget   : in Widget_not_null_Access;
     Surface  : in SU.Surface_not_null_Access;
     X,Y      : in C.int)
    with Import, Convention => C, Link_Name => "ag_widget_blit";

  function AG_WidgetSensitive
    (Widget : in Widget_not_null_Access;
     X,Y    : in C.int) return C.int
    with Import, Convention => C, Link_Name => "AG_WidgetSensitive";

  function AG_MouseNew
    (Driver : in Driver_not_null_Access;
     Descr  : in CS.chars_ptr) return MSE.Mouse_Device_not_null_Access
    with Import, Convention => C, Link_Name => "AG_MouseNew";

  procedure AG_MouseCursorUpdate
    (Window : Window_not_null_Access;
     X,Y    : C.int)
    with Import, Convention => C, Link_Name => "AG_MouseCursorUpdate";
  
  procedure AG_ProcessMouseMotion
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Xrel,Yrel : C.int;
     Buttons   : MSE.Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseMotion";

  procedure AG_ProcessMouseButtonUp
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Button    : MSE.Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseButtonUp";

  procedure AG_ProcessMouseButtonDown
    (Window    : Window_not_null_Access;
     X,Y       : C.int;
     Button    : MSE.Mouse_Button)
    with Import, Convention => C, Link_Name => "AG_ProcessMouseButtonDown";

end Agar.Widget;
