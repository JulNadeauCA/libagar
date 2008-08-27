package agar.core.error is

  function get return string;
  pragma inline (get);

  procedure set (str : string);
  pragma inline (set);

  procedure fatal (str : string);
  pragma inline (fatal);

end agar.core.error;
