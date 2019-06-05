package body agar.gui.widget.toolbar is

  package cbinds is
    function allocate
      (parent   : widget_access_t;
       bar_type : type_t;
       num_rows : c.int;
       flags    : flags_t) return toolbar_access_t;
    pragma import (c, allocate, "AG_ToolbarNew");

    procedure row                  
      (toolbar  : toolbar_access_t;
       row_name : c.int);
    pragma import (c, row, "AG_ToolbarRow");
  end cbinds;

  function allocate
    (parent   : widget_access_t;
     bar_type : type_t;
     num_rows : natural;
     flags    : flags_t) return toolbar_access_t is
  begin
    return cbinds.allocate
      (parent   => parent,
       bar_type => bar_type,
       num_rows => c.int (num_rows),
       flags    => flags);
  end allocate;
                                 
  procedure row                  
    (toolbar  : toolbar_access_t;
     row_name : natural) is
  begin
    cbinds.row
      (toolbar  => toolbar,
       row_name => c.int (row_name));
  end row;

  function widget (toolbar : toolbar_access_t) return widget_access_t is
  begin
    return agar.gui.widget.box.widget (toolbar.box'access);
  end widget;

end agar.gui.widget.toolbar;
