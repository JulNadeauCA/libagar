package body agar.gui.widget.scrollbar is

  use type c.int;

  package cbinds is
    procedure set_size
      (scrollbar : scrollbar_access_t;
       size      : c.int);
    pragma import (c, set_size, "agar_gui_widget_scrollbar_set_size");
 
    function get_size (scrollbar : scrollbar_access_t) return c.int;
    pragma import (c, get_size, "agar_gui_widget_scrollbar_get_size");

    function visible (scrollbar : scrollbar_access_t) return c.int;
    pragma import (c, visible, "AG_ScrollbarVisible");

    procedure set_int_increment
      (scrollbar : scrollbar_access_t;
       increment : c.int);
    pragma import (c, set_int_increment, "AG_ScrollbarSetIntIncrement");

    procedure set_real_increment
      (scrollbar : scrollbar_access_t;
       increment : c.double);
    pragma import (c, set_real_increment, "AG_ScrollbarSetRealIncrement");
  end cbinds;

  procedure set_size
    (scrollbar : scrollbar_access_t;
     size      : natural) is
  begin
    cbinds.set_size
      (scrollbar => scrollbar,
       size      => c.int (size));
  end set_size;

  function get_size (scrollbar : scrollbar_access_t) return natural is
  begin
    return natural (cbinds.get_size (scrollbar));
  end get_size;

  function visible (scrollbar : scrollbar_access_t) return boolean is
  begin
    return cbinds.visible (scrollbar) = 1;
  end visible;

  procedure set_increment
    (scrollbar : scrollbar_access_t;
     increment : positive) is
  begin
    cbinds.set_int_increment
      (scrollbar => scrollbar,
       increment => c.int (increment));
  end set_increment;

  procedure set_increment
    (scrollbar : scrollbar_access_t;
     increment : long_float) is
  begin
    cbinds.set_real_increment
      (scrollbar => scrollbar,
       increment => c.double (increment));
  end set_increment;

  function widget (scrollbar : scrollbar_access_t) return widget_access_t is
  begin
    return scrollbar.widget'access;
  end widget;

end agar.gui.widget.scrollbar;
