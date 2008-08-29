package body agar.gui.widget.box is

  procedure c_set_homogenous
    (box        : box_access_t;
     homogenous : c.int);
  pragma import (c, c_set_homogenous, "AG_BoxSetHomogenous");

  procedure c_set_padding
    (box     : box_access_t;
     padding : c.int);
  pragma import (c, c_set_padding, "AG_BoxSetPadding");

  procedure c_set_spacing
    (box     : box_access_t;
     spacing : c.int);
  pragma import (c, c_set_spacing, "AG_BoxSetSpacing");

  procedure c_set_depth
    (box   : box_access_t;
     depth : c.int);
  pragma import (c, c_set_depth, "AG_BoxSetDepth");

  -- api

  function allocate_horizontal
    (widget   : widget_access_t;
     flags    : flags_t) return box_access_t is
  begin
    return allocate (widget, BOX_HORIZ, flags);
  end allocate_horizontal;
 
  function allocate_vertical
    (widget   : widget_access_t;
     flags    : flags_t) return box_access_t is
  begin
    return allocate (widget, BOX_VERT, flags);
  end allocate_vertical;
 
  procedure set_homogenous
    (box        : box_access_t;
     homogenous : boolean) is
    hflag : c.int := 0;
  begin
    if homogenous then hflag := 1; end if;
    c_set_homogenous (box, hflag);
  end set_homogenous;

  procedure set_padding
    (box     : box_access_t;
     padding : natural) is
  begin
    c_set_padding (box, c.int (padding));
  end set_padding;

  procedure set_spacing
    (box     : box_access_t;
     spacing : natural) is
  begin
    c_set_spacing (box, c.int (spacing));
  end set_spacing;

  procedure set_depth
    (box   : box_access_t;
     depth : natural) is
  begin
    c_set_depth (box, c.int (depth));
  end set_depth;

end agar.gui.widget.box;
