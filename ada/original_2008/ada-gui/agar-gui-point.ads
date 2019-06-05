package agar.gui.point is

  type point_t is record
    x : c.int;
    y : c.int;
  end record;
  type point_access_t is access all point_t;
  pragma convention (c, point_t);
  pragma convention (c, point_access_t);

end agar.gui.point;
