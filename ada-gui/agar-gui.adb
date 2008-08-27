package body agar.gui is

  use type c.int;

  function init (flags : agar.core.init_flags_t := 0) return boolean is
  begin
    return init (flags) = 0;
  end init;

  function init_video
    (width  : positive;
     height : positive;
     bpp    : natural;
     flags  : video_flags_t := 0) return boolean is
  begin
    return init_video
      (width  => c.int (width),
       height => c.int (height),
       bpp    => c.int (bpp),
       flags  => flags) = 0;
  end init_video;

end agar.gui;
