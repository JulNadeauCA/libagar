/*	$Csoft: green1.h,v 1.4 2003/01/20 12:08:16 vedge Exp $	    */
/*	Public domain	*/

static const SDL_Color default_border[] = {
	{  13,  91,  47, 0 },
	{  16,  75,  41, 0 },
	{  25, 114,  63, 0 },
	{  32, 127,  71, 0 },
	{  77, 165, 115, 0 },
	{  32, 127,  71, 0 },
	{  25, 114,  63, 0 },
	{  16,  75,  41, 0 },
	{  13,  91,  47, 0 },
	{ 200, 200, 200, 0 },
};

static const int default_nborder = sizeof(default_border) /
    sizeof(default_border[0]) - 1;
