/*	$Csoft: grey10.h,v 1.3 2002/11/24 03:11:11 vedge Exp $	*/
/*	Public domain	*/

static const SDL_Color default_border[] = {
	{  50,  50,  50, 0 },
	{  90,  90, 100, 0 },
	{ 100, 100, 120, 0 },
	{ 110, 110, 125, 0 },
	{ 135, 135, 135, 0 },
	{ 135, 135, 135, 0 },
	{ 110, 110, 125, 0 },
	{ 100, 100, 120, 0 },
	{  90,  90, 100, 0 },
	{  50,  50,  50, 0 },
	{ 200, 200, 200, 0 }
};

static const int default_nborder = sizeof(default_border) /
    sizeof(default_border[0]) - 1;
