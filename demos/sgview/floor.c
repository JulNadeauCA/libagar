/*	Public domain */
/* 
 * Sample 'floor' object for sgview demo.
 */

#include <agar/core/core.h>
#include <agar/sg.h>

static SDL_Surface *bmp = NULL;
static GLint texture = 0;

static void
FloorDraw(SG *sg, void *pNode)
{
	SG_Node *node = pNode;
	static GLfloat material[4] = { 0.6, 0.6, 0.6, 1.0 };
	SG_Real x, z;
	SG_Real area = 50.0, tilesz = 1.0;
	int flag = 0;
	SG_Vector p1, p2, p3;

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material);

	p1.x = -area;
	p1.y = 0.0;
	p1.z = -area;
	p2.x = area;
	p2.y = 0.0;
	p2.z = 0.0;
	p3.x = area;
	p3.y = 0.0;
	p3.z = area;
	SG_Begin(SG_QUADS);
	{
		glTexCoord2i(0, 0);
		glVertex3d(-area, 0.0, -area);
	
		glTexCoord2i(1, 0);
		glVertex3d(-area, 0.0, area);
		
		glTexCoord2i(1, 1);
		glVertex3d(area, 0.0, area);
		
		glTexCoord2i(0, 1);
		glVertex3d(area, 0.0, -area);
	}
	SG_End();
}

SG_NodeOps FloorOps = {
	"Test Floor",
	sizeof(SG_Node),
	0,
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
	NULL,		/* menuInstance */
	NULL,		/* menuClass */
	FloorDraw
};
