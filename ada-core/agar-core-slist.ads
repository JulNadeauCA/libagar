generic
  type entry_type is private;

package agar.core.slist is

  type head_t is record
    first : entry_type;
  end record;
  pragma convention (c, head_t);

  type entry_t is record
    next : entry_type;
  end record;
  pragma convention (c, entry_t);

end agar.core.slist;
