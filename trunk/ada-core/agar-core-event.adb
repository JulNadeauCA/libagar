package body agar.core.event is

  use type c.int;

  function set_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string;
     handler    : func_t;
     str        : string) return event_access_t
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
    ca_str        : aliased c.char_array := c.to_c (str);
  begin
    return set_event
      (object     => object,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access),
       handler    => handler,
       str        => cs.to_chars_ptr (ca_str'unchecked_access));
  end set_event;

  function add_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string;
     handler    : func_t;
     str        : string) return event_access_t
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
    ca_str        : aliased c.char_array := c.to_c (str);
  begin
    return add_event
      (object     => object,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access),
       handler    => handler,
       str        => cs.to_chars_ptr (ca_str'unchecked_access));
  end add_event;

  procedure unset_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string)
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
  begin
    unset_event
      (object     => object,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access));
  end unset_event;

  procedure post_event
    (sender     : agar.core.types.void_ptr_t;
     receiver   : agar.core.types.void_ptr_t;
     event_name : string;
     fmt        : string)
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
    ca_fmt        : aliased c.char_array := c.to_c (fmt);
  begin
    post_event
      (sender     => sender,
       receiver   => receiver,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access),
       fmt        => cs.to_chars_ptr (ca_fmt'unchecked_access));
  end post_event;

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

  function schedule_event
    (sender     : agar.core.types.void_ptr_t;
     receiver   : agar.core.types.void_ptr_t;
     ticks      : agar.core.types.uint32_t;
     event_name : string;
     fmt        : string) return boolean 
  is
    ca_event_name : aliased c.char_array := c.to_c (event_name);
    ca_fmt        : aliased c.char_array := c.to_c (fmt);
  begin
    return schedule_event
      (sender     => sender,
       receiver   => receiver,
       ticks      => ticks,
       event_name => cs.to_chars_ptr (ca_event_name'unchecked_access),
       fmt        => cs.to_chars_ptr (ca_fmt'unchecked_access)) = 0;
  end schedule_event;

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
