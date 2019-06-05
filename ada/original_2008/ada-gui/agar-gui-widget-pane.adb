package body agar.gui.widget.pane is

  package cbinds is
    procedure set_divider_width
      (pane   : pane_access_t;
       pixels : c.int);
    pragma import (c, set_divider_width, "AG_PaneSetDividerWidth");

    procedure set_division_minimum
      (pane      : pane_access_t;
       which     : partition_t;
       min_width : c.int;
       max_width : c.int);
    pragma import (c, set_division_minimum, "AG_PaneSetDivisionMin");

    function move_divider
      (pane : pane_access_t;
       x    : c.int) return c.int;
    pragma import (c, move_divider, "AG_PaneMoveDivider");
  end cbinds;

  procedure set_divider_width
    (pane   : pane_access_t;
     pixels : positive) is
  begin
    cbinds.set_divider_width
      (pane   => pane,
       pixels => c.int (pixels));
  end set_divider_width;

  procedure set_division_minimum
    (pane      : pane_access_t;
     which     : partition_t;
     min_width : positive;
     max_width : positive) is
  begin
    cbinds.set_division_minimum
      (pane      => pane,
       which     => which,
       min_width => c.int (min_width),
       max_width => c.int (max_width));
  end set_division_minimum;

  function move_divider
    (pane : pane_access_t;
     x    : positive) return positive is
  begin
    return positive (cbinds.move_divider
      (pane => pane,
       x    => c.int (x)));
  end move_divider;

  function widget (pane : pane_access_t) return widget_access_t is
  begin
    return pane.widget'access;
  end widget;

end agar.gui.widget.pane;
