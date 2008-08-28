package body agar.core.event is

  use type c.int;

  function c_find_handler
    (object : agar.core.object.object_access_t;
     name   : cs.chars_ptr) return event_access_t;
  pragma import (c, c_find_handler, "AG_FindEventHandler");

  function c_reschedule
    (object     : agar.core.object.object_access_t;
     event_name : cs.chars_ptr;
     ticks      : agar.core.types.uint32_t) return c.int;
  pragma import (c, c_reschedule, "AG_ReschedEvent");

  function c_cancel
    (object     : agar.core.object.object_access_t;
     event_name : cs.chars_ptr) return c.int;
  pragma import (c, c_cancel, "AG_CancelEvent");

  function c_set
    (object  : agar.core.object.object_access_t;
     name    : cs.chars_ptr;
     handler : access procedure (event : event_access_t);
     fmt     : agar.core.object.object_access_t) return event_access_t;
  pragma import (c, c_set, "AG_SetEvent");

  function c_add
    (object  : agar.core.object.object_access_t;
     name    : cs.chars_ptr;
     handler : access procedure (event : event_access_t);
     fmt     : agar.core.object.object_access_t) return event_access_t;
  pragma import (c, c_add, "AG_AddEvent");

  procedure c_unset
    (object : agar.core.object.object_access_t;
     name   : cs.chars_ptr);
  pragma import (c, c_unset, "AG_UnsetEvent");

  function c_post
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     name     : cs.chars_ptr) return c.int;
  pragma import (c, c_post, "AG_PostEvent");

  function c_schedule
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     ticks    : agar.core.types.uint32_t;
     name     : cs.chars_ptr) return c.int;
  pragma import (c, c_schedule, "AG_SchedEvent");

  function set
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : access procedure (event : event_access_t)) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return c_set
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       handler => handler,
       fmt     => null);
  end set;

  function add
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : access procedure (event : event_access_t)) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return c_add
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       handler => handler,
       fmt     => null);
  end add;

  procedure unset
    (object : agar.core.object.object_access_t;
     name   : string)
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    c_unset
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access));
  end unset;

  function post
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     name     : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return c_post
      (sender   => sender,
       receiver => receiver,
       name     => cs.to_chars_ptr (ca_name'unchecked_access)) = 1;
  end post;

  function schedule
    (sender   : agar.core.object.object_access_t;
     receiver : agar.core.object.object_access_t;
     ticks    : agar.core.types.uint32_t;
     name     : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return c_schedule
      (sender   => sender,
       receiver => receiver,
       ticks    => ticks,
       name     => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end schedule;

  function find_handler
    (object : agar.core.object.object_access_t;
     name   : string) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return c_find_handler
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access));
  end find_handler;

  function reschedule
    (object     : agar.core.object.object_access_t;
     event_name : string; 
     ticks      : agar.core.types.uint32_t) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (event_name);
  begin
    return c_reschedule
      (object     => object,
       event_name => cs.to_chars_ptr (ca_name'unchecked_access),
       ticks      => ticks) /= 0;
  end reschedule;

  function cancel
    (object     : agar.core.object.object_access_t;
     event_name : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (event_name);
  begin
    return c_cancel
      (object     => object,
       event_name => cs.to_chars_ptr (ca_name'unchecked_access)) /= 0;
  end cancel;

  -- argument manipulation

  procedure c_push_pointer
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : agar.core.types.void_ptr_t);
  pragma import (c, c_push_pointer, "agar_event_push_pointer");

  procedure c_push_char
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.char);
  pragma import (c, c_push_char, "agar_event_push_char");

  procedure c_push_unsigned_char
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.unsigned_char);
  pragma import (c, c_push_unsigned_char, "agar_event_push_unsigned_char");

  procedure c_push_int
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.int);
  pragma import (c, c_push_int, "agar_event_push_int");

  procedure c_push_unsigned_int
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.unsigned);
  pragma import (c, c_push_unsigned_int, "agar_event_push_unsigned_int");

  procedure c_push_long
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.long);
  pragma import (c, c_push_long, "agar_event_push_long");

  procedure c_push_unsigned_long
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.unsigned_long);
  pragma import (c, c_push_unsigned_long, "agar_event_push_unsigned_long");

  procedure c_push_float
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.c_float);
  pragma import (c, c_push_float, "agar_event_push_float");

  procedure c_push_double
    (event : event_access_t;
     key   : cs.chars_ptr;
     value : c.double);
  pragma import (c, c_push_double, "agar_event_push_double");

  procedure push_pointer
    (event : event_access_t;
     key   : string;
     value : agar.core.types.void_ptr_t)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_pointer (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_pointer;

  procedure push_char
    (event : event_access_t;
     key   : string;
     value : c.char)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_char (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_char;

  procedure push_unsigned_char
    (event : event_access_t;
     key   : string;
     value : c.unsigned_char)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_unsigned_char (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_char;

  procedure push_int
    (event : event_access_t;
     key   : string;
     value : c.int)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_int (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_int;

  procedure push_unsigned_int
    (event : event_access_t;
     key   : string;
     value : c.unsigned)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_unsigned_int (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_int;

  procedure push_long
    (event : event_access_t;
     key   : string;
     value : c.long)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_long (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_long;

  procedure push_unsigned_long
    (event : event_access_t;
     key   : string;
     value : c.unsigned_long)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_unsigned_long (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_long;

  procedure push_float
    (event : event_access_t;
     key   : string;
     value : float)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_float (event, cs.to_chars_ptr (ca_key'unchecked_access), c.c_float (value));
  end push_float;

  procedure push_long_float
    (event : event_access_t;
     key   : string;
     value : long_float)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    c_push_double (event, cs.to_chars_ptr (ca_key'unchecked_access), c.double (value));
  end push_long_float;

end agar.core.event;
