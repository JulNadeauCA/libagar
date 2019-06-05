package body agar.gui.widget.vbox is

  package cbinds is
    procedure set_homogenous
     (box        : vbox_access_t;
      homogenous : c.int);
    pragma import (c, set_homogenous, "agar_gui_widget_vbox_set_homogenous");

    procedure set_padding
     (box     : vbox_access_t;
      padding : c.int);
    pragma import (c, set_padding, "agar_gui_widget_vbox_set_padding");

    procedure set_spacing
     (box     : vbox_access_t;
      spacing : c.int);
    pragma import (c, set_spacing, "agar_gui_widget_vbox_set_spacing");
  end cbinds;

  procedure set_homogenous
    (box        : vbox_access_t;
     homogenous : boolean := true) is
  begin
    if homogenous then
      cbinds.set_homogenous (box, 1);
    else
      cbinds.set_homogenous (box, 0);
    end if;
  end set_homogenous;
              
  procedure set_padding
    (box     : vbox_access_t;
     padding : natural) is
  begin
    cbinds.set_padding
      (box     => box,
       padding => c.int (padding));
  end set_padding;
              
  procedure set_spacing
    (box     : vbox_access_t;
     spacing : natural) is
  begin
    cbinds.set_spacing
      (box     => box,
       spacing => c.int (spacing));
  end set_spacing;

  function widget (box : vbox_access_t) return widget_access_t is
  begin
    return agar.gui.widget.box.widget (box.box'access);
  end widget;

end agar.gui.widget.vbox;
