/*	$Csoft: green2.h,v 1.1 2003/01/21 04:21:42 vedge Exp $	    */
/*	Public domain	*/

static const SDL_Color default_border[] = {
	{  13,  71,  57, 0 },
	{  16,  55,  51, 0 },
	{  25,  94,  73, 0 },
	{  32, 107,  81, 0 },
	{  77, 155, 100, 0 },
	{  32, 107,  81, 0 },
	{  25,  94,  73, 0 },
	{  16,  55,  51, 0 },
	{  13,  71,  57, 0 },
	{ 200, 200, 200, 0 },
};

static const int default_nborder = sizeof(default_border) /
    sizeof(default_border[0]) - 1;
