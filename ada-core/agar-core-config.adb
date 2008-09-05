package body agar.core.config is

  use type c.int;

  package cbinds is

    function file
      (path_key  : cs.chars_ptr;
       name      : cs.chars_ptr;
       extension : cs.chars_ptr;
       dst_path  : cs.chars_ptr;
       dst_len   : c.size_t) return c.int;
    pragma import (c, file, "AG_ConfigFile");

  end cbinds;

  function file
    (path_key  : string;
     name      : string;
     extension : string;
     dst_path  : string;
     dst_len   : positive) return boolean
  is
    ca_path_key  : aliased c.char_array := c.to_c (path_key);
    ca_name      : aliased c.char_array := c.to_c (name);
    ca_extension : aliased c.char_array := c.to_c (extension);
    ca_dst_path  : aliased c.char_array := c.to_c (dst_path);
  begin
    return cbinds.file
      (path_key  => cs.to_chars_ptr (ca_path_key'unchecked_access),
       name      => cs.to_chars_ptr (ca_name'unchecked_access),
       extension => cs.to_chars_ptr (ca_extension'unchecked_access),
       dst_path  => cs.to_chars_ptr (ca_dst_path'unchecked_access),
       dst_len   => c.size_t (dst_len)) = 0;
  end file;

end agar.core.config;
