/*	$Csoft: grey8.h,v 1.4 2002/11/24 03:11:11 vedge Exp $	    */
/*	Public domain	*/

static const SDL_Color default_border[] = {
	{  50,  50,  50, 0 },
	{ 100, 100, 100, 0 },
	{ 100, 100, 100, 0 },
	{ 180, 180, 180, 0 },
	{ 180, 180, 180, 0 },
	{ 100, 100, 100, 0 },
	{ 100, 100, 100, 0 },
	{  50,  50,  50, 0 },
	{  50,  50,  50, 0 },
	{ 200, 200, 200, 0 }
};

static const int default_nborder = sizeof(default_border) /
    sizeof(default_border[0]) - 1;
