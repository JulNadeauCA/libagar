package body agar.core.error is

  package cbinds is
    procedure set_error
      (fmt : cs.chars_ptr;
       str : cs.chars_ptr);
    pragma import (c, set_error, "AG_SetError");

    procedure fatal_error
      (fmt : cs.chars_ptr;
       str : cs.chars_ptr);
    pragma import (c, fatal_error, "AG_FatalError");

    function get_error return cs.chars_ptr;
    pragma import (c, get_error, "AG_GetError");
  end cbinds;

  function get return string is
  begin
    return cs.value (cbinds.get_error);
  end get;

  procedure set (str : string) is
    ca_fmt : aliased c.char_array := c.to_c ("%s");
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    cbinds.set_error
     (fmt => cs.to_chars_ptr (ca_fmt'unchecked_access),
      str => cs.to_chars_ptr (ca_str'unchecked_access));
  end set;

  procedure fatal (str : string) is
    ca_fmt : aliased c.char_array := c.to_c ("%s");
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    cbinds.fatal_error
      (fmt => cs.to_chars_ptr (ca_fmt'unchecked_access),
       str => cs.to_chars_ptr (ca_str'unchecked_access));
  end fatal;

end agar.core.error;
