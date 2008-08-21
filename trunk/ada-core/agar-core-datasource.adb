package body agar.core.datasource is

  function open
    (path : string;
     mode : string) return datasource_access_t
  is
    ca_path : aliased c.char_array := c.to_c (path);
    ca_mode : aliased c.char_array := c.to_c (mode);
  begin
    return open
      (path => cs.to_chars_ptr (ca_path'unchecked_access),
       mode => cs.to_chars_ptr (ca_mode'unchecked_access));
  end open;

end agar.core.datasource;
