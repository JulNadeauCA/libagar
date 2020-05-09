------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                         A G A R  . S U R F A C E                         --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018-2019 Julien Nadeau Carriere (vedge@csoft.net)         --
--                                                                          --
-- Permission to use, copy, modify, and/or distribute this software for any --
-- purpose with or without fee is hereby granted, provided that the above   --
-- copyright notice and this permission notice appear in all copies.        --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES --
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         --
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  --
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   --
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    --
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  --
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           --
------------------------------------------------------------------------------
with Agar.Error;
with Ada.Text_IO;

package body Agar.Surface is
  
  procedure Pixel_Format_RGB
    (Format         : in Pixel_Format_not_null_Access;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in AG_Pixel := 16#000000000000ffff#;
     G_Mask         : in AG_Pixel := 16#00000000ffff0000#;
     B_Mask         : in AG_Pixel := 16#0000ffff00000000#) is
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in AG_Pixel := 16#000000ff#;
     G_Mask         : in AG_Pixel := 16#0000ff00#;
     B_Mask         : in AG_Pixel := 16#00ff0000#) is
#end if;
  begin
    AG_PixelFormatRGB
      (Format         => Format,
       Bits_per_Pixel => C.int(Bits_per_Pixel),
       R_Mask         => R_Mask,
       G_Mask         => G_Mask,
       B_Mask         => B_Mask);
  end;
  
  procedure Pixel_Format_RGBA
    (Format         : Pixel_Format_not_null_Access;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in AG_Pixel := 16#000000000000ffff#;
     G_Mask         : in AG_Pixel := 16#00000000ffff0000#;
     B_Mask         : in AG_Pixel := 16#0000ffff00000000#;
     A_Mask         : in AG_Pixel := 16#ffff000000000000#) is
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in AG_Pixel := 16#000000ff#;
     G_Mask         : in AG_Pixel := 16#0000ff00#;
     B_Mask         : in AG_Pixel := 16#00ff0000#;
     A_Mask         : in AG_Pixel := 16#ff000000#) is
#end if;
  begin
    AG_PixelFormatRGBA
      (Format         => Format,
       Bits_per_Pixel => C.int(Bits_per_Pixel),
       R_Mask         => R_Mask,
       G_Mask         => G_Mask,
       B_Mask         => B_Mask,
       A_Mask         => A_Mask);
  end;
  
  procedure Pixel_Format_Indexed
    (Format         : Pixel_Format_not_null_Access;
     Bits_per_Pixel : Positive := 8) is
  begin
    AG_PixelFormatIndexed
      (Format         => Format,
       Bits_per_Pixel => C.int(Bits_per_Pixel));
  end;
  
  procedure Pixel_Format_Grayscale
    (Format         : Pixel_Format_not_null_Access;
     Bits_per_Pixel : Positive := 32) is
  begin
    AG_PixelFormatGrayscale
      (Format         => Format,
       Bits_per_Pixel => C.int(Bits_per_Pixel));
  end;
  
  --
  -- Compare the contents of two Pixel formats (if both are indexed,
  -- then compare their palettes as well).
  --
  --function "=" (Left, Right : Pixel_Format_Access) return Boolean is
  --begin
  --  if Left = null and Right = null then
  --    return True;
  --  end if;
  --  if Left = null or Right = null then
  --    return False;
  --  end if;
  --  return 0 = AG_PixelFormatCompare (Left, Right);
  --end;
  
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
     GL_Texture     : in Boolean := false) return Surface_Access
  is
    Flags : C.unsigned := 0;
  begin
    if Src_Colorkey then Flags := Flags or SURFACE_COLORKEY;   end if;
    if Src_Alpha    then Flags := Flags or SURFACE_ALPHA;      end if;
    if GL_Texture   then Flags := Flags or SURFACE_GL_TEXTURE; end if;

    if Format = null then
      Ada.Text_IO.Put_Line("Format is null, auto-selecting");
      case (Mode) is
        when PACKED =>
#if AG_MODEL = AG_LARGE
          case (Bits_per_Pixel) is
            when 64 =>
              Ada.Text_IO.Put_Line("64-bit RGBA");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000000000000ffff#,
                 G_Mask         => 16#00000000ffff0000#,
                 B_Mask         => 16#0000ffff00000000#,
                 A_Mask         => 16#ffff000000000000#);
            when 48 =>
              Ada.Text_IO.Put_Line("48-bit RGB");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000000000000ffff#,
                 G_Mask         => 16#00000000ffff0000#,
                 B_Mask         => 16#0000ffff00000000#,
                 A_Mask         => 0);
            when others =>
              null;
          end case;
#end if;
          case (Bits_per_Pixel) is
            when 32 =>
              Ada.Text_IO.Put_Line("32-bit RGBA");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000000ff#,
                 G_Mask         => 16#0000ff00#,
                 B_Mask         => 16#00ff0000#,
                 A_Mask         => 16#ff000000#);
            when 24 =>
              Ada.Text_IO.Put_Line("24-bit RGB");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000000ff#,
                 G_Mask         => 16#0000ff00#,
                 B_Mask         => 16#00ff0000#,
                 A_Mask         => 0);
            when 16 =>
              Ada.Text_IO.Put_Line("16-bit RGBA");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000f#,
                 G_Mask         => 16#00f0#,
                 B_Mask         => 16#0f00#,
                 A_Mask         => 16#f000#);
            when 12 =>
              Ada.Text_IO.Put_Line("12-bit RGB");
              return AG_SurfaceRGBA
                (W              => C.unsigned(W),
                 H              => C.unsigned(H),
                 Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
                 Flags          => Flags,
                 R_Mask         => 16#000f#,
                 G_Mask         => 16#00f0#,
                 B_Mask         => 16#0f00#,
                 A_Mask         => 0);
            when others =>
              null;
          end case;
        when INDEXED =>
          Ada.Text_IO.Put_Line(Integer'Image(Bits_per_Pixel) & "-bit Indexed");
          return AG_SurfaceIndexed
            (W              => C.unsigned(W),
             H              => C.unsigned(H),
             Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
             Flags          => Flags);
        when GRAYSCALE =>
          Ada.Text_IO.Put_Line(Integer'Image(Bits_per_Pixel) & "-bit Grayscale");
          return AG_SurfaceGrayscale
            (W              => C.unsigned(W),
             H              => C.unsigned(H),
             Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
             Flags          => Flags);
      end case;  -- Mode
    end if;      -- Format = null

    return AG_SurfaceNew
      (Format => Format,
       W      => C.unsigned(W),
       H      => C.unsigned(H),
       Flags  => Flags);
  end;
  
  --
  -- Create a new surface in PACKED pixel format with RGBA masks.
  --
  function New_Surface
    (W,H            : in Natural := 0;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in AG_Pixel := 16#000000000000ffff#;
     G_Mask         : in AG_Pixel := 16#00000000ffff0000#;
     B_Mask         : in AG_Pixel := 16#0000ffff00000000#;
     A_Mask         : in AG_Pixel := 16#ffff000000000000#;
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in AG_Pixel := 16#000000ff#;
     G_Mask         : in AG_Pixel := 16#0000ff00#;
     B_Mask         : in AG_Pixel := 16#00ff0000#;
     A_Mask         : in AG_Pixel := 16#ff000000#;
#end if;
     Src_Colorkey   : in Boolean := false;
     Src_Alpha      : in Boolean := false;
     GL_Texture     : in Boolean := false) return Surface_Access
  is
    Flags : C.unsigned := 0;
  begin
    if Src_Colorkey then Flags := Flags or SURFACE_COLORKEY; end if;
    if Src_Alpha    then Flags := Flags or SURFACE_ALPHA;    end if;
    if GL_Texture   then Flags := Flags or SURFACE_GL_TEXTURE;   end if;

    if A_Mask /= 0 then
      return AG_SurfaceRGBA
        (W              => C.unsigned(W),
         H              => C.unsigned(H),
         Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
         Flags          => Flags,
         R_Mask         => R_Mask,
         G_Mask         => G_Mask,
         B_Mask         => B_Mask,
         A_Mask         => A_Mask);
    else
      return AG_SurfaceRGB
        (W              => C.unsigned(W),
         H              => C.unsigned(H),
         Bits_per_Pixel => C.unsigned(Bits_per_Pixel),
         Flags          => Flags,
         R_Mask         => R_Mask,
         G_Mask         => G_Mask,
         B_Mask         => B_Mask);
    end if;
  end New_Surface;
  
  --
  -- Create a new PACKED surface (with given RGBA masks),
  -- and initialize its contents from existing pixel data.
  --
  function New_Surface
    (Pixels         : in Pixel_not_null_Access;
     W,H            : in Natural;
#if AG_MODEL = AG_LARGE
     Bits_per_Pixel : in Positive := 64;
     R_Mask         : in AG_Pixel := 16#000000000000ffff#;
     G_Mask         : in AG_Pixel := 16#00000000ffff0000#;
     B_Mask         : in AG_Pixel := 16#0000ffff00000000#;
     A_Mask         : in AG_Pixel := 16#ffff000000000000#;
#else
     Bits_per_Pixel : in Positive := 32;
     R_Mask         : in AG_Pixel := 16#000000ff#;
     G_Mask         : in AG_Pixel := 16#0000ff00#;
     B_Mask         : in AG_Pixel := 16#00ff0000#;
     A_Mask         : in AG_Pixel := 16#ff000000#;
#end if;
     Src_Colorkey   : in Boolean := false;
     Src_Alpha      : in Boolean := false;
     GL_Texture     : in Boolean := false) return Surface_Access
  is
    S : Surface_Access;
  begin
    if A_Mask /= 0 then
      S := AG_SurfaceFromPixelsRGBA
        (Pixels         => Pixels,
         W              => C.unsigned(W),
         H              => C.unsigned(H),
         Bits_per_Pixel => C.int(Bits_per_Pixel),
         R_Mask         => R_Mask,
         G_Mask         => G_Mask,
         B_Mask         => B_Mask,
         A_Mask         => A_Mask);
    else
      S := AG_SurfaceFromPixelsRGB
        (Pixels         => Pixels,
         W              => C.unsigned(W),
         H              => C.unsigned(H),
         Bits_per_Pixel => C.int(Bits_per_Pixel),
         R_Mask         => R_Mask,
         G_Mask         => G_Mask,
         B_Mask         => B_Mask);
    end if;
    if S = null then
      return null;
    end if;
    if Src_Colorkey then S.Flags := S.Flags or SURFACE_COLORKEY;   end if;
    if Src_Alpha    then S.Flags := S.Flags or SURFACE_ALPHA;      end if;
    if GL_Texture   then S.Flags := S.Flags or SURFACE_GL_TEXTURE; end if;
    return S;
  end New_Surface;

  --
  -- Create a new surface by loading a BMP, PNG or JPEG image file.
  --
  function New_Surface
    (File         : in String;
     Src_Colorkey : in Boolean := false;
     Src_Alpha    : in Boolean := false;
     GL_Texture   : in Boolean := false) return Surface_Access
  is
    Ch_File : aliased C.char_array := C.To_C(File);
    S : Surface_Access;
  begin
    S := AG_SurfaceFromFile
      (File => CS.To_Chars_Ptr(Ch_File'Unchecked_Access));
    if S = null then
      return null;
    end if;
    if Src_Colorkey then S.Flags := S.Flags or SURFACE_COLORKEY; end if;
    if Src_Alpha    then S.Flags := S.Flags or SURFACE_ALPHA;    end if;
    if GL_Texture   then S.Flags := S.Flags or SURFACE_GL_TEXTURE;   end if;
    return S;
  end;
  
  --
  -- Return a Color from 8-bit RGBA components.
  --
  function Color_8
    (R,G,B : in Unsigned_8;
     A     : in Unsigned_8 := 255) return AG_Color
  is
#if AG_MODEL = AG_LARGE
    Color : constant AG_Color := (Unsigned_16(Float(R) / 255.0 * 65535.0),
                                  Unsigned_16(Float(G) / 255.0 * 65535.0),
                                  Unsigned_16(Float(B) / 255.0 * 65535.0),
                                  Unsigned_16(Float(A) / 255.0 * 65535.0));
#else
    Color : constant AG_Color := (R,G,B,A);
#end if;
  begin
    return Color;
  end;

  --
  -- Return an AG_Color from 16-bit RGBA components.
  --
  function Color_16
    (R,G,B : in Unsigned_16;
     A     : in Unsigned_16 := 65535) return AG_Color
  is
#if AG_MODEL = AG_LARGE
    Color : constant AG_Color := (R,G,B,A);
#else
    Color : constant AG_Color := (Unsigned_8(Float(R) / 65535.0 * 255.0),
                                  Unsigned_8(Float(G) / 65535.0 * 255.0),
                                  Unsigned_8(Float(B) / 65535.0 * 255.0),
                                  Unsigned_8(Float(A) / 65535.0 * 255.0));
#end if;
  begin
    return Color;
  end;

  --
  -- Return a Color from Hue, Saturation, Value and Alpha components.
  --
  function Color_HSV
    (H,S,V : in Intensity;
     A     : in Intensity := 1.0) return AG_Color
  is
    Color : aliased AG_Color;
  begin
    AG_HSV2Color
      (H     => C.c_float(H),
       S     => C.c_float(S),
       V     => C.c_float(V),
       Color => Color'Unchecked_Access);
#if AG_MODEL = AG_LARGE
    Color.A := AG_Component(A * 65535.0);
#else
    Color.A := AG_Component(A * 255.0);
#end if;
    return Color;
  end;

  --
  -- Return a native component offset amount for a given 8-bit component.
  --
  function Component_Offset_8
    (X : in Unsigned_8) return AG_Component is
  begin
#if AG_MODEL = AG_LARGE
    return AG_Component(Float(X) / 255.0 * 65535.0);
#else
    return AG_Component(X);
#end if;
  end;

  --
  -- Return a native component offset amount for a given 16-bit component.
  --
  function Component_Offset_16
    (X : in Unsigned_16) return AG_Component is
  begin
#if AG_MODEL = AG_LARGE
    return AG_Component(X);
#else
    return AG_Component(Float(X) / 65535.0 * 255.0);
#end if;
  end;

  --
  -- Set a color palette entry of an Indexed surface (AG_Color argument)
  --
  procedure Set_Color
    (Surface : in Surface_not_null_Access;
     Index   : in Natural;
     Color   : in AG_Color)
  is
    C_Color : aliased AG_Color := Color;
  begin
    AG_SurfaceSetColors
      (Surface => Surface,
       Color   => C_Color'Unchecked_Access,
       Offset  => C.unsigned(Index),
       Count   => 1);
  end;
  
  --
  -- Set a color palette entry of an Indexed surface (AG_Color access argument)
  --
  procedure Set_Color
    (Surface : in Surface_not_null_Access;
     Index   : in Natural;
     Color   : in Color_not_null_Access) is
  begin
    AG_SurfaceSetColors
      (Surface => Surface,
       Color   => Color,
       Offset  => C.unsigned(Index),
       Count   => 1);
  end;
  
  --
  -- Copy a Source surface (or a region of its pixels) to a Target surface
  -- which can be of a different format. Handle format conversion, clipping
  -- and blending according to the pixel formats of the surfaces (and their
  -- Colorkey and Src_Alpha settings).
  --
  procedure Blit_Surface
    (Source   : in Surface_not_null_Access;
     Src_Rect : in Rect_access := null;
     Target   : in Surface_not_null_Access;
     Dst_X    : in Natural := 0;
     Dst_Y    : in Natural := 0) is
  begin
    AG_SurfaceBlit
      (Source   => Source,
       Src_Rect => Src_Rect,
       Target   => Target,
       X        => C.int(Dst_X),
       Y        => C.int(Dst_Y));
  end;

  --
  -- Change the dimensions of a surface without scaling its contents. Pixel
  -- data is reallocated (if growing the surface, then then new pixels are
  -- left uninitialized).
  --
  function Resize_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural) return Boolean is
  begin
    return 0 = AG_SurfaceResize
      (Surface => Surface,
       W       => C.unsigned(W),
       H       => C.unsigned(H));
  end;
  procedure Resize_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural)
  is
    Result : C.int;
  begin
    Result := AG_SurfaceResize
      (Surface => Surface,
       W       => C.unsigned(W),
       H       => C.unsigned(H));
    if Result /= 0 then
      raise Program_Error with Agar.Error.Get_Error;
    end if;
  end;
  
  --
  -- Export the surface to a BMP, PNG or JPEG image file.
  --
  function Export_Surface
    (Surface : in Surface_not_null_Access;
     File    : in String) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return 0 = AG_SurfaceExportFile
      (Surface => Surface,
       File    => CS.To_Chars_Ptr(Ch_File'Unchecked_Access));
  end;
 
  --
  -- Export the surface to a Windows bitmap image file.
  --
  function Export_BMP
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.bmp") return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
  begin
    return 0 = AG_SurfaceExportBMP
      (Surface => Surface,
       File    => CS.To_Chars_Ptr(Ch_File'Unchecked_Access));
  end;
  
  --
  -- Export the surface to a PNG image file.
  --
  function Export_PNG
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.png";
     Adam7   : in Boolean := false) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
    Flags   : C.unsigned := 0;
  begin
    if Adam7 then
      Flags := EXPORT_PNG_ADAM7;
    end if;
    return 0 = AG_SurfaceExportPNG
      (Surface => Surface,
       File    => CS.To_Chars_Ptr(Ch_File'Unchecked_Access),
       Flags   => Flags);
  end;

  --
  -- Export the surface to a JPEG image file.
  --
  function Export_JPEG
    (Surface : in Surface_not_null_Access;
     File    : in String := "output.jpg";
     Quality : in JPEG_Quality := 100;
     Method  : in JPEG_Method := JDCT_ISLOW) return Boolean
  is
    Ch_File : aliased C.char_array := C.To_C(File);
    Flags   : C.unsigned := 0;
  begin
    case (Method) is
      when JDCT_ISLOW => Flags := EXPORT_JPEG_JDCT_ISLOW;
      when JDCT_IFAST => Flags := EXPORT_JPEG_JDCT_IFAST;
      when JDCT_FLOAT => Flags := EXPORT_JPEG_JDCT_FLOAT;
    end case;
    return 0 = AG_SurfaceExportJPEG
      (Surface => Surface,
       File    => CS.To_Chars_Ptr(Ch_File'Unchecked_Access),
       Quality => C.unsigned(Quality),
       Flags   => Flags);
  end;
  
  --
  -- Return a string describing a blending function.
  --
  function Alpha_Func_Name
    (Func : Alpha_Func) return String is
  begin
    case Func is
    when ALPHA_OVERLAY       => return "src+dst";
    when ALPHA_ZERO          => return "zero";
    when ALPHA_ONE           => return "one";
    when ALPHA_SRC           => return "src";
    when ALPHA_DST           => return "dst";
    when ALPHA_ONE_MINUS_DST => return "1-dst";
    when ALPHA_ONE_MINUS_SRC => return "1-src";
    end case;
  end;
  
  --
  -- Blend a target pixel against a specified color. The target pixel's
  -- alpha component is computed according to Func (by pixel address).
  --
  procedure Blend_Pixel
    (Surface : in Surface_not_null_Access;
     Pixel   : in Pixel_not_null_Access;
     Color   : in Color_not_null_Access;
     Func    : in Alpha_Func := ALPHA_OVERLAY) is
  begin
    AG_SurfaceBlend_At
      (Surface => Surface,
       Pixel   => Pixel,
       Color   => Color,
       Func    => Alpha_Func'Pos(Func));
  end;

  --
  -- Blend a target pixel against a specified color. The target pixel's
  -- alpha component is computed according to Func (by X,Y coordinates).
  --
  procedure Blend_Pixel
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Color    : in Color_not_null_Access;
     Func     : in Alpha_Func := ALPHA_OVERLAY) is
  begin
    AG_SurfaceBlend
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y),
       Color   => Color,
       Func    => Alpha_Func'Pos(Func));
  end;
  
  --
  -- Return the native-width packed pixel corresponding to an AG_Color
  -- (under the pixel format of Surface).
  --
  function Map_Pixel
    (Surface : in Surface_not_null_Access;
     Color   : in AG_Color) return AG_Pixel
  is
    C_Color : aliased AG_Color := Color;
  begin
#if AG_MODEL = AG_LARGE
    return AG_MapPixel64
      (Format => Surface.Format'Access,
       Color  => C_Color'Unchecked_Access);
#else
    return AG_MapPixel32
      (Format => Surface.Format'Access,
       Color  => C_Color'Unchecked_Access);
#end if;
  end;

  --
  -- Return the native-width packed pixel corresponding to an AG_Color
  -- (under the specified pixel format).
  --
  function Map_Pixel
    (Format : in Pixel_Format_not_null_Access;
     Color  : in AG_Color) return AG_Pixel
  is
    C_Color : aliased AG_Color := Color;
  begin
#if AG_MODEL = AG_LARGE
    return AG_MapPixel64
      (Format => Format,
       Color  => C_Color'Unchecked_Access);
#else
    return AG_MapPixel32
      (Format => Format,
       Color  => C_Color'Unchecked_Access);
#end if;
  end;
  
  --
  -- Return the native-width packed pixel corresponding to an AG_Color
  -- (under the pixel format of Surface).
  --
  function Map_Pixel
    (Surface : in Surface_not_null_Access;
     Color   : in Color_not_null_access) return AG_Pixel is
  begin
#if AG_MODEL = AG_LARGE
    return AG_MapPixel64
      (Format => Surface.Format'Access,
       Color  => Color);
#else
    return AG_MapPixel32
      (Format => Surface.Format'Access,
       Color  => Color);
#end if;
  end;
  
  
  --
  -- Return a new surface generated by scaling an Input surface to specified
  -- dimensions. Function form may fail and return null.
  --
  function Scale_Surface
    (Surface : in Surface_not_null_Access;
     W,H     : in Natural) return Surface_Access is
  begin
    return AG_SurfaceScale
      (Surface => Surface,
       W       => C.unsigned(W),
       H       => C.unsigned(H),
       Flags   => C.unsigned(0));
  end;
  
  --
  -- Return a new surface generated by scaling an Input surface to specified
  -- dimensions. Procedure form raises fatal exception on failure.
  --
  procedure Scale_Surface
    (Original : in     Surface_not_null_Access;
     W,H      : in     Natural;
     Scaled   :    out Surface_not_null_Access)
  is
     Result : Surface_Access; 
  begin
    Result := AG_SurfaceScale
      (Surface => Original,
       W       => C.unsigned(W),
       H       => C.unsigned(H),
       Flags   => C.unsigned(0));
    if Result = null then
      raise Program_Error with Agar.Error.Get_Error;
    end if;
    Scaled := Result;
  end;

  --
  -- Fill a rectangle of pixels with a specified color (AG_Color argument).
  --
  procedure Fill_Rect
    (Surface : in Surface_not_null_Access;
     Rect    : in Rect_Access := null;
     Color   : in AG_Color)
  is
    C_Color : aliased AG_Color := Color;
  begin
    AG_FillRect
      (Surface => Surface,
       Rect    => Rect,
       Color   => C_Color'Unchecked_Access);
  end;
  
  --
  -- Extract a native-width packed pixel from a surface (by coordinates).
  --
  function Get_Pixel
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return AG_Pixel is
  begin
#if AG_MODEL = AG_LARGE
    return AG_SurfaceGet64
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y));
#else
    return AG_SurfaceGet32
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y));
#end if;
  end;

  --
  -- Extract a 32-bit packed pixel from a surface (by coordinates).
  --
  function Get_Pixel_32
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return Unsigned_32 is
  begin
    return AG_SurfaceGet32
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y));
  end;

#if AG_MODEL = AG_LARGE
  --
  -- Extract a 64-bit packed pixel from a surface (by coordinates).
  --
  function Get_Pixel_64
    (Surface : in Surface_not_null_Access;
     X,Y     : in Natural) return Unsigned_64 is
  begin
    return AG_SurfaceGet64
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y));
  end;
#end if;

  --
  -- Set a native-width packed pixel in a surface (by coordinates).
  --
  procedure Put_Pixel
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Pixel    : in AG_Pixel;
     Clipping : in Boolean := true) is
  begin
    if Clipping then
      if C.int(X) <  Surface.Clip_Rect.X or
         C.int(Y) <  Surface.Clip_Rect.Y or
         C.int(X) >= Surface.Clip_Rect.X+Surface.Clip_Rect.W or
         C.int(Y) >= Surface.Clip_Rect.Y+Surface.Clip_Rect.H then
        return;
      end if;
    end if;
#if AG_MODEL = AG_LARGE
    AG_SurfacePut64
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y),
       Pixel   => Pixel);
#else
    AG_SurfacePut32
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y),
       Pixel   => Pixel);
#end if;
  end;
  
  --
  -- Set a 32-bit packed pixel in a surface (by coordinates).
  --
  procedure Put_Pixel_32
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Pixel    : in Unsigned_32;
     Clipping : in Boolean := true) is
  begin
    if Clipping then
      if C.int(X) <  Surface.Clip_Rect.X or
         C.int(Y) <  Surface.Clip_Rect.Y or
         C.int(X) >= Surface.Clip_Rect.X+Surface.Clip_Rect.W or
         C.int(Y) >= Surface.Clip_Rect.Y+Surface.Clip_Rect.H then
        return;
      end if;
    end if;
    AG_SurfacePut32
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y),
       Pixel   => Pixel);
  end;

#if AG_MODEL = AG_LARGE
  --
  -- Set a 64-bit packed pixel in a surface (by coordinates).
  --
  procedure Put_Pixel_64
    (Surface  : in Surface_not_null_Access;
     X,Y      : in Natural;
     Pixel    : in Unsigned_64;
     Clipping : in Boolean := true) is
  begin
    if Clipping then
      if C.int(X) <  Surface.Clip_Rect.X or
         C.int(Y) <  Surface.Clip_Rect.Y or
         C.int(X) >= Surface.Clip_Rect.X+Surface.Clip_Rect.W or
         C.int(Y) >= Surface.Clip_Rect.Y+Surface.Clip_Rect.H then
        return;
      end if;
    end if;
    AG_SurfacePut64
      (Surface => Surface,
       X       => C.int(X),
       Y       => C.int(Y),
       Pixel   => Pixel);
  end;
#end if;
 
  procedure Unpack_Pixel
    (Pixel   : in     AG_Pixel;
     Format  : in     Pixel_Format_not_null_Access;
     R,G,B,A :    out AG_Component)
  is
    Color : aliased AG_Color;
  begin
#if AG_MODEL = AG_LARGE
    AG_GetColor64
      (Color  => Color'Unchecked_Access,
       Pixel  => Pixel,
       Format => Format);
#else
    AG_GetColor32
      (Color  => Color'Unchecked_Access,
       Pixel  => Pixel,
       Format => Format);
#end if;
    R := Color.R;
    G := Color.G;
    B := Color.B;
    A := Color.A;
  end;

  --
  -- Set source alpha flag and per-surface alpha value.
  --
  procedure Set_Alpha
    (Surface   : in Surface_not_null_Access;
     Enable    : in Boolean := false;
     Alpha     : in AG_Component := AG_OPAQUE) is
  begin
    if (Enable) then
      Surface.Flags := Surface.Flags or SURFACE_ALPHA;
    else
      Surface.Flags := Surface.Flags and not SURFACE_ALPHA;
    end if;
    Surface.Alpha := Alpha;
  end;
  
  --
  -- Set source colorkey flag and surface colorkey value.
  --
  procedure Set_Colorkey
    (Surface  : in Surface_not_null_Access;
     Enable   : in Boolean := false;
     Colorkey : in AG_Pixel := 0) is
  begin
    if (Enable) then
      Surface.Flags := Surface.Flags or SURFACE_COLORKEY;
    else
      Surface.Flags := Surface.Flags and not SURFACE_COLORKEY;
    end if;
    Surface.Colorkey := Colorkey;
  end;
  
  --
  -- Get surface clipping rectangle.
  -- 
  procedure Get_Clipping_Rect
    (Surface : in     Surface_not_null_Access;
     X,Y,W,H :    out Natural) is
  begin
    X := Natural(Surface.Clip_Rect.X);
    Y := Natural(Surface.Clip_Rect.Y);
    W := Natural(Surface.Clip_Rect.W);
    H := Natural(Surface.Clip_Rect.H);
  end;
  
  procedure Set_Clipping_Rect
    (Surface : in Surface_not_null_Access;
     X,Y,W,H : in Natural) is
  begin
    Surface.Clip_Rect.X := C.int(X);
    Surface.Clip_Rect.Y := C.int(Y);
    Surface.Clip_Rect.W := C.int(W);
    Surface.Clip_Rect.H := C.int(H);
  end;

end Agar.Surface;
