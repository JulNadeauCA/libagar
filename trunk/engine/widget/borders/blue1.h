/*	$Csoft: green1.h,v 1.4 2003/01/20 12:08:16 vedge Exp $	    */
/*	Public domain	*/

static const SDL_Color default_border[] = {
	{  50, 60, 73, 0 },
	{  55, 65, 79, 0 },
	{  70, 80, 93, 0 },
	{  55, 65, 79, 0 },
	{  50, 60, 73, 0 },
	{ 200, 200, 200, 0 },
};

static const int default_nborder = sizeof(default_border) /
    sizeof(default_border[0]) - 1;

