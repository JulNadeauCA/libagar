with agar.core.event;
with agar.core;
with agar.gui.widget;
with agar.gui.window;
with agar.gui;

with demo;
with slider_callbacks;

procedure slider is
  package gui_event renames agar.core.event;
  package gui_widget renames agar.gui.widget;
  package gui_window renames agar.gui.window;
begin
  demo.init ("slider", window_height => 350);

  -- allocate integer-bound slider
  slider_callbacks.init (demo.window);

  -- quit when closing window
  gui_event.set
    (object  => gui_widget.object (gui_window.widget (demo.window)),
     name    => "window-close",
     handler => slider_callbacks.quit'access);

  demo.run;
  demo.finish;
end slider;
