package body agar.core is

  use type c.int;

  function init_core
    (progname : string;
     flags    : init_flags_t) return boolean
  is
    ca_progname : aliased c.char_array := c.to_c (progname);
  begin
    return init_core
      (progname => cs.to_chars_ptr (ca_progname'unchecked_access),
       flags    => flags) = 0;
  end init_core;

end agar.core;
