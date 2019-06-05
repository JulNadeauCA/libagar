--------------------------------------------------
--    Example of a custom Agar widget class:    --
--                                              --
-- AG_Object(3) -> AG_Widget(3) -> CustomWidget --
--------------------------------------------------
with Agar.Object;
with Agar.Widget; use Agar.Widget;
with Agar.Data_Source;
with Agar.Variable;
with Interfaces.C;

package CustomWidget is
  package OBJ renames Agar.Object; 
  package AV renames Agar.Variable; 
  package C renames Interfaces.C;
  
  type CustomWidget_t is limited record
    widget     : Widget_t;             -- AG_Object -> AG_Widget -> CustomWidget

    Color      : Interfaces.Unsigned_32;
    State_X    : Interfaces.Unsigned_8;
    State_Y    : Interfaces.Unsigned_8;
    Label_Surf : C.int;
  end record
    with Convention => C;
  type CustomWidget_Access_t is access all CustomWidget_t
    with Convention => C;
  
  Widget_Class : Widget_Class_Access_t := null;
  
  function Create_Class return OBJ.Class_Not_Null_Access_t;
  procedure Destroy_Class;

  procedure Init (Object : OBJ.Object_Access_t) with Convention => C;
  procedure Draw (Widget : Widget_Access_t) with Convention => C;
  procedure Size_Request
    (Widget : in Widget_Access_t;
     Size   : in SizeReq_not_null_Access_t) with Convention => C;
  function Size_Allocate
    (Widget : in Widget_Access_t;
     Size   : in SizeAlloc_not_null_Access_t) return C.int with Convention => C;

end CustomWidget;
