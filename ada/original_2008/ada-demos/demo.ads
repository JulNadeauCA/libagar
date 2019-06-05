with agar.gui.window;

package demo is

  window : agar.gui.window.window_access_t;

  procedure init
    (name          : string;
     video_width   : integer := 640;
     video_height  : integer := 480;
     window_width  : integer := 320;
     window_height : integer := 240);

  procedure run;
  procedure finish;

end demo;
