# ex:syn=make
MAP_LIBS=        ${OPENGL_LIBS}
MAP_CFLAGS=      ${OPENGL_CFLAGS}
MAP_LINKS=       ag_map ${MAP_LIBS}
MAP_LINKS_STATIC=ag_map_static ${MAP_LIBS}
