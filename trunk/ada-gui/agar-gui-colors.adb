package body agar.gui.colors is

  use type c.int;

  function load (path : string) return boolean is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return load (cs.to_chars_ptr (ca_path'unchecked_access)) = 0;
  end load;

  function save (path : string) return boolean is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return save (cs.to_chars_ptr (ca_path'unchecked_access)) = 0;
  end save;

  function save_default return boolean is
  begin
    return save_default = 0;
  end save_default;

end agar.gui.colors;
