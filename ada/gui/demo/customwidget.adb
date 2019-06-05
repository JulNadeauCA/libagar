--------------------------------------------------
--    Example of a custom Agar widget class:    --
--                                              --
-- AG_Object(3) -> AG_Widget(3) -> CustomWidget --
--------------------------------------------------
with Ada.Text_IO;
with System.Address_To_Access_Conversions;

package body CustomWidget is
  package C_obj is new System.Address_To_Access_Conversions (CustomWidget_t);
  package T_IO renames Ada.Text_IO;

  use type C.int;

  --
  -- Register the CustomWidget class with the Agar object system. We give
  -- the size of our Object instance and the size of our Class structure
  -- (or Widget_Class_t'Size if the Class structure is not being overloaded,
  -- as is the case in this example).
  --
  -- The Version (Major, Minor) fields, and the Load and Save functions
  -- can be omitted since our example widget is not serializable.
  --
  function Create_Class return Widget_Class_not_null_Access_t is
  begin
    Widget_Class := Create_Widget_Class
      (Hierarchy       => "Agar(Widget):CustomWidget",
       Object_Size     => CustomWidget.CustomWidget_t'Size,
       Class_Size      => Widget_Class_t'Size,
       Init_Func       => CustomWidget.Init'Access,
       Destroy_Func    => CustomWidget.Destroy'Access,
       Size_Req_Func   => CustomWidget.Size_Req'Access,
       Size_Alloc_Func => CustomWidget.Size_Alloc'Access,
       Draw_Func       => CustomWidget.Draw'Access);

    return Widget_Class;
  end;

  --
  -- Delete the CustomWidget class (done automatically on exit).
  --
  procedure Destroy_Class is
  begin
    Destroy_Widget_Class (Widget_Class);
    Widget_Class := null;
  end;

  --
  -- Initialize a widget instance.
  --
  procedure Init (Object : OBJ.Object_Access_t)
  is
    CW : constant C_obj.Object_Pointer := C_obj.To_Pointer(Object.all'Address);
  begin
    T_IO.Put_Line("Custom widget init");
    CW.Color      := 16#c0c0c0#;
    CW.State_X    := 1;
    CW.State_Y    := 10;
    CW.Label_Surf := -1;
  end;

  --
  -- Render the widget to the display.
  --
  procedure Draw (Widget : Widget_Access_t)
  is
    CW : constant C_obj.Object_Pointer := C_obj.To_Pointer(Widget.all'Address);
  begin
    --
    -- Render a 3d-style box and a line across it.
    --
    Agar.Primitives.Draw_Box
      (X      => 0,
       Y      => 0,
       Width  => Widget.W,
       Height => Widget.H,
       Z      => 1,
       Color  => Widget_Color(Background_Color));
    Agar.Primitives.Draw_Line
      (X1    => 0,
       Y1    => 0,
       X2    => Widget.W,
       Y2    => Widget.H,
       Color => Widget_Color(Line_Color));

    --
    -- Render a text label. We generate and map the surface on demand so that
    -- it be stored efficiently as a hardware texture when possible.
    --
    if CW.Label_Surf = -1 then
      CW.Label_Surf := Map_Surface
        (Widget  => Widget,
	 Surface => Agar.Text.Render("Hello")));
    end if;
    if CW.Label_Surf /= -1 then
      Blit_Surface
        (Widget  => Widget,
	 Surface => CW.Label_Surf,
	 X       => Widget.W/2 - Surface_Width(CW.Label.Surf),
	 Y       => Widget.H/2 - Surface_Height(CW.Label.Surf));
    end if;
  end;

  --
  -- Request an initial preferred widget geometry.
  --
  procedure Size_Request
    (Widget : in Widget_Access_t;
     Size   : in SizeReq_not_null_Access_t)
  is
  begin
    Size.W := 320;
    Size.H := 240;
  end;

  --
  -- Callback after final widget geometry has been allocated by the parent
  -- container. If Widget is a container itself, this is also where the
  -- effective geometry of the attached child widgets is determined.
  --
  function Size_Allocate
    (Widget : in Widget_Access_t;
     Size   : in SizeAlloc_not_null_Access_t) return C.int
  is
  begin
    if Size.W < 160 or Size.H < 120 then              -- Require a minimum size
      return -1;
    end if;

    -- Allocate the geometry of any child widgets here.

    return 0;
  end;

end CustomWidget;
