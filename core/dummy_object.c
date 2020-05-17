/*	Public domain	*/
/*
 * Example Agar object class for a serializable object containing
 * dynamically allocated data.
 */

#include <agar/core/core.h>

#define MYDATASIZE 100

/*
 * Create a new auto-allocated and initialized instance of AG_Dummy.
 *
 * Constructor routines such as this are optional and provided by our API
 * as a language convenience (objects can also be allocated statically
 * and initialized by the user by calling AG_ObjectInit(3) directly).
 */
AG_Dummy *
AG_DummyNew(int x, int y, Uint flags)
{
	AG_Dummy *d;

	if ((d = AG_TryMalloc(sizeof(AG_Dummy))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(d, &mdDummyClass);

	d->x = x;
	d->y = y;
	d->flags = flags;

	return (d);
}

/* Initialize a new object instance */
static void
Init(void *_Nonnull obj)
{
	AG_Dummy *d = obj;

	d->flags = 0;
	d->x = 0;
	d->y = 0;
	d->myData = Malloc(MYDATASIZE);

	memset(d->myData, 0, MYDATASIZE);
}

/* Release all resources allocated by an object instance */
static void
Destroy(void *_Nonnull obj)
{
	AG_Dummy *d = obj;

	free(d->myData);
}

/* Read the serialized state of an object from a data source. */
static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
    const AG_Version *_Nonnull ver)
{
	AG_Dummy *d = obj;

	d->flags &= ~(AG_DUMMY_SAVED);
	d->flags |= AG_ReadUint8(ds) & AG_DUMMY_SAVED;
	d->x = (int)AG_ReadUint16(ds);
	d->y = (int)AG_ReadUint16(ds);
	return AG_Read(ds, d->myData, MYDATASIZE);
}

/* Serialize an object to a data source. */
static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	AG_Dummy *d = obj;

	AG_WriteUint8(ds, (Uint8)(d->flags & AG_DUMMY_SAVED));
	AG_WriteUint16(ds, (Uint16)d->x);
	AG_WriteUint16(ds, (Uint16)d->y);
	return AG_Write(ds, d->myData, MYDATASIZE);
}

#ifdef ENABLE_GUI
/* Construct a graphical edition box. */
static void *_Nullable
Edit(void *_Nonnull obj)
{
	AG_Dummy *d = obj;
	AG_Box *box;

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_CheckboxNewFlag(box, 0, "Option #1", &d->flags, AG_DUMMY_OPTION1);
	AG_CheckboxNewFlag(box, 0, "Option #2", &d->flags, AG_DUMMY_OPTION2);
	AG_NumericalNewInt(box, 0, NULL, "X value", &d->x);
	AG_NumericalNewInt(box, 0, NULL, "Y value", &d->y);
	return (box);
}
#endif /* ENABLE_GUI */

AG_ObjectClass agDummyClass = {
	"AG_Dummy",
	sizeof(AG_Dummy),
	{ 1,0 },
	Init,
	NULL,		/* reset */
	Destroy,
	Load,
	Save,
#ifdef ENABLE_GUI
	Edit
#else
	NULL		/* edit */
#endif
};
