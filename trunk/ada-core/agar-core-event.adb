package body agar.core.event is

  use type c.int;

  function find_event_handler
    (object : agar.core.types.void_ptr_t;
     name   : cs.chars_ptr) return event_access_t;
  pragma import (c, find_event_handler, "AG_FindEventHandler");

  function reschedule_event
    (object     : agar.core.types.void_ptr_t;
     event_name : cs.chars_ptr;
     ticks      : agar.core.types.uint32_t) return c.int;
  pragma import (c, reschedule_event, "AG_ReschedEvent");

  function cancel_event
    (object     : agar.core.types.void_ptr_t;
     event_name : cs.chars_ptr) return c.int;
  pragma import (c, cancel_event, "AG_CancelEvent");

  function find_event_handler
    (object : agar.core.types.void_ptr_t;
     name   : string) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return find_event_handler
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access));
  end find_event_handler;

  function reschedule_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string; 
     ticks      : agar.core.types.uint32_t) return boolean
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
  begin
    return reschedule_event
      (object     => object,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access),
       ticks      => ticks) /= 0;
  end reschedule_event;

  function cancel_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string) return boolean
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
  begin
    return cancel_event
      (object     => object,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access)) /= 0;
  end cancel_event;

end agar.core.event;
