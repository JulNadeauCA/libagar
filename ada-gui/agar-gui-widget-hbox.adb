package body agar.gui.widget.hbox is

  package cbinds is
    procedure set_homogenous
     (box        : hbox_access_t;
      homogenous : c.int);
    pragma import (c, set_homogenous, "AG_HBoxSetHomogenous");

    procedure set_padding
     (box     : hbox_access_t;
      padding : c.int);
    pragma import (c, set_padding, "AG_HBoxSetPadding");

    procedure set_spacing
     (box     : hbox_access_t;
      spacing : c.int);
    pragma import (c, set_spacing, "AG_HBoxSetSpacing");
  end cbinds;

  procedure set_homogenous
    (box        : hbox_access_t;
     homogenous : boolean := true) is
  begin
    if homogenous then
      cbinds.set_homogenous (box, 1);
    else
      cbinds.set_homogenous (box, 0);
    end if;
  end set_homogenous;
              
  procedure set_padding
    (box     : hbox_access_t;
     padding : natural) is
  begin
    cbinds.set_padding
      (box     => box,
       padding => c.int (padding));
  end set_padding;
              
  procedure set_spacing
    (box     : hbox_access_t;
     spacing : natural) is
  begin
    cbinds.set_spacing
      (box     => box,
       spacing => c.int (spacing));
  end set_spacing;

end agar.gui.widget.hbox;
