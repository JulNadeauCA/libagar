with agar.core.types;

package agar.core.threads is

  -- thread types
  type mutex_t is new agar.core.types.void_ptr_t;
  type mutex_attr_t is new agar.core.types.void_ptr_t;
  type thread_t is new agar.core.types.void_ptr_t;
  type cond_t is new agar.core.types.void_ptr_t;
  type key_t is new agar.core.types.void_ptr_t;

end agar.core.threads;
