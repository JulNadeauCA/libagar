package body agar.core.event is

  use type agar.core.event_types.arg_type_t;
  use type c.int;

  package cbinds is
    function find_handler
      (object : agar.core.object.object_access_t;
       name   : cs.chars_ptr) return event_access_t;
    pragma import (c, find_handler, "AG_FindEventHandler");
  
    function reschedule
      (object     : agar.core.object.object_access_t;
       event_name : cs.chars_ptr;
       ticks      : agar.core.types.uint32_t) return c.int;
    pragma import (c, reschedule, "AG_ReschedEvent");
  
    function cancel
      (object     : agar.core.object.object_access_t;
       event_name : cs.chars_ptr) return c.int;
    pragma import (c, cancel, "AG_CancelEvent");
  
    function set
      (object  : agar.core.object.object_access_t;
       name    : cs.chars_ptr;
       handler : access procedure (event : event_access_t);
       fmt     : agar.core.object.object_access_t) return event_access_t;
    procedure set
      (object  : agar.core.object.object_access_t;
       name    : cs.chars_ptr;
       handler : access procedure (event : event_access_t);
       fmt     : agar.core.object.object_access_t);
    pragma import (c, set, "AG_SetEvent");
  
    function add
      (object  : agar.core.object.object_access_t;
       name    : cs.chars_ptr;
       handler : access procedure (event : event_access_t);
       fmt     : agar.core.object.object_access_t) return event_access_t;
    procedure add
      (object  : agar.core.object.object_access_t;
       name    : cs.chars_ptr;
       handler : access procedure (event : event_access_t);
       fmt     : agar.core.object.object_access_t);
    pragma import (c, add, "AG_AddEvent");
  
    procedure unset
      (object : agar.core.object.object_access_t;
       name   : cs.chars_ptr);
    pragma import (c, unset, "AG_UnsetEvent");
  
    function post
      (sender   : agar.core.object.object_access_t;
       receiver : agar.core.object.object_access_t;
       name     : cs.chars_ptr) return c.int;
    pragma import (c, post, "AG_PostEvent");
  
    function schedule
      (sender   : agar.core.object.object_access_t;
       receiver : agar.core.object.object_access_t;
       ticks    : agar.core.types.uint32_t;
       name     : cs.chars_ptr) return c.int;
    pragma import (c, schedule, "AG_SchedEvent");
  
    procedure push_pointer
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : agar.core.types.void_ptr_t);
    pragma import (c, push_pointer, "agar_event_push_pointer");
  
    procedure push_char
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.char);
    pragma import (c, push_char, "agar_event_push_char");
  
    procedure push_unsigned_char
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.unsigned_char);
    pragma import (c, push_unsigned_char, "agar_event_push_unsigned_char");
  
    procedure push_int
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.int);
    pragma import (c, push_int, "agar_event_push_int");
  
    procedure push_unsigned_int
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.unsigned);
    pragma import (c, push_unsigned_int, "agar_event_push_unsigned_int");
  
    procedure push_long
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.long);
    pragma import (c, push_long, "agar_event_push_long");
  
    procedure push_unsigned_long
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.unsigned_long);
    pragma import (c, push_unsigned_long, "agar_event_push_unsigned_long");
  
    procedure push_float
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.c_float);
    pragma import (c, push_float, "agar_event_push_float");
  
    procedure push_double
      (event : event_access_t;
       key   : cs.chars_ptr;
       value : c.double);
    pragma import (c, push_double, "agar_event_push_double");
  end cbinds;

  function set
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.set
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       handler => handler,
       fmt     => null);
  end set;

  procedure set
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t)
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    cbinds.set
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       handler => handler,
       fmt     => null);
  end set;

  function add
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t) return event_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.add
      (object  => object,
       name    => cs.to_chars_ptr (ca_name'unchecked_access),
       handler => handler,
       fmt     => null);
  end add;

  procedure add
    (object  : agar.core.object.object_access_t;
     name    : string;
     handler : callback_t)
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    cbinds.add
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
    cbinds.unset
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
    return cbinds.post
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
    return cbinds.schedule
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
    return cbinds.find_handler
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
    return cbinds.reschedule
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
    return cbinds.cancel
      (object     => object,
       event_name => cs.to_chars_ptr (ca_name'unchecked_access)) /= 0;
  end cancel;

  -- argument manipulation

  procedure push_pointer
    (event : event_access_t;
     key   : string;
     value : agar.core.types.void_ptr_t)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_pointer (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_pointer;

  procedure push_char
    (event : event_access_t;
     key   : string;
     value : c.char)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_char (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_char;

  procedure push_unsigned_char
    (event : event_access_t;
     key   : string;
     value : c.unsigned_char)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_unsigned_char (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_char;

  procedure push_int
    (event : event_access_t;
     key   : string;
     value : c.int)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_int (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_int;

  procedure push_unsigned_int
    (event : event_access_t;
     key   : string;
     value : c.unsigned)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_unsigned_int (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_int;

  procedure push_long
    (event : event_access_t;
     key   : string;
     value : c.long)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_long (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_long;

  procedure push_unsigned_long
    (event : event_access_t;
     key   : string;
     value : c.unsigned_long)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_unsigned_long (event, cs.to_chars_ptr (ca_key'unchecked_access), value);
  end push_unsigned_long;

  procedure push_float
    (event : event_access_t;
     key   : string;
     value : float)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_float (event, cs.to_chars_ptr (ca_key'unchecked_access), c.c_float (value));
  end push_float;

  procedure push_long_float
    (event : event_access_t;
     key   : string;
     value : long_float)
  is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    cbinds.push_double (event, cs.to_chars_ptr (ca_key'unchecked_access), c.double (value));
  end push_long_float;

  --

  -- event argument type and index checking
  procedure check
    (event         : event_access_t;
     index         : positive;
     expected_type : agar.core.event_types.arg_type_t)
  is
    procedure check_bounds
      (event : event_access_t;
       index : positive) is
    begin
      if index > positive (event.argc) then
        raise constraint_error with "invalid index for event argument list";
      end if;
    end check_bounds;

    procedure check_type
      (event         : event_access_t;
       index         : positive;
       expected_type : agar.core.event_types.arg_type_t) is
    begin
      if event.argt (index) /= expected_type then
        raise constraint_error with "invalid type for event argument index";
      end if;
    end check_type;
  begin
    check_bounds (event, index + 1);
    check_type   (event, index + 1, expected_type);
  end check;

  function get_pointer
    (event : event_access_t;
     index : positive) return agar.core.types.void_ptr_t is
  begin
    check (event, index, agar.core.event_types.EVARG_POINTER);
    return event.argv (index + 1).p;
  end get_pointer;

  function get_char
    (event : event_access_t;
     index : positive) return c.char is
  begin
    check (event, index, agar.core.event_types.EVARG_CHAR);
    return event.argv (index + 1).ch;
  end get_char;

  function get_character
    (event : event_access_t;
     index : positive) return character is
  begin
    check (event, index, agar.core.event_types.EVARG_CHAR);
    return character (event.argv (index + 1).ch);
  end get_character;

  function get_unsigned_char
    (event : event_access_t;
     index : positive) return c.unsigned_char is
  begin
    check (event, index, agar.core.event_types.EVARG_UCHAR);
    return c.unsigned_char (event.argv (index + 1).uch);
  end get_unsigned_char;

  function get_int
    (event : event_access_t;
     index : positive) return c.int is
  begin
    check (event, index, agar.core.event_types.EVARG_INT);
    return event.argv (index + 1).i;
  end get_int;

  function get_integer
    (event : event_access_t;
     index : positive) return integer is
  begin
    check (event, index, agar.core.event_types.EVARG_INT);
    return integer (event.argv (index + 1).i);
  end get_integer;

  function get_unsigned_int
    (event : event_access_t;
     index : positive) return c.unsigned is
  begin
    check (event, index, agar.core.event_types.EVARG_UINT);
    return event.argv (index + 1).ui;
  end get_unsigned_int;

  function get_natural
    (event : event_access_t;
     index : positive) return natural is
  begin
    check (event, index, agar.core.event_types.EVARG_UINT);
    return natural (event.argv (index + 1).ui);
  end get_natural;

  function get_long
    (event : event_access_t;
     index : positive) return c.long is
  begin
    check (event, index, agar.core.event_types.EVARG_LONG);
    return event.argv (index + 1).li;
  end get_long;

  function get_long_integer
    (event : event_access_t;
     index : positive) return long_integer is
  begin
    check (event, index, agar.core.event_types.EVARG_LONG);
    return long_integer (event.argv (index + 1).li);
  end get_long_integer;

  function get_unsigned_long
    (event : event_access_t;
     index : positive) return c.unsigned_long is
  begin
    check (event, index, agar.core.event_types.EVARG_ULONG);
    return c.unsigned_long (event.argv (index + 1).uli);
  end get_unsigned_long;

  function get_float
    (event : event_access_t;
     index : positive) return float is
  begin
    check (event, index, agar.core.event_types.EVARG_FLOAT);
    return float (event.argv (index + 1).f);
  end get_float;

  function get_long_float
    (event : event_access_t;
     index : positive) return long_float is
  begin
    check (event, index, agar.core.event_types.EVARG_DOUBLE);
    return long_float (event.argv (index + 1).df);
  end get_long_float;

end agar.core.event;
