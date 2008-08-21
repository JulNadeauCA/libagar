package agar.core.config is

  function file
    (path_key  : cs.chars_ptr;
     name      : cs.chars_ptr;
     extension : cs.chars_ptr;
     dst_path  : cs.chars_ptr;
     dst_len   : c.size_t) return c.int;
  pragma import (c, file, "AG_ConfigFile");

  function file
    (path_key  : string;
     name      : string;
     extension : string;
     dst_path  : string;
     dst_len   : positive) return boolean;
  pragma inline (file); 

end agar.core.config;
