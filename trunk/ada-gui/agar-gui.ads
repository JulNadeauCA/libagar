with agar.core;
with interfaces.c;

package agar.gui is
  package c renames interfaces.c;

  function init (flags : agar.core.init_flags_t) return c.int;
  pragma import (c, init, "AG_InitGUI");

  procedure destroy;
  pragma import (c, destroy, "AG_DestroyGUI");

end agar.gui;
