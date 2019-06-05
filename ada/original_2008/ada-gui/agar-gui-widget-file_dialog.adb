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
    pragma import (c, set_directory, "AG_FileDlgSetDirectoryS");

    procedure set_directory_mru
      (dialog : file_dialog_access_t;
       key    : cs.chars_ptr;
       path   : cs.chars_ptr);
    pragma import (c, set_directory_mru, "AG_FileDlgSetDirectoryMRU");

    procedure set_filename
      (dialog : file_dialog_access_t;
       file   : cs.chars_ptr);
    pragma import (c, set_filename, "AG_FileDlgSetFilenameS");

    function add_filetype
      (dialog      : file_dialog_access_t;
       description : cs.chars_ptr;
       extensions  : cs.chars_ptr;
       fmt         : agar.core.types.void_ptr_t) return filetype_access_t;
    pragma import (c, add_filetype, "AG_FileDlgAddType");
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

  function add_filetype
    (dialog      : file_dialog_access_t;
     description : string;
     extensions  : string) return filetype_access_t
  is
    ca_desc : aliased c.char_array := c.to_c (description);
    ca_ext  : aliased c.char_array := c.to_c (extensions);
  begin
    return cbinds.add_filetype
      (dialog      => dialog,
       description => cs.to_chars_ptr (ca_desc'unchecked_access),
       extensions  => cs.to_chars_ptr (ca_ext'unchecked_access),
       fmt         => agar.core.types.null_ptr);
  end add_filetype;

  function widget (dialog : file_dialog_access_t) return widget_access_t is
  begin
    return dialog.widget'access;
  end widget;

end agar.gui.widget.file_dialog;
