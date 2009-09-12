package body agar.gui.widget.graph is

  package body vertex is
    package cbinds is
      procedure label
        (vertex : vertex_access_t;
         text   : cs.chars_ptr);
      pragma import (c, label, "AG_GraphVertexLabelS");

      procedure size
        (vertex : vertex_access_t;
         width  : c.int;
         height : c.int);
      pragma import (c, size, "AG_GraphVertexSize");

      procedure position
        (vertex : vertex_access_t;
         x      : c.int;
         y      : c.int);
      pragma import (c, position, "AG_GraphVertexPosition");
    end cbinds;

    procedure label
      (vertex : vertex_access_t;
       label  : string)
    is
      c_lab : aliased c.char_array := c.to_c (label);
    begin
      cbinds.label
        (vertex => vertex,
         arg    => cs.to_chars_ptr (c_lab'unchecked_access));
    end label;

    procedure size
      (vertex : vertex_access_t;
       width  : positive;
       height : positive) is
    begin
      cbinds.size
        (vertex => vertex,
         width  => c.int (width),
         height => c.int (height));
    end size;

    procedure position
      (vertex : vertex_access_t;
       x      : natural;
       y      : natural) is
    begin
      cbinds.position
        (vertex => vertex,
         x      => c.int (x),
         y      => c.int (y));
    end position;
  end vertex;

  package body edge is
    package cbinds is
      procedure label
        (edge : edge_access_t;
         arg  : cs.chars_ptr);
      pragma import (c, label, "AG_GraphEdgeLabelS");
    end cbinds;

    procedure label
      (edge  : edge_access_t;
       label : string)
    is
      c_lab : aliased c.char_array := c.to_c (label);
    begin
      cbinds.label
        (edge => edge,
         arg  => cs.to_chars_ptr (c_lab'unchecked_access));
    end label;
  end edge;

  function widget (graph : graph_access_t) return widget_access_t is
  begin
    return graph.widget'access;
  end widget;

end agar.gui.widget.graph;
