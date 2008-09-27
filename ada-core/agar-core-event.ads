with agar.core.event_types;
with agar.core.object;
with agar.core.types;

package agar.core.event is

  subtype event_t is agar.core.event_types.event_t;
  subtype event_access_t is agar.core.event_types.event_access_t;

  -- constants
  event_args_max : constant := agar.core.event_types.event_args_max;
  event_name_max : constant := agar.core.event_types.event_name_max;

  -- flags
  event_async     : constant := 16#01#;
  event_propogate : constant := 16#02#;
  event_scheduled : constant := 16#04#;

  -- callback
  subtype callback_t is agar.core.event_types.callback_t;

  --
  -- API
  --

  -- event processing

  function set
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t) return event_access_t;
  pragma inline (set);

  procedure set
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t);
  pragma inline (set);

  function add
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t) return event_access_t;
  pragma inline (add);

  procedure add
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t);
  pragma inline (add);

  procedure unset
    (object : agar.core.object.object_access_t;
     name   : string);
  pragma inline (unset);

  function post
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     name     : string) return boolean;
  pragma inline (post);

  function schedule
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     ticks    : agar.core.types.uint32_t;
     name     : string) return boolean;
  pragma inline (schedule);

  function find_handler
    (object : agar.core.object.object_access_t;
     name   : string) return event_access_t;
  pragma inline (find_handler);

  function reschedule
    (object     : agar.core.object.object_access_t;
     event_name : string; 
     ticks      : agar.core.types.uint32_t) return boolean;
  pragma inline (reschedule);

  function cancel
    (object     : agar.core.object.object_access_t;
     event_name : string) return boolean;
  pragma inline (cancel);

  procedure forward
    (sender     : agar.core.object.object_access_t;
     receiver   : agar.core.object.object_access_t;
     event      : event_access_t);
  pragma import (c, forward, "AG_ForwardEvent");

  -- argument manipulation

  procedure push_pointer
    (event : event_access_t;
     key   : string;
     value : agar.core.types.void_ptr_t);
  pragma inline (push_pointer);

  procedure push_char
    (event : event_access_t;
     key   : string;
     value : c.char);
  pragma inline (push_char);

  procedure push_unsigned_char
    (event : event_access_t;
     key   : string;
     value : c.unsigned_char);
  pragma inline (push_unsigned_char);

  procedure push_int
    (event : event_access_t;
     key   : string;
     value : c.int);
  pragma inline (push_int);

  procedure push_unsigned_int
    (event : event_access_t;
     key   : string;
     value : c.unsigned);
  pragma inline (push_unsigned_int);

  procedure push_long
    (event : event_access_t;
     key   : string;
     value : c.long);
  pragma inline (push_long);

  procedure push_unsigned_long
    (event : event_access_t;
     key   : string;
     value : c.unsigned_long);
  pragma inline (push_unsigned_long);

  procedure push_float
    (event : event_access_t;
     key   : string;
     value : float);
  pragma inline (push_float);

  procedure push_long_float
    (event : event_access_t;
     key   : string;
     value : long_float);
  pragma inline (push_long_float);

  -- argument retrieval

  function get_pointer
    (event : event_access_t;
     index : positive) return agar.core.types.void_ptr_t;
  pragma inline (get_pointer);

  function get_char
    (event : event_access_t;
     index : positive) return c.char;
  pragma inline (get_char);

  function get_character
    (event : event_access_t;
     index : positive) return character;
  pragma inline (get_character);

  function get_unsigned_char
    (event : event_access_t;
     index : positive) return c.unsigned_char;
  pragma inline (get_unsigned_char);

  function get_int
    (event : event_access_t;
     index : positive) return c.int;
  pragma inline (get_int);

  function get_integer
    (event : event_access_t;
     index : positive) return integer;
  pragma inline (get_integer);

  function get_unsigned_int
    (event : event_access_t;
     index : positive) return c.unsigned;
  pragma inline (get_unsigned_int);

  function get_natural
    (event : event_access_t;
     index : positive) return natural;
  pragma inline (get_natural);

  function get_long
    (event : event_access_t;
     index : positive) return c.long;
  pragma inline (get_long);

  function get_long_integer
    (event : event_access_t;
     index : positive) return long_integer;
  pragma inline (get_long_integer);

  function get_unsigned_long
    (event : event_access_t;
     index : positive) return c.unsigned_long;
  pragma inline (get_unsigned_long);

  function get_float
    (event : event_access_t;
     index : positive) return float;
  pragma inline (get_float);

  function get_long_float
    (event : event_access_t;
     index : positive) return long_float;
  pragma inline (get_long_float);

end agar.core.event;
