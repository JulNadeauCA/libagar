package body agar.core.timeout is

  use type c.int;

  function timeout_is_scheduled
    (object  : agar.core.types.void_ptr_t;
     timeout : timeout_access_t) return boolean is
  begin
    return timeout_is_scheduled
      (object  => object,
       timeout => timeout) = 1;
  end timeout_is_scheduled;

end agar.core.timeout;
