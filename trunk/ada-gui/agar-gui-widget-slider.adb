package body agar.gui.widget.slider is

  use type c.int;

  package cbinds is
    procedure set_int_increment
      (slider : slider_access_t;
       increment : c.int);
    pragma import (c, set_int_increment, "AG_SliderSetIntIncrement");

    procedure set_real_increment
      (slider : slider_access_t;
       increment : c.double);
    pragma import (c, set_real_increment, "AG_SliderSetRealIncrement");
  end cbinds;

  procedure set_increment
    (slider : slider_access_t;
     increment : positive) is
  begin
    cbinds.set_int_increment
      (slider => slider,
       increment => c.int (increment));
  end set_increment;

  procedure set_increment
    (slider : slider_access_t;
     increment : long_float) is
  begin
    cbinds.set_real_increment
      (slider => slider,
       increment => c.double (increment));
  end set_increment;

  function widget (slider : slider_access_t) return widget_access_t is
  begin
    return slider.widget'access;
  end widget;

end agar.gui.widget.slider;
