package agar.core.config is

  function file
    (path_key  : string;
     name      : string;
     extension : string;
     dst_path  : string;
     dst_len   : positive) return boolean;
  pragma inline (file); 

end agar.core.config;
