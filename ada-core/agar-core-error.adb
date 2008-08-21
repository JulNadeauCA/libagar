package body agar.core.error is

  function get return string is
  begin
    return cs.value (get);
  end get;

  procedure set (str : string) is
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    set (cs.to_chars_ptr (ca_str'unchecked_access));
  end set;

  procedure fatal (str : string) is
    ca_str : aliased c.char_array := c.to_c (str);
  begin
    fatal (cs.to_chars_ptr (ca_str'unchecked_access));
  end fatal;

end agar.core.error;
