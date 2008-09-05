package body agar.core.datasource is

  package cbinds is
    function open
      (path : cs.chars_ptr;
       mode : cs.chars_ptr) return datasource_access_t;
    pragma import (c, open, "AG_OpenFile");
  end cbinds;

  function open
    (path : string;
     mode : string) return datasource_access_t
  is
    ca_path : aliased c.char_array := c.to_c (path);
    ca_mode : aliased c.char_array := c.to_c (mode);
  begin
    return cbinds.open
      (path => cs.to_chars_ptr (ca_path'unchecked_access),
       mode => cs.to_chars_ptr (ca_mode'unchecked_access));
  end open;

end agar.core.datasource;
