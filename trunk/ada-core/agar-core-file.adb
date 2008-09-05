with agar.core.limits;

package body agar.core.file is

  use type c.int;
  use type c.unsigned;

  package cbinds is
    function get_file_info
      (name : cs.chars_ptr;
       info : info_access_t) return c.int;
    pragma import (c, get_file_info, "AG_GetFileInfo");

    function get_system_temp_dir
      (name : cs.chars_ptr;
       size : c.size_t) return c.int;
    pragma import (c, get_system_temp_dir, "AG_GetSystemTempDir");

    function file_exists (name : cs.chars_ptr) return c.int;
    pragma import (c, file_exists, "AG_FileExists");

    function file_delete (name : cs.chars_ptr) return c.int;
    pragma import (c, file_delete, "AG_FileDelete");
  end cbinds;

  --

  function get_info
    (name : string;
     info : info_access_t) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.get_file_info
      (name => cs.to_chars_ptr (ca_name'unchecked_access),
       info => info) = 0;
  end get_info;

  function get_system_temp_dir return string
  is
    ca_siz : constant c.size_t := agar.core.limits.pathname_max;
    ca_dir : aliased c.char_array := (1 .. ca_siz => c.nul);
  begin
    if cbinds.get_system_temp_dir
      (name => cs.to_chars_ptr (ca_dir'unchecked_access),
       size => ca_siz) = 0 then
      return c.to_ada (ca_dir);
    else
      raise program_error with "could not get temporary directory";
    end if;
  end get_system_temp_dir;

  function exists (name : string) return boolean is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.file_exists (cs.to_chars_ptr (ca_name'unchecked_access)) = 1;
  end exists;

  function delete (name : string) return boolean is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.file_delete (cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end delete;

end agar.core.file;
