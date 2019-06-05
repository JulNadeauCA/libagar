with agar.core.event;
with agar.core;
with agar.gui.widget.button;
with agar.gui.widget;
with agar.gui.window;
with agar.gui;

with demo;
with keyevent_callbacks;

procedure keyevent is
  package gui_button renames agar.gui.widget.button;
  package gui_event renames agar.core.event;
  package gui_widget renames agar.gui.widget;
  package gui_window renames agar.gui.window;

  quit_button : gui_button.button_access_t;
begin
  demo.init ("keyevent");

  -- attach keydown handler function
  gui_event.set
    (object  => gui_widget.object (gui_window.widget (demo.window)),
     name    => "key-down",
     handler => keyevent_callbacks.keydown'access);

  -- attach keyup handler function
  gui_event.set
    (object  => gui_widget.object (gui_window.widget (demo.window)),
     name    => "key-up",
     handler => keyevent_callbacks.keyup'access);

  -- quit when closing window
  gui_event.set
    (object  => gui_widget.object (gui_window.widget (demo.window)),
     name    => "window-close",
     handler => keyevent_callbacks.quit'access);

  -- create quit button
  quit_button := gui_button.allocate_function
    (parent   => gui_window.widget (demo.window),
     label    => "quit",
     callback => keyevent_callbacks.quit'access);

  -- position and size
  gui_widget.modify_position
    (widget => gui_button.widget (quit_button),
     y      => 10);
  gui_widget.set_size
    (widget => gui_button.widget (quit_button),
     width  => 300,
     height => 32);

  demo.run;
  demo.finish;
end keyevent;
