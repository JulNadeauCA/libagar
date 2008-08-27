package body agar.gui is

  use type c.int;

  function init (flags : agar.core.init_flags_t := 0) return boolean is
  begin
    return init (flags) = 0;
  end init;

end agar.gui;
