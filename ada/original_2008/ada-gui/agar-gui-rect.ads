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

  type rect2_t is record
    x1 : c.int;
    y1 : c.int;
    w  : c.int;
    h  : c.int;
    x2 : c.int;
    y2 : c.int;
  end record;
  type rect2_access_t is access all rect2_t;
  pragma convention (c, rect2_access_t);
  pragma convention (c, rect2_t);

end agar.gui.rect;
