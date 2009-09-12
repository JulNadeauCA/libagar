package body agar.gui.widget.box is

  package cbinds is
    procedure set_homogenous
      (box        : box_access_t;
       homogenous : c.int);
    pragma import (c, set_homogenous, "AG_BoxSetHomogenous");
    
    procedure set_label
      (box   : box_access_t;
       label : cs.chars_ptr);
    pragma import (c, set_label, "AG_BoxSetLabelS");
  
    procedure set_padding
      (box     : box_access_t;
       padding : c.int);
    pragma import (c, set_padding, "AG_BoxSetPadding");
  
    procedure set_spacing
      (box     : box_access_t;
       spacing : c.int);
    pragma import (c, set_spacing, "AG_BoxSetSpacing");
  
    procedure set_depth
      (box   : box_access_t;
       depth : c.int);
    pragma import (c, set_depth, "AG_BoxSetDepth");
  end cbinds;

  -- api

  function allocate_horizontal
    (parent   : widget_access_t;
     flags    : flags_t) return box_access_t is
  begin
    return allocate (parent, BOX_HORIZ, flags);
  end allocate_horizontal;
 
  function allocate_vertical
    (parent   : widget_access_t;
     flags    : flags_t) return box_access_t is
  begin
    return allocate (parent, BOX_VERT, flags);
  end allocate_vertical;
 
  procedure set_homogenous
    (box        : box_access_t;
     homogenous : boolean) is
    hflag : c.int := 0;
  begin
    if homogenous then hflag := 1; end if;
    cbinds.set_homogenous (box, hflag);
  end set_homogenous;

  procedure set_padding
    (box     : box_access_t;
     padding : natural) is
  begin
    cbinds.set_padding (box, c.int (padding));
  end set_padding;

  procedure set_spacing
    (box     : box_access_t;
     spacing : natural) is
  begin
    cbinds.set_spacing (box, c.int (spacing));
  end set_spacing;

  procedure set_depth
    (box   : box_access_t;
     depth : natural) is
  begin
    cbinds.set_depth (box, c.int (depth));
  end set_depth;

  function widget (box : box_access_t) return widget_access_t is
  begin
    return box.widget'access;
  end widget;

end agar.gui.widget.box;
