package body agar.gui.widget.separator is

  package cbinds is
    procedure set_padding
      (separator : separator_access_t;
       pixels    : c.int);
    pragma import (c, set_padding, "AG_SeparatorSetPadding");
  end cbinds;

  procedure set_padding
    (separator : separator_access_t;
     pixels    : natural) is
  begin
    cbinds.set_padding (separator, c.int (pixels));
  end set_padding;

  function widget (separator : separator_access_t) return widget_access_t is
  begin
    return separator.widget'access;
  end widget;

end agar.gui.widget.separator;
