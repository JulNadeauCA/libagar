package body agar.core.error is

  procedure c_set_error
    (fmt : cs.chars_ptr;
     str : cs.chars_ptr);
  pragma import (c, c_set_error, "AG_SetError");

  procedure c_fatal_error
    (fmt : cs.chars_ptr;
     str : cs.chars_ptr);
  pragma import (c, c_fatal_error, "AG_FatalError");

  function c_get_error return cs.chars_ptr;
  pragma import (c, c_get_error, "AG_GetError");

  function get return string is
  begin
    return cs.value (c_get_error);
  end get;

  procedure set (str : string) is
    ca_fmt : aliased c.char_array := c.to_c ("%s");
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    c_set_error
     (fmt => cs.to_chars_ptr (ca_fmt'unchecked_access),
      str => cs.to_chars_ptr (ca_str'unchecked_access));
  end set;

  procedure fatal (str : string) is
    ca_fmt : aliased c.char_array := c.to_c ("%s");
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    c_fatal_error
      (fmt => cs.to_chars_ptr (ca_fmt'unchecked_access),
       str => cs.to_chars_ptr (ca_str'unchecked_access));
  end fatal;

end agar.core.error;
