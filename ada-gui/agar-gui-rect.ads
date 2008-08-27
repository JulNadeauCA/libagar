package agar.gui.rect is

  type rect_t is record
    x : c.int;
    y : c.int;
    w : c.int;
    h : c.int;
  end record;
  type rect_access_t is access all rect_t;
  pragma convention (c, rect_access_t);
  pragma convention (c, rect_t);

end agar.gui.rect;
