with agar.core;
with interfaces.c;

package agar.gui is
  package c renames interfaces.c;

  function init (flags : agar.core.init_flags_t := 0) return c.int;
  pragma import (c, init, "AG_InitGUI");

  function init (flags : agar.core.init_flags_t := 0) return boolean;
  pragma inline (init);

  procedure destroy;
  pragma import (c, destroy, "AG_DestroyGUI");

end agar.gui;
