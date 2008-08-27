with agar.core.types;
with agar.core.timeout;
with agar.core.tail_queue;

package agar.core.event is

  type event_t;
  type event_access_t is access all event_t;
  pragma convention (c, event_access_t);

  -- queues
  package event_queue is new agar.core.tail_queue
    (entry_type => event_access_t);

  -- constants
  event_args_max : constant := 16;
  event_name_max : constant := 32;

  -- flags
  event_async     : constant := 16#01#;
  event_propogate : constant := 16#02#;
  event_scheduled : constant := 16#04#;

  type arg_type_t is (
    EVARG_POINTER,
    EVARG_STRING,
    EVARG_CHAR,
    EVARG_UCHAR,
    EVARG_INT,
    EVARG_UINT,
    EVARG_LONG,
    EVARG_ULONG,
    EVARG_FLOAT,
    EVARG_DOUBLE
  );
  for arg_type_t use (
    EVARG_POINTER => 0,
    EVARG_STRING  => 1,
    EVARG_CHAR    => 2,
    EVARG_UCHAR   => 3,
    EVARG_INT     => 4,
    EVARG_UINT    => 5,
    EVARG_LONG    => 6,
    EVARG_ULONG   => 7,
    EVARG_FLOAT   => 8,
    EVARG_DOUBLE  => 9
  );
  for arg_type_t'size use c.unsigned'size;
  pragma convention (c, arg_type_t);

  type arg_t (ev_type : arg_type_t := EVARG_POINTER) is record
    case ev_type is
      when evarg_pointer => p   : agar.core.types.void_ptr_t;
      when evarg_string  => s   : cs.chars_ptr;
      when evarg_char    => ch  : c.char;
      when evarg_uchar   => uch : c.char;
      when evarg_int     => i   : c.int;
      when evarg_uint    => ui  : c.unsigned;
      when evarg_long    => li  : c.long;
      when evarg_ulong   => uli : c.long;
      when evarg_float   => f   : c.double;
      when evarg_double  => df  : c.double;
    end case;
  end record;
  pragma convention (c, arg_t);
  pragma unchecked_union (arg_t);

  -- callback
  type callback_t is access procedure (ev : event_access_t);

  type event_name_t is array (1 .. event_name_max) of aliased c.char;
  pragma convention (c, event_name_t);
  type event_argv_t is array (1 .. event_args_max) of aliased arg_t;
  pragma convention (c, event_argv_t);
  type event_argt_t is array (1 .. event_args_max) of aliased c.int;
  pragma convention (c, event_argt_t);
  type event_argn_t is array (1 .. event_args_max) of aliased cs.chars_ptr;
  pragma convention (c, event_argn_t);

  type event_t is record
    name    : event_name_t;
    flags   : c.unsigned;
    handler : callback_t;
    argc    : c.int;
    argc0   : c.int;
    argv    : event_argv_t;
    argt    : event_argt_t;
    argn    : event_argn_t;
    timeout : agar.core.timeout.timeout_t;
    events  : event_queue.entry_t;    
  end record;
  pragma convention (c, event_t);

  type func_t is access procedure (event : event_access_t);
  pragma convention (c, func_t);

  --
  -- API
  --

  function find_event_handler
    (object : agar.core.types.void_ptr_t;
     name   : string) return event_access_t;
  pragma inline (find_event_handler);

  function reschedule_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string; 
     ticks      : agar.core.types.uint32_t) return boolean;
  pragma inline (reschedule_event);

  function cancel_event
    (object     : agar.core.types.void_ptr_t;
     event_name : string) return boolean;
  pragma inline (cancel_event);

  procedure forward_event
    (sender     : agar.core.types.void_ptr_t;
     receiver   : agar.core.types.void_ptr_t;
     event      : event_access_t);
  pragma import (c, forward_event, "AG_ForwardEvent");

end agar.core.event;
