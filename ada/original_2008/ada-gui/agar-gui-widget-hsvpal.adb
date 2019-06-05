package body agar.gui.widget.hsvpal is

  function widget (hsvpal : hsvpal_access_t) return widget_access_t is
  begin
    return hsvpal.widget'access;
  end widget;

end agar.gui.widget.hsvpal;
