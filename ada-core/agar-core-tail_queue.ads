generic
  type entry_type is private;

package agar.core.tail_queue is

  type head_t is record
    first : entry_type;
    last  : access entry_type;
  end record;
  pragma convention (c, head_t);

  type entry_t is record
    next : entry_type;
    prev : access entry_type;
  end record;
  pragma convention (c, entry_t);

end agar.core.tail_queue;
