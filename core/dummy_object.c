/*	Public domain	*/

#include <agar/core.h>

/*
 * Definition of Dummy Agar object class
 */

typedef struct ag_dummy {
	struct object _inherit;
	int foo;
} AG_Dummy;

extern AG_ObjectClass agDummyClass;

/*
 * Implementation of Dummy Agar object class
 */

static void
Init(void *obj)
{
	AG_Dummy *d = obj;

	d->foo = 0;
}

static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	AG_Dummy *d = obj;

	d->foo = (int)AG_ReadUint32(ds);
	return (0);
}

static int
Save(void *obj, AG_DataSource *ds)
{
	AG_Dummy *d = obj;

	AG_WriteUint32(ds, (Uint32)d->foo);
	return (0);
}

AG_ObjectClass agDummyClass = {
	"AG_Dummy",
	sizeof(AG_Dummy),
	{ 0,0 },
	Init,
	NULL,			/* free */
	NULL,			/* destroy */
	Load,
	Save,
	NULL			/* edit */
};
