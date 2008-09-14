package body agar.gui.widget.notebook is

  package cbinds is
    procedure set_padding
      (notebook : notebook_access_t;
       padding  : c.int);
    pragma import (c, set_padding, "AG_NotebookSetPadding");
 
    procedure set_spacing
      (notebook : notebook_access_t;
       spacing  : c.int);
    pragma import (c, set_spacing, "AG_NotebookSetSpacing");

    procedure set_tab_visibility
      (notebook : notebook_access_t;
       flag     : c.int);
    pragma import (c, set_tab_visibility, "AG_NotebookSetTabVisibility");

    procedure add_tab
      (notebook : notebook_access_t;
       name     : cs.chars_ptr;
       box_type : agar.gui.widget.box.type_t);
    pragma import (c, add_tab, "AG_NotebookAddTab");
  end cbinds;

  procedure set_padding
    (notebook : notebook_access_t;
     padding  : natural) is
  begin
    cbinds.set_padding
      (notebook => notebook,
       padding  => c.int (padding));
  end set_padding;

  procedure set_spacing
    (notebook : notebook_access_t;
     spacing  : natural) is
  begin
    cbinds.set_spacing
      (notebook => notebook,
       spacing  => c.int (spacing));
  end set_spacing;

  procedure set_tab_visibility
    (notebook : notebook_access_t;
     flag     : boolean := false)
  is
    c_flag : c.int := 0;
  begin
    if flag then c_flag := 1; end if;
    cbinds.set_tab_visibility
      (notebook => notebook,
       flag     => c_flag);
  end set_tab_visibility;

  procedure add_tab
    (notebook : notebook_access_t;
     name     : string;
     box_type : agar.gui.widget.box.type_t)
  is
    c_name : aliased c.char_array := c.to_c (name);
  begin
    cbinds.add_tab
      (notebook => notebook,
       name     => cs.to_chars_ptr (c_name'unchecked_access),
       box_type => box_type);
  end add_tab;
 
  function widget (notebook : notebook_access_t) return widget_access_t is
  begin
    return notebook.widget'access;
  end widget;

end agar.gui.widget.notebook;
