with agar.core.tail_queue;
with agar.core.types;

package agar.core.timeout is

  type timeout_t;
  type timeout_access_t is access all timeout_t;
  pragma convention (c, timeout_access_t);

  -- constants
  cancel_ondetach : constant := 16#01#;
  cancel_onload   : constant := 16#02#;

  -- queue
  package timeout_queue is new agar.core.tail_queue
    (entry_type => timeout_access_t);

  type callback_t is access function
    (ptr  : agar.core.types.void_ptr_t;
     ival : agar.core.types.uint32_t;
     arg  : agar.core.types.void_ptr_t) return agar.core.types.uint32_t;
  pragma convention (c, callback_t);

  type timeout_t is record
    func     : callback_t;
    arg      : agar.core.types.void_ptr_t;
    running  : c.int;
    flags    : c.unsigned;
    ticks    : agar.core.types.uint32_t;
    ival     : agar.core.types.uint32_t;
    timeouts : timeout_queue.entry_t;
  end record;
  pragma convention (c, timeout_t);

  procedure set_timeout
    (timeout : timeout_access_t;
     func    : callback_t;
     arg     : agar.core.types.void_ptr_t;
     flags   : c.int);
  pragma import (c, set_timeout, "AG_SetTimeout");

  procedure schedule_timeout
    (object  : agar.core.types.void_ptr_t;
     timeout : timeout_access_t;
     ival    : agar.core.types.uint32_t);
  pragma import (c, schedule_timeout, "AG_ScheduleTimeout");

  procedure delete_timeout
    (object   : agar.core.types.void_ptr_t;
     timeout  : timeout_access_t);
  pragma import (c, delete_timeout, "AG_DelTimeout");

  function timeout_is_scheduled
    (object  : agar.core.types.void_ptr_t;
     timeout : timeout_access_t) return boolean;
  pragma inline (timeout_is_scheduled);
 
  procedure lock_timeouts (object : agar.core.types.void_ptr_t);
  pragma import (c, lock_timeouts, "AG_LockTimeouts");

  procedure unlock_timeouts (object : agar.core.types.void_ptr_t);
  pragma import (c, unlock_timeouts, "AG_UnlockTimeouts");
 
end agar.core.timeout;
