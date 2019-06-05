with agar.core;
with interfaces.c;

package agar.gui is
  package c renames interfaces.c;

  -- Flags for init_video
  type video_flags_t is new c.unsigned;
  HWSURFACE     : constant video_flags_t := 16#001#;
  ASYNCBLIT     : constant video_flags_t := 16#002#;
  ANYFORMAT     : constant video_flags_t := 16#004#;
  HWPALETTE     : constant video_flags_t := 16#008#;
  DOUBLEBUF     : constant video_flags_t := 16#010#;
  FULLSCREEN    : constant video_flags_t := 16#020#;
  RESIZABLE     : constant video_flags_t := 16#040#;
  NOFRAME       : constant video_flags_t := 16#080#;
  BGPOPUPMENU   : constant video_flags_t := 16#100#;
  OPENGL        : constant video_flags_t := 16#200#;
  OPENGL_OR_SDL : constant video_flags_t := 16#400#;
  NOBGCLEAR     : constant video_flags_t := 16#800#;
  SDL		: constant video_flags_t := 16#2000#;

  function init (flags : agar.core.init_flags_t := 0) return boolean;
  pragma inline (init);

  function init_video
    (width  : positive;
     height : positive;
     bpp    : natural;
     flags  : video_flags_t := 0) return boolean;
  pragma inline (init_video);

  procedure destroy;
  pragma import (c, destroy, "AG_DestroyGUI");

  procedure event_loop;
  pragma import (c, event_loop, "agar_event_loop");

  procedure event_loop_fixed_fps;
  pragma import (c, event_loop_fixed_fps, "AG_EventLoop_FixedFPS");

end agar.gui;
