with agar.core.error;
with agar.core;
with agar.gui.window;
with agar.gui;

procedure helloworld1 is
  use type agar.gui.window.window_access_t;

  win : agar.gui.window.window_access_t;
begin

  if not agar.core.init ("helloworld1") then
    raise program_error with agar.core.error.get;
  end if;
  if not agar.gui.init_video
    (width  => 640,
     height => 480,
     bpp    => 32) then
    raise program_error with agar.core.error.get;
  end if;

  win := agar.gui.window.allocate_named (name => "helloworld1 test");
  if win = null then
    raise program_error with agar.core.error.get;
  end if;

  agar.gui.window.set_caption (win, "example");
  agar.gui.window.set_geometry
    (window => win,
     x      => 10,
     y      => 10,
     width  => 320,
     height => 240);
  agar.gui.window.show (win);
  agar.gui.event_loop;

end helloworld1;
