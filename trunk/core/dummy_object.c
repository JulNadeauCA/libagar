/*	Public domain	*/
/*
 * Implementation of an example Agar object class.
 */

#include <agar/core/core.h>

/* Optional constructor routine */
AG_Dummy *
AG_DummyNew(void)
{
	AG_Dummy *d;

	if ((d = AG_TryMalloc(sizeof(AG_Dummy))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(d, &mdDummyClass);
	return (d);
}

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
