package body agar.core.dso is

  use type c.int;

  function load                
    (name  : string;           
     path  : string;           
     flags : c.unsigned) return dso_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return load
      (name  => cs.to_chars_ptr (ca_name'unchecked_access),
       path  => cs.to_chars_ptr (ca_path'unchecked_access),
       flags => flags);
  end load;

  function symbol  
    (dso         : dso_access_t;
     symbol_name : string;
     value       : agar.core.types.void_ptr_t) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (symbol_name);
  begin
    return symbol
      (dso         => dso,
       symbol_name => cs.to_chars_ptr (ca_name'unchecked_access),
       value       => value) = 0;
  end symbol;

  function unload (dso : dso_access_t) return boolean is
  begin
    return unload (dso) = 0;
  end unload;

end agar.core.dso;
