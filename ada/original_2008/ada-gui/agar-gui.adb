with agar.core.error;

package body agar.gui is

  use type c.int;

  package cbinds is
    function init (flags : agar.core.init_flags_t := 0) return c.int;
    pragma import (c, init, "AG_InitGUI");

    function init_video
      (width  : c.int;
       height : c.int;
       bpp    : c.int;
       flags  : video_flags_t := 0) return c.int;
    pragma import (c, init_video, "AG_InitVideo");
  end cbinds;

  protected state is
    procedure init_gui;
    procedure init_video;
    function inited_gui return boolean;
    function inited_video return boolean;
  private
    gui   : boolean := false;
    video : boolean := false;
  end state;

  protected body state is
    procedure init_gui is begin gui := true; end;
    procedure init_video is begin video := true; end;
    function inited_gui return boolean is begin return gui; end;
    function inited_video return boolean is begin return video; end;
  end state;

  function init_video
    (width  : positive;
     height : positive;
     bpp    : natural;
     flags  : video_flags_t := 0) return boolean is
  begin
    if cbinds.init_video
      (width  => c.int (width),
       height => c.int (height),
       bpp    => c.int (bpp),
       flags  => flags) = 0 then
      state.init_video;
    end if;
    return state.inited_video;
  end init_video;

  function init (flags : agar.core.init_flags_t := 0) return boolean is
  begin
    if state.inited_video then
      if cbinds.init (flags) = 0 then
        state.init_gui;
      end if;
    else
      agar.core.error.set ("video must be initialised before GUI");
    end if;
    return state.inited_gui;
  end init;


end agar.gui;
