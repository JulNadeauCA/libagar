" Vim syntax file
" Language:     LibAgar - Agar-Math C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarmath.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_math") || exists("c_agar_math_typedefs")
  " math/m_geometry.h
  syn keyword cType M_Line2 M_Line3 M_Circle2 M_Circle3 M_Sphere M_Plane
  syn keyword cType M_Triangle2 M_Triangle3 M_Rectangle2 M_Rectangle3
  syn keyword cType M_Polygon M_Halfedge M_Facet M_Polyhedron M_GeomType
  syn keyword cType M_Geom2 M_Geom3 M_GeomSet2 M_GeomSet3
  syn keyword cType M_PointSet2 M_PointSet3 M_PointSet2i M_PointSet3i
  syn keyword cConstant M_NONE M_POINT M_LINE M_CIRCLE M_TRIANGLE M_RECTANGLE
  syn keyword cConstant M_POLYGON M_PLANE M_SPHERE M_POLYHEDRON
  " math/m_math.h
  syn keyword cType M_Real M_Time M_Range M_TimeRange M_Complex M_Quaternion
  syn keyword cType M_Vector2 M_Rectangular M_Polar M_Parabolic M_Spherical M_Cylindrical
  syn keyword cType M_Vector3 M_Vector4 M_Matrix44 M_Color
  syn keyword cType M_Vector M_Matrix M_MatrixOps M_MatrixOps44
  syn keyword cConstant M_VARIABLE_REAL M_VARIABLE_P_REAL M_VARIABLE_TIME M_VARIABLE_P_TIME
  syn keyword cConstant M_E M_LOG2E M_LOG10E M_LN2 M_LN10 M_PI M_PI_2 M_PI_4
  syn keyword cConstant M_1_PI M_2_PI M_2_SQRTPI M_SQRT2 M_SQRT1_2
  syn keyword cConstant M_EXPMIN M_EXPMAX M_PRECISION M_PRECISION_2
  syn keyword cConstant M_NUMMAX M_MACHEP M_TINYVAL M_HUGEVAL M_INFINITY
  " math/m_matview.h
  syn keyword cType M_Matview
  syn keyword cConstant M_MATVIEW_GREYSCALE M_MATVIEW_NUMERICAL
  " math/m_plotter.h
  syn keyword cType M_Plotter M_Plot M_PlotLabel
  syn keyword cConstant M_PLOTTER_NDEFCOLORS M_PLOTTER_LABEL_MAX
  syn keyword cConstant M_LABEL_X M_LABEL_Y M_LABEL_FREE M_LABEL_OVERLAY
  syn keyword cConstant M_PLOT_POINTS M_PLOT_LINEAR M_PLOT_CUBIC_SPLINE
  syn keyword cConstant M_PLOT_VECTORS M_PLOT_MANUALLY M_PLOT_FROM_VARIABLE_VFS
  syn keyword cConstant M_PLOT_FROM_REAL M_PLOT_FROM_INT M_PLOT_FROM_COMPONENT
  syn keyword cConstant M_PLOT_DERIVATIVE M_PLOT_SELECTED M_PLOT_MOUSEOVER
  syn keyword cConstant M_PLOT_DRAGGING M_PLOT_HIDDEN M_PLOT_2D M_PLOT_POLAR
  syn keyword cConstant M_PLOT_SMITH M_PLOT_3D M_PLOT_SPHERICAL M_PLOTTER_SCROLL
  syn keyword cConstant M_PLOTTER_HFILL M_PLOTTER_VFILL M_PLOTTER_EXPAND
  " math/m_point_set.h
  syn keyword cConstant M_POINT_SET_SORT_XY M_POINT_SET_SORT_YX
  syn keyword cConstant M_POINT_SET_SORT_XYZ M_POINT_SET_SORT_XZY
  syn keyword cConstant M_POINT_SET_SORT_YXZ M_POINT_SET_SORT_YZX
  syn keyword cConstant M_POINT_SET_SORT_ZXY M_POINT_SET_SORT_ZYX
  " math/m_sparse.h
  syn keyword cType M_MatrixSP
  " math/m_vector.h
  syn keyword cType M_VectorOps M_VectorOps2 M_VectorOps3 M_VectorOps4
  " math/m_vectorz.h
  syn keyword cType M_VectorZ
endif
