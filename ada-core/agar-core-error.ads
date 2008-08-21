package agar.core.error is

  function get return cs.chars_ptr;
  pragma import (c, get, "AG_GetError");

  function get return string;
  pragma inline (get);

  procedure set (str : cs.chars_ptr);
  pragma import (c, set, "AG_SetError");

  procedure set (str : string);
  pragma inline (set);

  procedure fatal (str : cs.chars_ptr);
  pragma import (c, fatal, "AG_FatalError");

  procedure fatal (str : string);
  pragma inline (fatal);

end agar.core.error;
