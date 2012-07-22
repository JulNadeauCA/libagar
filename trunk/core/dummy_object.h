/*	Public domain	*/
/*
 * Definition of an example Agar object class.
 */

typedef struct ag_dummy {
	struct ag_object _inherit;
	int foo;
} AG_Dummy;

extern AG_ObjectClass agDummyClass;

AG_Dummy *AG_DummyNew(void);

