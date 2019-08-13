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

/*
 * Register an example Agar object class.
 */
static void
MyClass_SayHello(AG_Event *event)
{
	AG_Object *obj = AG_OBJECT_SELF();
	const char *thing = AG_STRING(1);

	AG_Debug(obj, "Hello, %s!\n", thing);
}
static void
MyClass_Init(void *pObj)
{
	AG_Object *obj = pObj;
	int mag;

	AG_SetInt(obj, "magic", 30000);
	if ((mag = AG_GetInt(obj,"magic")) == 30000) {
		AG_Verbose("Variables OK! (magic=%d)", mag);
	} else {
		AG_Verbose("Variables don't work (%d!=30000)", mag);
	}
	AG_SetEvent(obj, "greet", MyClass_SayHello, NULL);
}
static void
MyClass_Destroy(void *pObj)
{
	AG_Object *obj = pObj;

	AG_Verbose("Instance of MyClass finalizing");
}
static AG_ObjectClass myClass = {
	"My_Class",
	sizeof(AG_Object),
	{ 1,0 },
	MyClass_Init,
	NULL,			/* reset */
	MyClass_Destroy,
	NULL,			/* load */
	NULL,			/* save */
	NULL			/* edit */
};

int
main(void)
{
	unsigned char XSize, YSize;
	AG_Object *myObj;
	
	textcolor(COLOR_WHITE);
	bordercolor(COLOR_BLACK);
	bgcolor(COLOR_BLACK);
	cursor(0);
	clrscr();

	_heapadd((void *)0x0803, 0x17fd);
/*	_heapadd((void *)0x0400, 0x0400); */

	if (AG_InitCore(NULL, AG_VERBOSE) == -1) {
		return EXIT_FAILURE;
	}
	AG_Verbose("Agar OK (%d bytes free)", _heapmemavail());

	/* Register a new Agar object class. */
	AG_RegisterClass(&myClass);

	/* Create an instance of our new class. */
	if ((myObj = AG_ObjectNew(NULL, "myobj1", &myClass)) == NULL) {
		AG_FatalError("ObjectNew");
	}
	AG_Verbose("%s lives at %p", myObj->name, myObj);

	/* Test event delivery. */
	AG_PostEvent(NULL, myObj, "greet", "%s", "world");

 	/* Clear the screen again */
//	clrscr ();

	/* Done */
	return EXIT_SUCCESS;
}

