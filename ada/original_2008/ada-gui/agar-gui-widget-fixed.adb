package body agar.gui.widget.fixed is

  package cbinds is
    procedure put
      (fixed : fixed_access_t;
       child : child_access_t;
       x     : c.int;
       y     : c.int);
    pragma import (c, put, "AG_FixedPut");
  
    procedure size
      (fixed  : fixed_access_t;
       child  : child_access_t;
       width  : c.int;
       height : c.int);
    pragma import (c, size, "AG_FixedSize");
  
    procedure move
      (fixed : fixed_access_t;
       child : child_access_t;
       x     : c.int;
       y     : c.int);
    pragma import (c, move, "AG_FixedMove");
  end cbinds;

  procedure put
    (fixed : fixed_access_t;
     child : child_access_t;
     x     : natural;
     y     : natural) is
  begin
    cbinds.put
      (fixed => fixed,
       child => child,
       x     => c.int (x),
       y     => c.int (y));
  end put;

  procedure size
    (fixed  : fixed_access_t;
     child  : child_access_t;
     width  : positive;
     height : positive) is
  begin
    cbinds.size
      (fixed  => fixed,
       child  => child,
       width  => c.int (width),
       height => c.int (height));
  end size;

  procedure move
    (fixed : fixed_access_t;
     child : child_access_t;
     x     : natural;
     y     : natural) is
  begin
    cbinds.move
      (fixed => fixed,
       child => child,
       x     => c.int (x),
       y     => c.int (y));
  end move;

  function widget (fixed : fixed_access_t) return widget_access_t is
  begin
    return fixed.widget'unchecked_access;
  end widget;

end agar.gui.widget.fixed;
