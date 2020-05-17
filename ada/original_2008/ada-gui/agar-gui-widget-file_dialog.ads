with agar.core.event;
with agar.core.limits;
with agar.core.tail_queue;
with agar.gui.widget.button;
with agar.gui.widget.combo;
with agar.gui.widget.label;
with agar.gui.widget.pane;
with agar.gui.widget.textbox;
with agar.gui.widget.tlist;

package agar.gui.widget.file_dialog is

  use type c.unsigned;

  type option_t is limited private;
  type option_access_t is access all option_t;
  pragma convention (c, option_access_t);

  type filetype_t;
  type filetype_access_t is access all filetype_t;
  pragma convention (c, filetype_access_t);

  type file_dialog_t is limited private;
  type file_dialog_access_t is access all file_dialog_t;
  pragma convention (c, file_dialog_access_t);

  package option_tail_queue is new agar.core.tail_queue
    (entry_type => option_access_t);
  package filetype_tail_queue is new agar.core.tail_queue
    (entry_type => filetype_access_t);

  type option_type_t is (
    FILE_DIALOG_BOOL,
    FILE_DIALOG_INT,
    FILE_DIALOG_FLOAT,
    FILE_DIALOG_DOUBLE,
    FILE_DIALOG_STRING
  );
  for option_type_t use (
    FILE_DIALOG_BOOL   => 0,
    FILE_DIALOG_INT    => 1,
    FILE_DIALOG_FLOAT  => 2,
    FILE_DIALOG_DOUBLE => 3,
    FILE_DIALOG_STRING => 4
  );
  for option_type_t'size use c.unsigned'size;
  pragma convention (c, option_type_t);

  -- union

  type data_int_t is record
    val : c.int;
    min : c.int;
    max : c.int;
  end record;
  pragma convention (c, data_int_t);

  type data_flt_t is record
    val : c.c_float;
    min : c.c_float;
    max : c.c_float;
  end record;
  pragma convention (c, data_flt_t);

  type data_dbl_t is record
    val : c.double;
    min : c.double;
    max : c.double;
  end record;
  pragma convention (c, data_dbl_t);

  type data_str_t is array (1 .. 128) of aliased c.char;
  pragma convention (c, data_str_t);

  type data_selector_t is (select_int, select_float, select_double, select_string);
  type option_data_t (member : data_selector_t := select_int) is record
    case member is
      when select_int    => i : data_int_t;
      when select_float  => f : data_flt_t;
      when select_double => d : data_dbl_t;
      when select_string => s : data_str_t;
    end case;
  end record;
  pragma convention (c, option_data_t);
  pragma unchecked_union (option_data_t);

  type filetype_t is record
    fd       : file_dialog_access_t;
    descr    : cs.chars_ptr;
    exts     : access cs.chars_ptr;
    num_exts : c.unsigned;
    action   : access agar.core.event.event_t;
    opts     : option_tail_queue.head_t;
    types    : filetype_tail_queue.entry_t;
  end record;
  pragma convention (c, filetype_t);

  type flags_t is new c.unsigned;
  FILEDLG_MULTI    : constant flags_t := 16#001#;
  FILEDLG_CLOSEWIN : constant flags_t := 16#002#;
  FILEDLG_LOAD     : constant flags_t := 16#004#;
  FILEDLG_SAVE     : constant flags_t := 16#008#;
  FILEDLG_ASYNC    : constant flags_t := 16#010#;
  FILEDLG_HFILL    : constant flags_t := 16#100#;
  FILEDLG_VFILL    : constant flags_t := 16#200#;
  FILEDLG_EXPAND   : constant flags_t := FILEDLG_HFILL or FILEDLG_VFILL;

  type path_t is array (1 .. agar.core.limits.pathname_max) of aliased c.char;
  pragma convention (c, path_t);

  -- API

  function allocate
    (parent : widget_access_t;
     flags  : flags_t) return file_dialog_access_t;
  pragma import (c, allocate, "AG_FileDlgNew");

  function allocate_mru
    (parent : widget_access_t;
     key    : string;
     flags  : flags_t) return file_dialog_access_t;
  pragma inline (allocate_mru);

  function set_directory
    (dialog : file_dialog_access_t;
     path   : string) return boolean;
  pragma inline (set_directory);

  procedure set_directory_mru
    (dialog : file_dialog_access_t;
     key    : string;
     path   : string);
  pragma inline (set_directory_mru);

  procedure set_filename
    (dialog : file_dialog_access_t;
     file   : string);
  pragma inline (set_filename);

  function add_filetype
    (dialog      : file_dialog_access_t;
     description : string;
     extensions  : string) return filetype_access_t;
  pragma inline (add_filetype);

  function widget (dialog : file_dialog_access_t) return widget_access_t;
  pragma inline (widget);

private

  type option_t is record
    desc        : cs.chars_ptr;
    key         : cs.chars_ptr;
    unit        : cs.chars_ptr;
    option_type : option_type_t;
    data        : option_data_t;
    opts        : option_tail_queue.entry_t;
  end record;
  pragma convention (c, option_t);

  type file_dialog_t is record
    widget        : aliased widget_t;
    flags         : flags_t;
    cwd           : path_t;
    cfile         : path_t;
    pane          : agar.gui.widget.pane.pane_access_t;
    dirs          : agar.gui.widget.tlist.tlist_access_t;
    files         : agar.gui.widget.tlist.tlist_access_t;
    label_cwd     : agar.gui.widget.label.label_access_t;
    file          : agar.gui.widget.textbox.textbox_access_t;
    combo_types   : agar.gui.widget.combo.combo_access_t;
    button_ok     : agar.gui.widget.button.button_access_t;
    button_cancel : agar.gui.widget.button.button_access_t;
    ok_action     : agar.core.event.event_access_t;
    cancel_action : agar.core.event.event_access_t;
    mru_dir       : cs.chars_ptr;
    opts_ctr      : agar.core.types.void_ptr_t;
    types         : filetype_tail_queue.head_t;
  end record;
  pragma convention (c, file_dialog_t);

end agar.gui.widget.file_dialog;
