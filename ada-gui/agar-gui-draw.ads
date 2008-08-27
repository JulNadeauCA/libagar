with agar.core.types;
with agar.gui.widget;
with agar.gui.rect;

package agar.gui.draw is

  procedure box 
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     color  : agar.core.types.uint32_t);
  pragma import (c, box, "agar_draw_box");

  procedure box_rounded
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     z      : c.int;
     radius : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, box_rounded, "agar_draw_box_rounded"); 

  procedure box_rounded
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     z      : natural;
     radius : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (box_rounded);
 
  procedure box_rounded_top
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     z      : c.int;
     radius : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, box_rounded_top, "agar_draw_box_rounded_top"); 

  procedure box_rounded_top
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     z      : natural;
     radius : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (box_rounded_top);
 
  procedure frame
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     color  : agar.core.types.uint32_t);
  pragma import (c, frame, "agar_draw_frame");

  procedure circle
    (widget : agar.gui.widget.widget_access_t;
     x      : c.int;
     y      : c.int;
     radius : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, circle, "agar_draw_circle");

  procedure circle
    (widget : agar.gui.widget.widget_access_t;
     x      : natural;
     y      : natural;
     radius : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (circle);
 
  procedure circle2
    (widget : agar.gui.widget.widget_access_t;
     x      : c.int;
     y      : c.int;
     radius : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, circle2, "agar_draw_circle2");

  procedure circle2
    (widget : agar.gui.widget.widget_access_t;
     x      : natural;
     y      : natural;
     radius : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (circle2);
 
  procedure line
    (widget : agar.gui.widget.widget_access_t;
     x1     : c.int;
     y1     : c.int;
     x2     : c.int;
     y2     : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, line, "agar_draw_line");

  procedure line
    (widget : agar.gui.widget.widget_access_t;
     x1     : natural;
     y1     : natural;
     x2     : natural;
     y2     : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (line);

  procedure line_horizontal
    (widget : agar.gui.widget.widget_access_t;
     x1     : c.int;
     x2     : c.int;
     y      : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, line_horizontal, "agar_draw_line_horizontal"); 

  procedure line_horizontal
    (widget : agar.gui.widget.widget_access_t;
     x1     : natural;
     x2     : natural;
     y      : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (line_horizontal);
 
  procedure line_vertical
    (widget : agar.gui.widget.widget_access_t;
     x      : c.int;
     y1     : c.int;
     y2     : c.int;
     color  : agar.core.types.uint32_t);
  pragma import (c, line_vertical, "agar_draw_line_vertical"); 

  procedure line_vertical
    (widget : agar.gui.widget.widget_access_t;
     x      : natural;
     y1     : natural;
     y2     : natural;
     color  : agar.core.types.uint32_t);
  pragma inline (line_vertical);
 
  procedure rect_outline
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     color  : agar.core.types.uint32_t);
  pragma import (c, rect_outline, "agar_draw_rect_outline");

  procedure rect_filled
    (widget : agar.gui.widget.widget_access_t;
     rect   : agar.gui.rect.rect_t;
     color  : agar.core.types.uint32_t);
  pragma import (c, rect_filled, "agar_draw_rect_filled");

end agar.gui.draw;
