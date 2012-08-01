# ex:syn=make
GUI_LIBS=		${SDL_LIBS} ${OPENGL_LIBS} ${FREETYPE_LIBS}
GUI_LINKS=		ag_gui ${GUI_LIBS}
GUI_LINKS_STATIC=	ag_gui_static ${GUI_LIBS}
