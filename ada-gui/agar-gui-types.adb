package body agar.gui.types is

  function widget_icon_widget (icon : widget_icon_access_t)
    return agar.gui.widget.widget_access_t is
  begin
    return icon.widget'access;
  end widget_icon_widget;

  function window_widget (window : window_access_t)
    return agar.gui.widget.widget_access_t is
  begin
    return window.widget'access;
  end window_widget;

  function widget_titlebar_widget (titlebar : widget_titlebar_access_t)
    return agar.gui.widget.widget_access_t is
  begin
    return agar.gui.widget.box.widget (titlebar.box'access);
  end widget_titlebar_widget;

end agar.gui.types;
