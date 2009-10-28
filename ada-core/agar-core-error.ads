package Agar.Core.Error is

  procedure Set_Error (Message : in String);

  procedure Fatal_Error (Message : in String);

  type    Error_Callback_t          is access procedure (Message : in String);
  subtype Error_Callback_Not_Null_t is not null Error_Callback_t;

  procedure Set_Fatal_Callback
    (Callback : Error_Callback_Not_Null_t);

end Agar.Core.Error;
