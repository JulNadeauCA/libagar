package body agar.gui.widget.progress_bar is

  package cbinds is
    procedure set_width
      (bar   : progress_bar_access_t;
       width : c.int);
    pragma import (c, set_width, "AG_ProgressBarPercent");
  end cbinds;

  procedure set_width
    (bar   : progress_bar_access_t;
     width : natural) is
  begin
    cbinds.set_width
      (bar   => bar,
       width => c.int (width));
  end set_width;
 
  function widget (bar : progress_bar_access_t) return widget_access_t is
  begin
    return bar.widget'access;
  end widget;

end agar.gui.widget.progress_bar;
