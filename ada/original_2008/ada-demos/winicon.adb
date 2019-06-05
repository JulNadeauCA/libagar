with SDL.image;
with SDL.video;
with SDL.error;

with agar.core.event;
with agar.core;
with agar.gui.surface;
with agar.gui.widget;
with agar.gui.window;
with agar.gui;

with demo;
with winicon_callbacks;

procedure winicon is
  package gui_event renames agar.core.event;
  package gui_widget renames agar.gui.widget;
  package gui_window renames agar.gui.window;
  package gui_surface renames agar.gui.surface;

  use type SDL.video.surface_access_t;
  use type gui_surface.surface_access_t;

  function open_icon (name : string) return gui_surface.surface_access_t is
    icon_sdl : SDL.video.surface_access_t;
    icon_ag  : gui_surface.surface_access_t;
  begin
    icon_sdl := SDL.image.load (name);
    if icon_sdl = null then
      raise program_error with SDL.image.get_error;
    end if;
    icon_ag := gui_surface.from_sdl (icon_sdl);
    if icon_ag = null then
      raise program_error with SDL.error.get_error;
    end if;
    SDL.video.free_surface (icon_sdl);
    return icon_ag;
  end open_icon;

  sphere : gui_surface.surface_access_t;
begin
  demo.init ("winicon");

  sphere := open_icon ("sphere.png");
  agar.gui.window.set_icon (demo.window, sphere);

  -- quit when closing window
  gui_event.set
    (object  => gui_widget.object (gui_window.widget (demo.window)),
     name    => "window-close",
     handler => winicon_callbacks.quit'access);

  demo.run;
  demo.finish;
end winicon;
