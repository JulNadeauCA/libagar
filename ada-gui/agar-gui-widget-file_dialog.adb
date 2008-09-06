package body agar.gui.widget.file_dialog is

  use type c.int;

  package cbinds is
    function allocate_mru
      (parent : widget_access_t;
       key    : cs.chars_ptr;
       flags  : flags_t) return file_dialog_access_t;
    pragma import (c, allocate_mru, "AG_FileDlgNewMRU");

    function set_directory
      (dialog : file_dialog_access_t;
       path   : cs.chars_ptr) return c.int;
    pragma import (c, set_directory, "AG_FileDlgSetDirectory");

    procedure set_directory_mru
      (dialog : file_dialog_access_t;
       key    : cs.chars_ptr;
       path   : cs.chars_ptr);
    pragma import (c, set_directory_mru, "AG_FileDlgSetDirectoryMRU");

    procedure set_filename
      (dialog : file_dialog_access_t;
       file   : cs.chars_ptr);
    pragma import (c, set_filename, "AG_FileDlgSetFilename");
  end cbinds;

  function allocate_mru
    (parent : widget_access_t;
     key    : string;
     flags  : flags_t) return file_dialog_access_t
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    return cbinds.allocate_mru
      (parent => parent,
       key    => cs.to_chars_ptr (ca_key'unchecked_access),
       flags  => flags);
  end allocate_mru;

  function set_directory
    (dialog : file_dialog_access_t;
     path   : string) return boolean
  is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return cbinds.set_directory
      (dialog => dialog,
       path   => cs.to_chars_ptr (ca_path'unchecked_access)) = 0;
  end set_directory;

  procedure set_directory_mru
    (dialog : file_dialog_access_t;
     key    : string;
     path   : string)
  is
    ca_key  : aliased c.char_array := c.to_c (key);
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    cbinds.set_directory_mru
      (dialog => dialog,
       key    => cs.to_chars_ptr (ca_key'unchecked_access),
       path   => cs.to_chars_ptr (ca_path'unchecked_access));
  end set_directory_mru;

  procedure set_filename
    (dialog : file_dialog_access_t;
     file   : string)
  is
    ca_file : aliased c.char_array := c.to_c (file);
  begin
    cbinds.set_filename
      (dialog => dialog,
       file   => cs.to_chars_ptr (ca_file'unchecked_access));
  end set_filename;

end agar.gui.widget.file_dialog;
