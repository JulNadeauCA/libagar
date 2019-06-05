package body agar.gui.widget.mpane is

  function widget (mpane : mpane_access_t) return widget_access_t is
  begin
    return agar.gui.widget.box.widget (mpane.box'access);
  end widget;

end agar.gui.widget.mpane;
