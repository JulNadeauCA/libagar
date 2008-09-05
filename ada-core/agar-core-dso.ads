with agar.core.limits;
with agar.core.tail_queue;
with agar.core.types;
with agar.core;

package agar.core.dso is

  dso_name_max : constant := 128;

  type sym_t;
  type sym_access_t is access all sym_t;
  pragma convention (c, sym_access_t);

  type dso_t;
  type dso_access_t is access all dso_t;
  pragma convention (c, dso_access_t);

  package sym_tail_queue is new agar.core.tail_queue
    (entry_type => sym_access_t);
  package dso_tail_queue is new agar.core.tail_queue
    (entry_type => dso_access_t);

  type sym_t is record
    sym  : cs.chars_ptr;
    ptr  : agar.core.types.void_ptr_t;
    syms : sym_tail_queue.entry_t;
  end record;
  pragma convention (c, sym_t);

  type name_t is array (1 .. dso_name_max) of aliased c.char;
  pragma convention (c, name_t);
  type path_t is array (1 .. agar.core.limits.pathname_max) of aliased c.char;
  pragma convention (c, path_t);

  type dso_t is record
    name      : name_t;
    path      : path_t;
    ref_count : c.unsigned;
    flags     : c.unsigned;
    syms      : sym_tail_queue.head_t;
    dsos      : dso_tail_queue.entry_t;
  end record;
  pragma convention (c, dso_t);

  --
  -- API
  --

  function load
    (name  : string;
     path  : string;
     flags : c.unsigned) return dso_access_t;
  pragma inline (load);

  function symbol
    (dso         : dso_access_t;
     symbol_name : string;
     value       : agar.core.types.void_ptr_t) return boolean;
  pragma inline (symbol);
 
  function unload (dso : dso_access_t) return boolean;
  pragma inline (unload);

  procedure lock;
  pragma import (c, lock, "agar_dso_lock");

  procedure unlock;
  pragma import (c, unlock, "agar_dso_unlock");

end agar.core.dso;
