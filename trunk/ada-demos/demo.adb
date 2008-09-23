with agar.core.error;
with agar.core;

package body demo is

  use type agar.gui.window.window_access_t;

  procedure init
    (name          : string;
     video_width   : integer := 640;
     video_height  : integer := 480;
     window_width  : integer := 320;
     window_height : integer := 240) is
  begin
    if not agar.core.init (name) then
      raise program_error with agar.core.error.get;
    end if;
    if not agar.gui.init_video
      (width  => video_width,
       height => video_height,
       bpp    => 32) then
      raise program_error with agar.core.error.get;
    end if;
  
    window := agar.gui.window.allocate_named (name => name & " test");
    if window = null then
      raise program_error with agar.core.error.get;
    end if;
  
    agar.gui.window.set_caption (window, "demo");
    agar.gui.window.set_geometry
      (window => window,
       x      => 10,
       y      => 10,
       width  => window_width,
       height => window_height);
  end init;

  procedure run is
  begin
    agar.gui.window.show (window);
    agar.gui.event_loop;
  end run;

  procedure finish is
  begin
    null;
  end finish;

end demo; 
