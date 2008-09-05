package body agar.core.dso is

  use type c.int;

  package cbinds is
    function load
      (name  : cs.chars_ptr;
       path  : cs.chars_ptr;
       flags : c.unsigned) return dso_access_t;
    pragma import (c, load, "AG_LoadDSO");

    function symbol
      (dso         : dso_access_t;
       symbol_name : cs.chars_ptr;
       value       : agar.core.types.void_ptr_t) return c.int;
    pragma import (c, symbol, "AG_SymDSO");

    function unload (dso : dso_access_t) return c.int;
    pragma import (c, unload, "AG_UnloadDSO");
  end cbinds;

  function load                
    (name  : string;           
     path  : string;           
     flags : c.unsigned) return dso_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return cbinds.load
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
    return cbinds.symbol
      (dso         => dso,
       symbol_name => cs.to_chars_ptr (ca_name'unchecked_access),
       value       => value) = 0;
  end symbol;

  function unload (dso : dso_access_t) return boolean is
  begin
    return cbinds.unload (dso) = 0;
  end unload;

end agar.core.dso;
