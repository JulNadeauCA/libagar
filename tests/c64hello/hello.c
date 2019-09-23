/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dbg.h>

#include <agar/core.h>
#include <agar/micro.h>

/*
 * Register an example Agar object class.
 */

/* Handler for `hello' event. */
static void
MyClass_SayHello(AG_Event *event)
{
	AG_Object *obj = AG_OBJECT_SELF();
	const char *thing = AG_STRING(1);

	AG_Debug(obj, "Hello, %s!\n", thing);
}

/* Handler for `find-primes' event (really a method). */
static void
MyClass_FindPrimes(AG_Event *event)
{
	const Uint8 nPrimes = AG_INT(1);
	Uint8 n, i, nFound=0, cc=2;

	for (n=0; nFound < nPrimes; n++) {
		int flag = 0;

		for (i = 2; i <= (n >> 1); ++i) {
			if ((n % i) == 0) {
				flag = 1;
				break;
			}
		}
		if (n != 1) {
			if (!flag) {
				textcolor(cc);
				if (++cc > 8) { cc = 2; }
				cprintf("%d ", n);
				nFound++;
			}
		}
	}
}
static void
MyClass_Init(void *pObj)
{
	AG_Object *obj = pObj;

//	AG_SetEvent(obj, "hello", MyClass_SayHello, NULL);
	AG_SetEvent(obj, "find-primes", MyClass_FindPrimes, NULL);
}
static AG_ObjectClass myClass = {
	"My_Class",
	sizeof(AG_Object),
	{ 1,0 },
	MyClass_Init,
	NULL,			/* reset */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL			/* edit */
};

int
main(void)
{
	unsigned char XSize, YSize;
	AG_Object *myObj = NULL;
	
	textcolor(COLOR_WHITE);
	bordercolor(COLOR_BLACK);
	bgcolor(COLOR_BLACK);
	cursor(0);
	clrscr();

	_heapadd((void *)0x0803, 0x17fd);

	if (AG_InitCore(NULL, AG_VERBOSE) == -1) {
		return EXIT_FAILURE;
	}

	/* Register a new Agar object class. */
	AG_RegisterClass(&myClass);

	while (1) {
		int c;

		AG_Verbose("Agar (%d bytes free)", _heapmemavail());
		AG_Verbose("AG_Object=%d, AG_Event=%d", sizeof(AG_Object),
		                                        sizeof(AG_Event));
		AG_Verbose("[O]bj | [P]rimes | [G]UI | [Q]uit ?");

		switch ((c = cgetc())) {
		case 'o':
			/* Create an instance of our new class. */
			AG_Verbose("Instantiating (%u bytes)\n", myClass.size);
			if ((myObj = AG_ObjectNew(NULL, "myobj1", &myClass))
			    == NULL) {
				AG_Verbose("Failed: %s", AG_GetError());
			} else {
				AG_Verbose("OK! %s is at %p", myObj->name, myObj);
			}
			break;
		case 'p':
			if (myObj) {
				AG_PostEvent(myObj, "find-primes", "%i", 100);
			} else {
				AG_Verbose("Spawn an [O]bject first!\n");
			}
			break;
		case 'g':
#if 0
			if (AG_InitGUI(0) == 0) {
				AG_Verbose("GUI OK");
			} else {
				AG_Verbose("AG_InitGUI: %s", AG_GetError());
			}
#endif
			break;
		case 'q':
			goto out;
		default:
			AG_Verbose("I don't know what %c means", c);
			break;
		}
	}
out:
 	/* Clear the screen again */
//	clrscr ();

	/* Done */
	return EXIT_SUCCESS;
}

