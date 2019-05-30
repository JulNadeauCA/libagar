/*
 * Fancy hello world program using cc65.
 *
 * Ullrich von Bassewitz (ullrich@von-bassewitz.de)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dbg.h>

#include <agar/core.h>


/*****************************************************************************/
/*     	      	    	  	     Data	     			     */
/*****************************************************************************/

static const char WebsiteURL [] = "http://libagar.org";
static const char HelloText [] = "Hello world!";

/*****************************************************************************/
/*     	      	    	  	     Code	     			     */
/*****************************************************************************/



int main (void)
{
    unsigned char XSize, YSize;
    AG_AgarVersion v;

    AG_GetVersion(&v);

    /* Set screen colors, hide the cursor */
    textcolor (COLOR_WHITE);
    bordercolor (COLOR_BLACK);
    bgcolor (COLOR_BLACK);
    cursor (0);

    /* Clear the screen, put cursor in upper left corner */
    clrscr ();

    /* Ask for the screen size */
    screensize (&XSize, &YSize);

    /* Draw a border around the screen */

    /* Top line */
    cputc (CH_ULCORNER);
    chline (XSize - 2);
    cputc (CH_URCORNER);

    /* Vertical line, left side */
    cvlinexy (0, 1, YSize - 2);

    /* Bottom line */
    cputc (CH_LLCORNER);
    chline (XSize - 2);
    cputc (CH_LRCORNER);

    /* Vertical line, right side */
    cvlinexy (XSize - 1, 1, YSize - 2);

    /* Write the Agar version in the mid of the screen */
    gotoxy ((XSize - strlen ("Agar X.X.X")) / 2, YSize/2 - 1);
    textcolor(5);
    cprintf ("Agar %d.%d.%d", v.major, v.minor, v.patch);
    gotoxy ((XSize - strlen (v.release) - 1) / 2, wherey()+1);
    textcolor(2);
    cprintf ("\"%s\"", v.release);
    gotoxy ((XSize - strlen (WebsiteURL)) / 2, wherey()+1);
    textcolor(3);
    cprintf ("%s", WebsiteURL);
    gotoxy ((XSize - strlen (HelloText)) / 2, wherey()+1);
    textcolor(4);
    cprintf ("%s", HelloText);

    /* Wait for the user to press a key */
    (void) cgetc ();

    /* Clear the screen again */
    clrscr ();

    /* Done */
    return EXIT_SUCCESS;
}
