------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                         A G A R  . S U R F A C E                         --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces; use Interfaces;
with Interfaces.C;
with Interfaces.C.Strings;
with Agar.Data_Source;
with System;

package Agar.Surface is
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;
  package DS renames Agar.Data_Source;

  use type C.int;
  use type C.unsigned;
  
  ------------------
  -- Surface Mode --                   -----------------------------------------
  ------------------                   -- Bits per pixel | 1 2 4 8 16 24 32 64 |
  type Surface_Mode is                 -----------------------------------------
    (PACKED,                           -- Packed RGB[a]  |     X   X  X  X  X  |
     INDEXED,                          -- Palletized     | X X X X             |
     GRAYSCALE);                       -- Grayscale      |         X     X  X  |
                                       -----------------------------------------
  for Surface_Mode'Size use C.int'Size;
  
  -------------------------------
  -- Grayscale Conversion Mode --
  -------------------------------
  type Grayscale_Mode is
    (BT709_GRAY,                           -- ITU-R Recommendation BT.709 --
     RMY_GRAY,                             -- R-Y algorithm --
     Y_GRAY);                              -- Y-Grayscale (YIQ / NTSC) --
  for Grayscale_Mode'Size use C.int'Size;

  ------------
  -- Colors --
  ------------
#if AG_MODEL = AG_LARGE
  subtype Pixel     is Interfaces.Unsigned_64;
  subtype Component is Interfaces.Unsigned_16;
  type    Component_Offset is range -(2 **15) .. +(2 **15 - 1) with Convention => C;
  for     Component_Offset'Size use 16;
  type    Gray_Component   is range -(2 **31) .. +(2 **31 - 1) with Convention => C;
  for     Gray_Component'Size use 32;
#elsif AG_MODEL = AG_MEDIUM
  subtype Pixel     is Interfaces.Unsigned_32;
  subtype Component is Interfaces.Unsigned_8;
  type    Component_Offset is range -127 .. 127 with Convention => C;
  for     Component_Offset'Size use 8;
  type    Gray_Component   is range -(2 **15) .. +(2 **15 - 1) with Convention => C;
  for     Gray_Component'Size use 16;
#elsif AG_MODEL = AG_SMALL
  subtype Pixel     is Interfaces.Unsigned_16;
  subtype Component is Interfaces.Unsigned_8;
  type    Component_Offset is range -127 .. 127 with Convention => C;
  for     Component_Offset'Size use 8;
  type    Gray_Component   is range -127 .. +127 with Convention => C;
  for     Gray_Component'Size use 8;
#end if;
  COMPONENT_BITS : constant C.unsigned := $AG_COMPONENT_BITS;
  COLOR_FIRST    : constant Component := $AG_COLOR_FIRST;
  COLOR_LAST     : constant Component := $AG_COLOR_LAST;
  TRANSPARENT    : constant Component := $AG_TRANSPARENT;
  OPAQUE         : constant Component := $AG_OPAQUE;

  --------------------------
  -- Native Color in RGBA --
  --------------------------
  type Color is record
    R,G,B,A : Component;
  end record
    with Convention => C;

  type Color_Access is access all Color with Convention => C;
  subtype Color_not_null_Access is not null Color_Access;

  --------------------------
  -- Native Color in HSVA --
  --------------------------
  type Color_HSV is record
#if HAVE_FLOAT
    H,S,V,A : C.c_float;
#else
    H,S,V,A : Interfaces.Unsigned_32;
#end if;
  end record
    with Convention => C;
  
  type Intensity is digits 6 range 0.0 .. 1.0;

  type Color_HSV_Access is access all Color_HSV with Convention => C;
  subtype Color_HSV_not_null_Access is not null Color_HSV_Access;
  
  -------------------------
  -- Native Color Offset --
  -------------------------
  type Color_Offset is record
    R,G,B,A : Component_Offset;
  end record
    with Convention => C;

  type Color_Offset_Access is access all Color_Offset with Convention => C;
  subtype Color_Offset_not_null_Access is not null Color_Offset_Access;

  -------------------------
  -- Rectangle of Pixels --
  -------------------------
  type Rect is record
    X,Y : C.int;                 -- Upper-left corner
    W,H : C.int;                 -- Width, height
  end record
    with Convention => C;

  type Rect_Access is access all Rect with Convention => C;
  subtype Rect_not_null_Access is not null Rect_Access;
  
  ----------------------------------------
  -- Rectangle of Pixels with Endpoints --
  ----------------------------------------
  type Rect2 is record
    X1,Y1 : C.int;               -- Upper-left corner
    W,H   : C.int;               -- Width, height
    X2,Y2 : C.int;               -- Lower-right corner
  end record
    with Convention => C;

  type Rect2_Access is access all Rect2 with Convention => C;
  subtype Rect2_not_null_Access is not null Rect2_Access;
  
  --------------------------------------
  -- Clipping Rectangle (GL-optional) --
  --------------------------------------
  type Clip_Equations is array (1 .. 16) of aliased C.double;
  type Clip_Rect is record
    R    : Rect;
#if HAVE_FLOAT
    Eqns : Clip_Equations;
#end if;
  end record
    with Convention => C;

  type Clip_Rect_Access is access all Clip_Rect with Convention => C;
  subtype Clip_Rect_not_null_Access is not null Clip_Rect_Access;
  
  --------------------------------------
  -- Texture Coordinate (GL-optional) --
  --------------------------------------
  type Texture_Coordinate is record
#if HAVE_FLOAT
    X,Y,W,H : C.C_float;
#else
    X,Y,W,H : C.int;
#end if;  
  end record
    with Convention => C;

  ----------------------------------
  -- Palette for Indexed Surfaces --
  ----------------------------------
  type Palette is limited record
    Colors : Color_Access;
    Count  : C.unsigned;
  end record
    with Convention => C;

  type Palette_Access is access all Palette with Convention => C;
  subtype Palette_not_null_Access is not null Palette_Access;
  
  ------------------------------
  -- Pixel Format Description --
  ------------------------------
  type Pixel_Format
    (Which_Mode : Surface_Mode := PACKED)
  is limited record
    Mode            : Surface_Mode;                    -- Image type
    Bits_per_Pixel  : C.int;                           -- Depth (bits/pixel)
    Bytes_per_Pixel : C.int;                           -- Depth (bytes/pixel)
    Pixels_per_Byte : C.int;                           -- Pixels per byte

    case Which_Mode is
    when INDEXED =>
      Palette : Palette_Access;                         -- Color map
    when GRAYSCALE =>
      Gray_Mode : Grayscale_Mode;                       -- Grayscale method
    when PACKED =>
      R_Loss,  G_Loss,  B_Loss,  A_Loss  : Unsigned_8;  -- Bits lost by packing
      R_Shift, G_Shift, B_Shift, A_Shift : Unsigned_8;  -- Bits at right of each
      R_Mask,  G_Mask,  B_Mask,  A_Mask  : Pixel;       -- Component masks
    end case;
  end record
    with Convention => C;
  pragma Unchecked_Union (Pixel_Format);

  type Pixel_Format_Access is access all Pixel_Format with Convention => C;
  subtype Pixel_Format_not_null_Access is not null Pixel_Format_Access;
 
  type Pixel_Access is access all Unsigned_8 with Convention => C;
  subtype Pixel_not_null_Access is not null Pixel_Access;
 
  ----------------------
  -- Surface Instance --
  ----------------------
  type Surface is limited record
    Format         : aliased Pixel_Format; -- Pixel format description
    Flags          : C.unsigned;           -- Surface Flags (below)
    W,H            : C.unsigned;           -- Size in pixels
    Pitch, Padding : C.unsigned;           -- Scanline byte length, end padding
    Pixels         : Pixel_Access;         -- Raw pixel data
    Clip_Rect      : Rect;                 -- Destination clipping rectangle
    Colorkey       : Pixel;                -- Color key pixel
    Alpha          : Component;            -- Per-surface alpha
    Frames         : System.Address;       -- TODO animation frames
    Frame_Count    : C.unsigned;           -- Animation frame count
  end record
    with Convention => C;
  
  -- Surface Flags --
  SURFACE_COLORKEY   : constant C.unsigned := 16#01#; -- Enable source colorkey
  SURFACE_ALPHA      : constant C.unsigned := 16#02#; -- Enable per-surface alpha
  SURFACE_GL_TEXTURE : constant C.unsigned := 16#04#; -- Is GL-texture-ready
  SURFACE_MAPPED     : constant C.unsigned := 16#08#; -- Is mapped to a widget
  SURFACE_STATIC     : constant C.unsigned := 16#10#; -- Never free()
  SURFACE_EXT_PIXELS : constant C.unsigned := 16#20#; -- Is allocated externally
  SURFACE_ANIMATED   : constant C.unsigned := 16#40#; -- Is an animation
  SURFACE_TRACE      : constant C.unsigned := 16#80#; -- Enable debugging

  type Surface_Access is access all Surface with Convention => C;
  subtype Surface_not_null_Access is not null Surface_Access;

  -----------------------------
  -- Alpha Blending Function --
  -----------------------------
  type Alpha_Func is
    (ALPHA_OVERLAY,         -- a => MIN(source.a + target.a, 1)
     ALPHA_ZERO,            -- a => 0
     ALPHA_ONE,             -- a => 255
     ALPHA_SRC,             -- a => source.a
     ALPHA_DST,             -- a => target.a
     ALPHA_ONE_MINUS_DST,   -- a => (1 - target.a)
     ALPHA_ONE_MINUS_SRC);  -- a => (1 - source.a)

  for Alpha_Func'Size use C.unsigned'Size;

  --------------------------
  -- Image Export Methods --
  --------------------------
  type JPEG_Quality is new Natural range 0 .. 100;
  type JPEG_Method is
    (JDCT_ISLOW,      -- Slow, accurate integer DCT
     JDCT_IFAST,      -- Fast, less accurate integer DCT
     JDCT_FLOAT);     -- Floating-point method

  --
  -- Initialize a new RGB packed-pixel format.
  --
  procedure Pixel_Format_RGB
    (Format         : in Pixel_Format_not_null_Access;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in Pixel := 16#000000000000ffff#;
     G_Mask         : in Pixel := 16#00000000ffff0000#;
     B_Mask         : in Pixel := 16#0000ffff00000000#);
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in Pixel := 16#000000ff#;
     G_Mask         : in Pixel := 16#0000ff00#;
     B_Mask         : in Pixel := 16#00ff0000#);
#end if;
  
  --
  -- Initialize a new RGBA packed-pixel format.
  --
  procedure Pixel_Format_RGBA
    (Format         : in Pixel_Format_not_null_Access;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in Pixel := 16#000000000000ffff#;
     G_Mask         : in Pixel := 16#00000000ffff0000#;
     B_Mask         : in Pixel := 16#0000ffff00000000#;
     A_Mask         : in Pixel := 16#ffff000000000000#);
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in Unsigned_32 := 16#000000ff#;
     G_Mask         : in Unsigned_32 := 16#0000ff00#;
     B_Mask         : in Unsigned_32 := 16#00ff0000#;
     A_Mask         : in Unsigned_32 := 16#ff000000#);
#end if;

  --
  -- Initialize a new palettized pixel format.
  --
  procedure Pixel_Format_Indexed
    (Format         : in Pixel_Format_not_null_Access;
     Bits_per_Pixel : in Positive := 8);
  
  --
  -- Initialize a new grayscale pixel format.
  --
  procedure Pixel_Format_Grayscale
    (Format         : in Pixel_Format_not_null_Access;
     Bits_per_Pixel : in Positive := 32);

  --
  -- Return a newly-allocated copy of a pixel format.
  --
  function Duplicate_Pixel_Format
    (Format : in Pixel_Format_not_null_Access) return Pixel_Format_Access
    with Import, Convention => C, Link_Name => "AG_PixelFormatDup";

  --
  -- Release all resources allocated by a pixel format.
  --
  procedure Free_Pixel_Format
    (Format : in Pixel_Format_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_PixelFormatFree";
  
  --
  -- Compare the contents of two pixel formats (including any Palettes).
  --
  --function "=" (Left, Right : Pixel_Format_Access) return Boolean;

  --
  -- Create a new surface in PACKED, INDEXED or GRAYSCALE pixel format.
  --
  -- With PACKED mode, masks can be specified optionally in the Format
  -- argument (if Format is null then the default RGBA masks are used).
  --
  -- Src_Colorkey enables colorkey transparency and Src_Alpha enables
  -- overall per-surface alpha in blits where Surface is the source.
  -- GL_Texture advises that the surface is a valid OpenGL texture.
  --
  function New_Surface
    (Mode           : in Surface_Mode := PACKED;
     W,H            : in Natural := 0;
     Bits_per_Pixel : in Positive := 32;
     Format         : in Pixel_Format_Access := null;
     Src_Colorkey   : in Boolean := false;
     Src_Alpha      : in Boolean := false;
     GL_Texture     : in Boolean := false) return Surface_Access;

  --
  -- Create a new surface in PACKED pixel format with RGBA masks.
  --
  function New_Surface
    (W,H            : in Natural := 0;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in Pixel := 16#000000000000ffff#;
     G_Mask         : in Pixel := 16#00000000ffff0000#;
     B_Mask         : in Pixel := 16#0000ffff00000000#;
     A_Mask         : in Pixel := 16#ffff000000000000#;
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in Pixel := 16#000000ff#;
     G_Mask         : in Pixel := 16#0000ff00#;
     B_Mask         : in Pixel := 16#00ff0000#;
     A_Mask         : in Pixel := 16#ff000000#;
#end if;
     Src_Colorkey   : in Boolean := false;
     Src_Alpha      : in Boolean := false;
     GL_Texture     : in Boolean := false) return Surface_Access;
  
  --
  -- Create a new PACKED surface (with given RGBA masks),
  -- and initialize its contents from existing pixel data.
  --
  function New_Surface
    (Pixels         : in Pixel_not_null_Access;
     W,H            : in Natural;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in Pixel := 16#000000000000ffff#;
     G_Mask         : in Pixel := 16#00000000ffff0000#;
     B_Mask         : in Pixel := 16#0000ffff00000000#;
     A_Mask         : in Pixel := 16#ffff000000000000#;
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in Pixel := 16#000000ff#;
     G_Mask         : in Pixel := 16#0000ff00#;
     B_Mask         : in Pixel := 16#00ff0000#;
     A_Mask         : in Pixel := 16#ff000000#;
#end if;
     Src_Colorkey   : in Boolean := false;
     Src_Alpha      : in Boolean := false;
     GL_Texture     : in Boolean := false) return Surface_Access;
  
  --
  -- Create a new surface by loading a BMP, PNG or JPEG image file.
  --
  function New_Surface
    (File         : in String;
     Src_Colorkey : in Boolean := false;
     Src_Alpha    : in Boolean := false;
     GL_Texture   : in Boolean := false) return Surface_Access;

  --
  -- Create a surface in the "best" format suitable for OpenGL textures.
  --
  function New_Surface_GL (W,H : in Natural) return Surface_Access;

  --
  -- Set a color palette entry of an Indexed surface
  --
  procedure Set_Color
    (Surface : in Surface_not_null_Access;
     Index   : in Natural;
     Color   : in Color_not_null_Access);
  procedure Set_Color
    (Surface : in Surface_not_null_Access;
     Index   : in Natural;
     R,G,B   : in Component;
     A       : in Component := OPAQUE);
  procedure Set_Color
    (Surface : in Surface_not_null_Access;
     Index   : in Natural;
     H,S,V   : in Intensity;
     A       : in Intensity := 1.0);

  -- TODO: Set_Color from array of Color.

  --
  -- Return a newly allocated copy of a surface.
  --
  function Duplicate_Surface
    (Surface : in Surface_not_null_Access) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceDup";
 
  --
  -- Convert a surface to a different pixel format as best as we can.
  --
  function Convert_Surface
    (Surface    : in Surface_not_null_Access;
     New_Format : in Pixel_Format_not_null_Access) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceConvert";
 
  --
  -- Copy pixel data from one surface to another. If formats differ, convert.
  -- If dimensions differ, clip to size. Ignore Target's colorkey & alpha.
  --
  procedure Copy_Surface
    (Target, Source : in Surface_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_SurfaceCopy";

  --
  -- Copy a Source surface (or a region of its pixels) to a Target surface
  -- which can be of a different format. Handle format conversion, clipping
  -- and blending according to the pixel formats of the surfaces (and their
  -- colorkey/alpha settings).
  --
  procedure Blit_Surface
    (Source   : in Surface_not_null_Access;
     Src_Rect : in Rect_access := null;
     Target   : in Surface_not_null_Access;
     Dst_X    : in Natural := 0;
     Dst_Y    : in Natural := 0);
 
  --
  -- Change the dimensions of a surface without scaling its contents. Pixel
  -- data is reallocated (if growing the surface, then then new pixels are
  -- left uninitialized).
  --
  function Resize_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural) return Boolean;
  procedure Resize_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural);

  --
  -- Deallocate a surface. Warning: The deallocation is unchecked. Never
  -- use on surfaces which have been attached to a widget using Map_Surface
  -- (widget-mapped surfaces are freed implicitely by Agar.Widget).
  -- 
  procedure Free_Surface
    (Surface : in Surface_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_SurfaceFree";
  
  --
  -- Export the surface to a BMP, PNG or JPEG image file.
  --
  function Export_Surface
    (Surface : in Surface_not_null_Access;
     File    : in String) return Boolean;
  function Export_BMP
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.bmp") return Boolean;
  function Export_PNG
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.png";
     Adam7   : in Boolean := false) return Boolean;
  function Export_JPEG
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.jpg";
     Quality : in JPEG_Quality := 100;
     Method  : in JPEG_Method := JDCT_ISLOW) return Boolean;
  
  -- TODO check for sdlada (SDL 2.x)
  --function Export_SDL
  --  (Surface : in Surface_not_null_Access) return SDL.Video.Surfaces.Surface;

  --
  -- Read compressed image data from a data source. If the data contains
  -- a complete image, return a newly-allocated surface for it.
  --
  function Read_From_BMP
    (Source : in DS.Data_Source_not_null_Access) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_ReadSurfaceFromBMP";
  function Read_From_PNG
    (Source : in DS.Data_Source_not_null_Access) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_ReadSurfaceFromPNG";
  function Read_From_JPEG
    (Source : in DS.Data_Source_not_null_Access) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_ReadSurfaceFromJPEG";

  --
  -- Return a string describing a blending function.
  --
  function Alpha_Func_Name
    (Func : Alpha_Func) return String;

  --
  -- Blend a target pixel against a specified color. The target pixel's
  -- alpha component is computed according to Func.
  --
  procedure Blend_Pixel
    (Surface : in Surface_not_null_Access;
     Pixel   : in Pixel_not_null_Access;
     Color   : in Color_not_null_Access;
     Func    : in Alpha_Func := ALPHA_OVERLAY);
  procedure Blend_Pixel
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Color    : in Color_not_null_Access;
     Func     : in Alpha_Func := ALPHA_OVERLAY);
 
  --
  -- Return the native-width packed pixel corresponding to a Color.
  --
  function Map_Pixel
    (Format : in Pixel_Format_not_null_Access;
     Color  : in Color_not_null_Access) return Pixel
#if AG_MODEL = AG_LARGE
    with Import, Convention => C, Link_Name => "ag_map_pixel64";
#else
    with Import, Convention => C, Link_Name => "ag_map_pixel32";
#end if;

  --
  -- Return the native-width packed pixel corresponding to RGBA components.
  --
  function Map_Pixel
    (Format : in Pixel_Format_not_null_Access;
     R,G,B  : in Component;
     A      : in Component := OPAQUE) return Pixel;

  --
  -- Return the 32-bit packed pixel corresponding to a given color.
  --
  function Map_Pixel_32
    (Format : in Pixel_Format_not_null_Access;
     Color  : in Color_not_null_Access) return Unsigned_32
    with Import, Convention => C, Link_Name => "ag_map_pixel32";
  function Map_Pixel_32
    (Format : in Pixel_Format_not_null_Access;
     R,G,B  : in Unsigned_8) return Unsigned_32
    with Import, Convention => C, Link_Name => "AG_MapPixel32_RGB8";
  function Map_Pixel_32
    (Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in Unsigned_8) return Unsigned_32
    with Import, Convention => C, Link_Name => "AG_MapPixel32_RGBA8";
  function Map_Pixel_32
    (Format  : in Pixel_Format_not_null_Access;
     R,G,B   : in Unsigned_16) return Unsigned_32
    with Import, Convention => C, Link_Name => "AG_MapPixel32_RGB16";
  function Map_Pixel_32
    (Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in Unsigned_16) return Unsigned_32
    with Import, Convention => C, Link_Name => "AG_MapPixel32_RGBA16";

#if AG_MODEL = AG_LARGE
  --
  -- Return the 64-bit packed pixel corresponding to a given color.
  --
  function Map_Pixel_64
    (Format : in Pixel_Format_not_null_Access;
     Color  : in Color_not_null_Access) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_map_pixel64";
  function Map_Pixel_64
    (Format : in Pixel_Format_not_null_Access;
     R,G,B  : in Unsigned_16) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_map_pixel64_rgb16";
  function Map_Pixel_64
    (Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in Unsigned_16) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_map_pixel64_rgba16";
#end if;

  procedure Unpack_Pixel
    (Value   : in     Pixel;
     Format  : in     Pixel_Format_not_null_Access;
     R,G,B,A :    out Component);

  --
  -- Return a new surface generated by scaling an Input surface to specified
  -- dimensions. Function form may fail and return null. Procedure form raises
  -- a fatal exception on failure.
  --
  function Scale_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural) return Surface_Access;
  procedure Scale_Surface
    (Original : in     Surface_not_null_Access;
     W,H      : in     Natural;
     Scaled   :    out Surface_not_null_Access);
 
  --
  -- Fill a rectangle of pixels with a specified color.
  --
  procedure Fill_Rect
    (Surface : in Surface_not_null_Access;
     Rect    : in Rect_Access := null;
     Color   : in Color_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_FillRect";
  procedure Fill_Rect
    (Surface : in Surface_not_null_Access;
     Rect    : in Rect_Access := null;
     R,G,B   : in Component;
     A       : in Component := OPAQUE);

  --
  -- Extract a native-width packed pixel from a surface.
  --
  function Get_Pixel
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return Pixel;
  function Get_Pixel
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access) return Pixel
#if AG_MODEL = AG_LARGE
    with Import, Convention => C, Link_Name => "ag_surface_get64_at";
#else
    with Import, Convention => C, Link_Name => "ag_surface_get32_at";
#end if;

  --
  -- Extract a 32-bit packed pixel from a surface.
  --
  function Get_Pixel_32
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return Unsigned_32;
  function Get_Pixel_32
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access) return Unsigned_32
    with Import, Convention => C, Link_Name => "ag_surface_get32";

#if AG_MODEL = AG_LARGE
  --
  -- Extract a 64-bit packed pixel from a surface.
  --
  function Get_Pixel_64
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return Unsigned_64;
  function Get_Pixel_64
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_surface_get64";
#end if;
  
  --
  -- Set a native-width packed pixel in a surface.
  --
  procedure Put_Pixel
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Value    : in Pixel;
     Clipping : in Boolean := true);
  procedure Put_Pixel
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access;
     Value   : in Pixel)
#if AG_MODEL = AG_LARGE
    with Import, Convention => C, Link_Name => "ag_surface_put64_at";
#else
    with Import, Convention => C, Link_Name => "ag_surface_put32_at";
#end if;


  --
  -- Set a 32-bit packed pixel in a surface.
  --
  procedure Put_Pixel_32
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access;
     Value   : in Unsigned_32)
    with Import, Convention => C, Link_Name => "ag_surface_put32_at";
  procedure Put_Pixel_32
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Value    : in Unsigned_32;
     Clipping : in Boolean := true);
 
#if AG_MODEL = AG_LARGE 
  --
  -- Set a 64-bit packed pixel in a surface.
  --
  procedure Put_Pixel_64
    (Surface : in Surface_not_null_Access;
     Address : in Pixel_not_null_Access;
     Value   : in Unsigned_64)
    with Import, Convention => C, Link_Name => "ag_surface_put64_at";
  procedure Put_Pixel_64
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Value    : in Unsigned_64;
     Clipping : in Boolean := true);
#end if;

  --
  -- Set source alpha flag and per-surface alpha value.
  --
  procedure Set_Alpha
    (Surface : in Surface_not_null_Access;
     Enable  : in Boolean := false;
     Alpha   : in Component := OPAQUE);
  
  --
  -- Set source colorkey flag and surface colorkey value.
  --
  procedure Set_Colorkey
    (Surface  : in Surface_not_null_Access;
     Enable   : in Boolean := false;
     Colorkey : in Pixel := 0);

  --
  -- Clipping rectangle.
  -- 
  procedure Get_Clipping_Rect
    (Surface : in     Surface_not_null_Access;
     X,Y,W,H :    out Natural);

  procedure Set_Clipping_Rect
    (Surface : in Surface_not_null_Access;
     X,Y,W,H : in Natural);
  
  private
  
  procedure AG_PixelFormatRGB
    (Format               : in Pixel_Format_not_null_Access;
     Bits_per_Pixel       : in C.int;
     R_Mask,G_Mask,B_Mask : in Pixel)
    with Import, Convention => C, Link_Name => "AG_PixelFormatRGB";

  procedure AG_PixelFormatRGBA
    (Format                      : in Pixel_Format_not_null_Access;
     Bits_per_Pixel              : in C.int;
     R_Mask,G_Mask,B_Mask,A_Mask : in Pixel)
    with Import, Convention => C, Link_Name => "AG_PixelFormatRGBA";

  procedure AG_PixelFormatIndexed
    (Format         : in Pixel_Format_not_null_Access;
     Bits_per_Pixel : in C.int)
    with Import, Convention => C, Link_Name => "AG_PixelFormatIndexed";
  
  procedure AG_PixelFormatGrayscale
    (Format         : in Pixel_Format_not_null_Access;
     Bits_per_Pixel : in C.int)
    with Import, Convention => C, Link_Name => "AG_PixelFormatGrayscale";
  
  function AG_PixelFormatCompare
    (Left, Right : in Pixel_Format_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "ag_pixel_format_compare";

  function AG_SurfaceNew
    (Format : in Pixel_Format_not_null_Access;
     W,H    : in C.unsigned;
     Flags  : in C.unsigned) return Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceNew";

  function AG_SurfaceRGB
    (W,H            : in C.unsigned;
     Bits_per_Pixel : in C.unsigned;
     Flags          : in C.unsigned;
     R_Mask         : in Pixel;
     G_Mask         : in Pixel;
     B_Mask         : in Pixel) return Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceRGB";

  function AG_SurfaceRGBA
    (W,H            : in C.unsigned;
     Bits_per_Pixel : in C.unsigned;
     Flags          : in C.unsigned;
     R_Mask         : in Pixel;
     G_Mask         : in Pixel;
     B_Mask         : in Pixel;
     A_Mask         : in Pixel) return Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceRGBA";
  
  function AG_SurfaceIndexed
    (W,H            : in C.unsigned;
     Bits_per_Pixel : in C.unsigned;
     Flags          : in C.unsigned) return Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceIndexed";
  
  function AG_SurfaceGrayscale
    (W,H            : in C.unsigned;
     Bits_per_Pixel : in C.unsigned;
     Flags          : in C.unsigned) return Surface_not_null_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceGrayscale";
  
  function AG_SurfaceFromPixelsRGB
    (Pixels         : in Pixel_not_null_Access;
     W,H            : in C.unsigned;
     Bits_per_Pixel : in C.int;
     R_Mask         : in Pixel;
     G_Mask         : in Pixel;
     B_Mask         : in Pixel) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceFromPixelsRGB";
  
  function AG_SurfaceFromPixelsRGBA
    (Pixels         : in Pixel_not_null_Access;
     W,H            : in C.unsigned;
     Bits_per_Pixel : in C.int;
     R_Mask         : in Pixel;
     G_Mask         : in Pixel;
     B_Mask         : in Pixel;
     A_Mask         : in Pixel) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceFromPixelsRGBA";

  function AG_SurfaceStdGL
    (W,H : in C.unsigned) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceStdGL";

  function AG_SurfaceFromFile
    (File : in CS.chars_ptr) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceFromFile";
  
  function AG_SurfaceExportFile
    (Surface : in Surface_not_null_Access;
     File    : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceExportFile";
 
   -- TODO AG_SurfaceSetAddress()
  
  procedure AG_SurfaceSetAddress
    (Surface : in Surface_not_null_Access;
     Pixels  : in System.Address)
    with Import, Convention => C, Link_Name => "AG_SurfaceSetAddress";
  
  procedure AG_SurfaceSetColors
    (Surface : in Surface_not_null_Access;
     Color   : in Color_not_null_Access;
     Offset  : in C.unsigned;
     Count   : in C.unsigned)
    with Import, Convention => C, Link_Name => "AG_SurfaceSetColors";

  function AG_SurfaceSetPalette
    (Surface : in Surface_not_null_Access;
     Palette : in Palette_not_null_Access) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceSetPalette";
   
  -- TODO AG_SurfaceCopyPixels()
  -- TODO AG_SurfaceSetPixels()

  procedure AG_SurfaceBlit
    (Source   : in Surface_not_null_Access;
     Src_Rect : in Rect_Access;
     Target   : in Surface_not_null_Access;
     X, Y     : in C.int)
    with Import, Convention => C, Link_Name => "AG_SurfaceBlit";

  function AG_SurfaceResize
    (Surface : in Surface_not_null_Access;
     W,H     : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceResize";

  procedure AG_SurfaceSetAlpha
    (Surface : in Surface_not_null_Access;
     Flags   : in C.unsigned;
     Alpha   : in Component)
    with Import, Convention => C, Link_Name => "ag_surface_set_alpha";
  
  procedure AG_SurfaceSetColorkey
    (Surface  : in Surface_not_null_Access;
     Flags    : in C.unsigned;
     Colorkey : in Pixel)
    with Import, Convention => C, Link_Name => "ag_surface_set_colorkey";
  
  procedure AG_HSV2Color
    (H,S,V : in C.c_float;
     Color : in Color_not_null_Access)
    with Import, Convention => C, Link_Name => "ag_hsv_2_color";
  
  function AG_SurfaceExportBMP
    (Surface : in Surface_not_null_Access;
     File    : in CS.chars_ptr) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceExportBMP";

  EXPORT_PNG_ADAM7 : constant C.unsigned := 16#01#;

  function AG_SurfaceExportPNG
    (Surface : in Surface_not_null_Access;
     File    : in CS.chars_ptr;
     Flags   : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceExportPNG";
  
  EXPORT_JPEG_JDCT_ISLOW  : constant C.unsigned := 16#01#;
  EXPORT_JPEG_JDCT_IFAST  : constant C.unsigned := 16#02#;
  EXPORT_JPEG_JDCT_FLOAT  : constant C.unsigned := 16#04#;

  function AG_SurfaceExportJPEG
    (Surface : in Surface_not_null_Access;
     File    : in CS.chars_ptr;
     Quality : in C.unsigned;
     Flags   : in C.unsigned) return C.int
    with Import, Convention => C, Link_Name => "AG_SurfaceExportJPEG";
  
  function AG_SurfaceScale
    (Surface : in Surface_not_null_Access;
     W,H     : in C.unsigned;
     Flags   : in C.unsigned) return Surface_Access
    with Import, Convention => C, Link_Name => "AG_SurfaceScale";
  
  procedure AG_SurfaceBlend
    (Surface : in Surface_not_null_Access;
     X,Y     : in C.int;
     Color   : in Color_not_null_Access;
     Func    : in C.unsigned)
    with Import, Convention => C, Link_Name => "ag_surface_blend";

  procedure AG_SurfaceBlend_At
    (Surface : in Surface_not_null_Access;
     Pixel   : in Pixel_not_null_Access;
     Color   : in Color_not_null_Access;
     Func    : in C.unsigned)
    with Import, Convention => C, Link_Name => "ag_surface_blend_at";

  procedure AG_FillRect
    (Surface : in Surface_not_null_Access;
     Rect    : in Rect_Access := null;
     Color   : in Color_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_FillRect";

  function AG_SurfaceGet32
    (Surface : in Surface_not_null_Access;
     X,Y     : in C.int) return Unsigned_32
    with Import, Convention => C, Link_Name => "ag_surface_get32";

  procedure AG_SurfacePut32
    (Surface : in Surface_not_null_Access;
     X,Y     : in C.int;
     Value   : in Unsigned_32)
    with Import, Convention => C, Link_Name => "ag_surface_put32";

  procedure AG_GetColor32
    (Color  : in Color_not_null_Access;
     Value  : in Unsigned_32;
     Format : in Pixel_Format_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_GetColor32";

  procedure AG_GetColor32_RGBA8
    (Value   : in Unsigned_32;
     Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetColor32_RGBA8";

  procedure AG_GetColor32_RGBA16
    (Value   : in Unsigned_32;
     Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in System.Address)
    with Import, Convention => C, Link_Name => "AG_GetColor32_RGBA16";
  
  function AG_MapPixel32
    (Format  : in Pixel_Format_not_null_Access;
     Color   : in Color_not_null_Access) return Unsigned_32
    with Import, Convention => C, Link_Name => "ag_map_pixel32";

#if AG_MODEL = AG_LARGE
  function AG_SurfaceGet64
    (Surface : in Surface_not_null_Access;
     X,Y     : in C.int) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_surface_get64";

  procedure AG_SurfacePut64
    (Surface : in Surface_not_null_Access;
     X,Y     : in C.int;
     Value   : in Unsigned_64)
    with Import, Convention => C, Link_Name => "ag_surface_put64";

  procedure AG_GetColor64
    (Color  : in Color_not_null_Access;
     Value  : in Unsigned_64;
     Format : in Pixel_Format_not_null_Access)
    with Import, Convention => C, Link_Name => "AG_GetColor64";

  procedure AG_GetColor64_RGBA8
    (Value   : in Unsigned_64;
     Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in System.Address)
    with Import, Convention => C, Link_Name => "ag_get_color64_rgba8";

  procedure AG_GetColor64_RGBA16
    (Value   : in Unsigned_64;
     Format  : in Pixel_Format_not_null_Access;
     R,G,B,A : in System.Address)
    with Import, Convention => C, Link_Name => "ag_get_color64_rgba16";

  function AG_MapPixel64
    (Format  : in Pixel_Format_not_null_Access;
     Color   : in Color_not_null_Access) return Unsigned_64
    with Import, Convention => C, Link_Name => "ag_map_pixel64";

#end if;

end Agar.Surface;
