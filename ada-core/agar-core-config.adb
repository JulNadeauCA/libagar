with Agar.Core.Thin;
with Interfaces.C;

package body Agar.Core.Config is
  package C renames Interfaces.C;

  use type C.int;

  function Load return Boolean is
  begin
    return Thin.Config.Load = 0;
  end Load;

  function Save return Boolean is
  begin
    return Thin.Config.Save = 0;
  end Save;

end Agar.Core.Config;
