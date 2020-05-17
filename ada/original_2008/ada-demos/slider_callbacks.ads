with agar.core.event;
with agar.core.types;
with agar.gui.widget.label;
with agar.gui.widget.slider;
with agar.gui.widget;
with agar.gui.window;

package slider_callbacks is
  package gui_event renames agar.core.event;
  package gui_label renames agar.gui.widget.label;
  package gui_slider renames agar.gui.widget.slider;
  package gui_widget renames agar.gui.widget;
  package gui_window renames agar.gui.window;

  type slider_t is record
    slider  : gui_slider.slider_access_t;
    label   : gui_label.label_access_t;
    value   : aliased agar.core.types.integer_t;
    minimum : aliased agar.core.types.integer_t;
    maximum : aliased agar.core.types.integer_t;
    changed : gui_event.event_access_t;
  end record;

  sliders : array (1 .. 8) of slider_t;

  procedure init (window : gui_window.window_access_t);

  procedure quit (event : gui_event.event_access_t);
  procedure changed (event : gui_event.event_access_t);

  pragma convention (c, changed);
  pragma convention (c, quit);
end slider_callbacks;
