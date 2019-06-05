------------------------------------------
-- agar_ada_demo.adb: Agar-GUI Ada demo --
------------------------------------------
with Agar.Init;
with Agar.Init_GUI;
with Agar.Error;
with Agar.Surface; use Agar.Surface;
with Interfaces; use Interfaces;
--with Agar.Widget;

with Ada.Real_Time; use Ada.Real_Time;
with Ada.Text_IO;
with Ada.Numerics.Elementary_Functions;
use Ada.Numerics.Elementary_Functions;

procedure agar_ada_demo is
  package T_IO renames Ada.Text_IO;
  package RT renames Ada.Real_Time;
  package SU renames Agar.Surface;
  
  Epoch : constant RT.Time := RT.Clock;
begin
  if not Agar.Init.Init_Core ("agar_ada_demo") then
    raise program_error with Agar.Error.Get_Error;
  end if;
  if not Agar.Init_GUI.Init_GUI then
    raise program_error with Agar.Error.Get_Error;
  end if;

  declare
    Major : Natural;
    Minor : Natural;
    Patch : Natural;
  begin
    Agar.Init.Get_Version(Major, Minor, Patch);
    T_IO.Put_Line
      ("Agar version" &
       Integer'Image(Major) & " ." &
       Integer'Image(Minor) & " ." &
       Integer'Image(Patch) & " initialized in " &
       Duration'Image(RT.To_Duration(RT.Clock - Epoch)) & "s");
  end;

  T_IO.Put_Line("Size of AG_Surface = " &
      Natural'Image(SU.Surface'Size));

  --
  -- Create a surface of pixels.
  --
  declare
    W        : constant Natural := 640;
    H        : constant Natural := 480;
    Surf     : constant SU.Surface_Access := SU.New_Surface(W,H);
    Blue     : aliased SU.Color := (0,0,200,255);
    Border_W : constant Natural := 20;
  begin
    if Surf = null then
      raise Program_Error with Agar.Error.Get_Error;
    end if;
    
    --
    -- Fill the background
    --
    T_IO.Put_Line("Fill_Rect");
    SU.Fill_Rect
      (Surface => Surf,
       Color   => Blue'Unchecked_Access);        -- With a Color

    SU.Fill_Rect
      (Surface => Surf,
       R       => 120,          -- With RGB[A] components
       G       => 0,
       B       => 0);
 
    --
    -- Use Put_Pixel to create a gradient.
    --
    T_IO.Put_Line("Creating gradient");
    for Y in Border_W .. H-Border_W loop
      if Y rem 4 = 0 then
        Blue.B := Blue.B - 1;
      end if;
      Blue.G := 0;
      for X in Border_W .. W-Border_W loop
        if X rem 8 = 0 then
          Blue.G := Blue.G + 1;
        end if;
        SU.Put_Pixel
          (Surface  => Surf,
           X        => X,
           Y        => Y,
           Value    => Map_Pixel(Surf.Format'Unchecked_Access,
                                 Blue'Unchecked_Access),
           Clipping => false);
      end loop;
    end loop;

    --
    -- Generate a 2bpp indexed surface and initialize its 4-color palette.
    --
    T_IO.Put_Line("Creating a 4-color indexed surface");
    declare
      Bitmap     : SU.Surface_Access;
    begin
      T_IO.Put_Line("Creating surface...");
      
      Bitmap := SU.New_Surface
        (Mode           => INDEXED,
         Bits_per_Pixel => 2,
         W              => 128,
         H              => 128);
   
      if Bitmap = null then
        Agar.Error.Fatal_Error ("null bitmap");
      end if;
      
      T_IO.Put_Line("Created bitmap successfully");

      T_IO.Put_Line("Setting color #1");
      SU.Set_Color(Bitmap, 0, 0,  0,  0);
      T_IO.Put_Line("Setting color #2");
      SU.Set_Color(Bitmap, 1, 0,  100,0);
      T_IO.Put_Line("Setting color #3");
      SU.Set_Color(Bitmap, 2, 150,0,  0);
      T_IO.Put_Line("Setting color #4");
      SU.Set_Color(Bitmap, 3, 200,200,0);
      
      T_IO.Put_Line("Creating pattern...");

      for Y in 0 .. Bitmap.H loop
        for X in 0 .. Bitmap.W loop
          if Natural(X) rem 16 = 0 then
            SU.Put_Pixel
              (Surface  => Bitmap,
               X        => Integer(X),
               Y        => Integer(Y),
               Value    => 1);
          else
            if Natural(Y) rem 8 = 0 then
              SU.Put_Pixel
                (Surface  => Bitmap,
                 X        => Integer(X),
                 Y        => Integer(Y),
                 Value    => 1);
            elsif Sqrt(Float(X)*Float(X) + Float(Y)*Float(Y)) < 50.0 then
              SU.Put_Pixel
                (Surface  => Bitmap,
                 X        => Integer(X),
                 Y        => Integer(Y),
                 Value    => 2);
            elsif Sqrt(Float(X)*Float(X) + Float(Y)*Float(Y)) > 150.0 then
              SU.Put_Pixel
                (Surface  => Bitmap,
                 X        => Integer(X),
                 Y        => Integer(Y),
                 Value    => 3);
            else
              SU.Put_Pixel
                (Surface  => Bitmap,
                 X        => Integer(X),
                 Y        => Integer(Y),
                 Value    => 0);
            end if;
          end if;
        end loop;

      end loop;
      
      --
      -- Export our 2bpp indexed surface to a PNG file.
      --
      T_IO.Put_Line("Writing indexed 2bpp surface to output-index.png");
      -- Export to an indexed PNG file.
      if not SU.Export_PNG(Bitmap, "output-index.png") then
        T_IO.Put_Line ("output-index.png: " & Agar.Error.Get_Error);
      end if;

      --
      -- Conversion from indexed to RGBA is done implicitely by Blit.
      --
      T_IO.Put_Line("Testing blit conversion from indexed to packed");
      SU.Blit_Surface
        (Source => Bitmap,
         Target => Surf,
         Dst_X  => 32,
         Dst_Y  => 32);
    
      -- 
      -- Blit our indexed surface again using a different palette.
      --
      SU.Set_Color(Bitmap, 0, 255,255,255);
      SU.Set_Color(Bitmap, 1, 100,100,180);
      SU.Set_Color(Bitmap, 2, 120,0,0);
      SU.Set_Color(Bitmap, 3, 0,0,150);
      SU.Blit_Surface
        (Source => Bitmap,
         Target => Surf,
         Dst_X  => 200,
         Dst_Y  => 32);

      SU.Free_Surface (Bitmap);
    end;

    --
    -- Set a clipping rectangle.
    --
    SU.Set_Clipping_Rect
      (Surface => Surf,
       X       => 55,
       Y       => 220,
       W       => 640-(55*2),
       H       => 200);

    --
    -- Show the extent of the clipping rectangle.
    --
    T_IO.Put_Line("Testing clipping rectangles");
    declare
      White  : constant Pixel := Map_Pixel(Surf.Format'Access, 255,255,255);
      Clip_X : constant Integer := Integer(Surf.Clip_Rect.X);
      Clip_Y : constant Integer := Integer(Surf.Clip_Rect.Y);
      Clip_W : constant Integer := Integer(Surf.Clip_Rect.W);
      Clip_H : constant Integer := Integer(Surf.Clip_Rect.H);
      procedure Put_Crosshairs
        (Surface : Surface_Access;
         X,Y     : Natural;
         Value   : Pixel) is
      begin
        for Z in 1 .. 3 loop
          SU.Put_Pixel (Surface, X+Z,Y, Value, Clipping => false);
          SU.Put_Pixel (Surface, X-Z,Y, Value, Clipping => false);
          SU.Put_Pixel (Surface, X,Y+Z, Value, Clipping => false);
          SU.Put_Pixel (Surface, X,Y-Z, Value, Clipping => false);
        end loop;
      end;
    begin
      Put_Crosshairs (Surf, Clip_X,        Clip_Y,        White);
      Put_Crosshairs (Surf, Clip_X+Clip_W, Clip_Y,        White);
      Put_Crosshairs (Surf, Clip_X+Clip_W, Clip_Y+Clip_H, White);
      Put_Crosshairs (Surf, Clip_X,        Clip_Y+Clip_H, White);
    end;

    T_IO.Put_Line
      ("Surf W:" & C.unsigned'Image(Surf.W) &
           " H:" & C.unsigned'Image(Surf.H) &
           " Pitch:" & C.unsigned'Image(Surf.Pitch) &
           " Clip_X:" & C.int'Image(Surf.Clip_Rect.X) &
           " Clip_Y:" & C.int'Image(Surf.Clip_Rect.Y) &
           " Clip_W:" & C.int'Image(Surf.Clip_Rect.W) &
           " Clip_H:" & C.int'Image(Surf.Clip_Rect.H) &
           " Padding:" & C.unsigned'Image(Surf.Padding));

    --
    -- Load a surface from a PNG file and blit it onto Surf. Transparency is
    -- expressed by colorkey, or by an alpha component of 0 (in packed RGBA).
    --
    T_IO.Put_Line("Testing transparency");
    declare
      Denis : constant SU.Surface_Access := New_Surface("axe.png");
      Degs  : Float := 0.0;
      Alpha : Component := 0;
    begin
      if Denis /= null then
        T_IO.Put_Line
          ("Denis W:" & C.unsigned'Image(Denis.W) &
	         " H:" & C.unsigned'Image(Denis.H) &
	         " Pitch:" & C.unsigned'Image(Denis.Pitch) &
	         " Clip_X:" & C.int'Image(Denis.Clip_Rect.X) &
	         " Clip_Y:" & C.int'Image(Denis.Clip_Rect.Y) &
	         " Clip_W:" & C.int'Image(Denis.Clip_Rect.W) &
	         " Clip_H:" & C.int'Image(Denis.Clip_Rect.H) &
	         " Padding:" & C.unsigned'Image(Denis.Padding));
        for Y in 1 .. 50 loop
          Degs := Degs + 30.0;

          SU.Set_Alpha
	         (Surface => Denis,
	          Alpha   => Alpha);      -- Per-surface alpha
          Alpha := Alpha + 12;

          -- Render to target coordinates under Surf.
          for Z in 1 .. 3 loop
            SU.Blit_Surface
              (Source => Denis,
               Target => Surf,
               Dst_X  => Y*25,
               Dst_Y  => H/2 + Z*40 - Natural(Denis.H)/2 -
                                      Integer(50.0 * Sin(Degs,360.0)));
          end loop;
        end loop;
      else
        T_IO.Put_Line (Agar.Error.Get_Error);
      end if;
    end;
        
    T_IO.Put_Line("Testing export to PNG");
    if not SU.Export_PNG(Surf, "output.png") then
      raise program_error with Agar.Error.Get_Error;
    end if;
    T_IO.Put_Line ("Surface saved to output.png");

    SU.Free_Surface(Surf);
  end;

  T_IO.Put_Line
    ("Exiting after" &
     Duration'Image(RT.To_Duration(RT.Clock - Epoch)) & "s");

  Agar.Init.Quit;
end agar_ada_demo;
