
/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverSDLGL *sgl, int wView, int hView)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		sgl->clipStates[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((sgl->clipRects = AG_TryMalloc(sizeof(AG_ClipRect))) == NULL)
		return (-1);

	cr = &sgl->clipRects[0];
	cr->r = AG_RECT(0, 0, wView, hView);

	cr->eqns[0][0] = 1.0;	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;	cr->eqns[0][3] = 0.0;
	cr->eqns[1][0] = 0.0;	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;	cr->eqns[1][3] = 0.0;
	cr->eqns[2][0] = -1.0;	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;	cr->eqns[2][3] = (double)wView;
	cr->eqns[3][0] = 0.0;	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;	cr->eqns[3][3] = (double)hView;
	
	sgl->nClipRects = 1;
	return (0);
}

static void
BeginRendering(void *drv)
{
	AG_DriverSDLGL *sgl = drv;
	Uint8 bR, bG, bB;

	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	} else {
		glPushAttrib(GL_VIEWPORT_BIT|GL_TRANSFORM_BIT|
		             GL_LIGHTING_BIT|GL_ENABLE_BIT);
		InitGLContext();
	}
}

static void
RenderWindow(void *drv, AG_Window *win)
{
	AG_WidgetDraw(win);
}

static void
EndRendering(void *drv)
{
	AG_DriverSDLGL *sgl = drv;
	Uint8 bR, bG, bB;

	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		SDL_GL_SwapBuffers();
		if (glx->clipStates[0])	{ glEnable(GL_CLIP_PLANE0); }
		else			{ glDisable(GL_CLIP_PLANE0); }
		if (glx->clipStates[1])	{ glEnable(GL_CLIP_PLANE1); }
		else			{ glDisable(GL_CLIP_PLANE1); }
		if (glx->clipStates[2])	{ glEnable(GL_CLIP_PLANE2); }
		else			{ glDisable(GL_CLIP_PLANE2); }
		if (glx->clipStates[3])	{ glEnable(GL_CLIP_PLANE3); }
		else			{ glDisable(GL_CLIP_PLANE3); }
	} else {
		glPopAttrib();
	}
}
