package body agar.gui.widget.fixed_plotter is

  package cbinds is
    function curve
      (plotter : plotter_access_t;
       name    : cs.chars_ptr;
       r       : agar.core.types.uint8_t;
       g       : agar.core.types.uint8_t;
       b       : agar.core.types.uint8_t;
       limit   : agar.core.types.uint32_t) return item_access_t;
    pragma import (c, curve, "AG_FixedPlotterCurve");
  end cbinds;

  function curve
    (plotter : plotter_access_t;
     name    : string;
     r       : agar.core.types.uint8_t;
     g       : agar.core.types.uint8_t;
     b       : agar.core.types.uint8_t;
     limit   : agar.core.types.uint32_t) return item_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.curve
      (plotter => plotter,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       r       => r,
       g       => g,
       b       => b,
       limit   => limit);
  end curve;

  function widget (plotter : plotter_access_t) return widget_access_t is
  begin
    return plotter.widget'access;
  end widget;

end agar.gui.widget.fixed_plotter;
