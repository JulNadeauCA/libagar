with ada.text_io;
with interfaces.c;

package body slider_callbacks is
  package io renames ada.text_io;
  package c renames interfaces.c;

  use type agar.core.types.integer_t;

  -- initialise slider array
  procedure init (window : gui_window.window_access_t) is
  begin
    for index in sliders'range loop
      -- set slider values
      sliders (index).value := 0;
      sliders (index).minimum := 0;
      sliders (index).maximum := agar.core.types.integer_t (index) * 100;

      -- create new slider
      sliders (index).slider := gui_slider.allocate_integer
        (parent   => gui_window.widget (window),
         bar_type => gui_slider.slider_horiz,
         flags    => gui_slider.slider_hfill,
         value    => sliders (index).value'access,
         min      => sliders (index).minimum'access,
         max      => sliders (index).maximum'access);

      -- create slider label
      sliders (index).label := gui_label.allocate_polled
        (parent => gui_window.widget (window),
         flags  => gui_label.label_hfill,
         text   => agar.core.types.integer_t'image (sliders (index).value));

      -- set callback for slider-changed events
      sliders (index).changed := gui_event.set
        (object  => gui_widget.object (gui_slider.widget (sliders (index).slider)),
         name    => "slider-changed",
         handler => changed'access);

      -- push slider index to event
      gui_event.push_int
        (event => sliders (index).changed,
         key   => "index",
         value => c.int (index));
    end loop;
  end init;

  procedure quit (event : gui_event.event_access_t) is
  begin
    io.put_line ("slider: exiting");
    agar.core.quit;
  end quit;

  -- fetch index from event, select slider and set label text.
  procedure changed (event : gui_event.event_access_t) is
    index : constant integer := gui_event.get_integer (event, 1);
    str   : constant string :=
      agar.core.types.integer_t'image (sliders (index).value);
  begin
    io.put_line ("slider: value changed");
    gui_label.text
      (label => sliders (index).label,
       text  => str);
  end changed;

end slider_callbacks;
