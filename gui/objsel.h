/*	Public domain	*/

#ifndef _AGAR_GUI_OBJSEL_H_
#define _AGAR_GUI_OBJSEL_H_

#include <agar/gui/widget.h>
#include <agar/gui/combo.h>

#include <agar/gui/begin.h>

typedef struct ag_object_selector {
	struct ag_combo com;
	Uint flags;
	char type_mask[AG_OBJECT_HIER_MAX];
	Uint32 _pad;
	void *_Nullable pobj;     /* Pointer to the object */
	void *_Nullable root;     /* Root of object's VFS */
	void *_Nullable object;   /* Default "object" binding */
} AG_ObjectSelector;

#define   AGOBJECTSELECTOR(obj)      ((AG_ObjectSelector *)(obj))
#define  AGcOBJECTSELECTOR(obj)      ((const AG_ObjectSelector *)(obj))
#define  AG_OBJECTSELECTOR_ISA(o)    (((AGOBJECT(o)->cid & 0xffff0000) >> 16) == 0x0C01)
#define  AG_OBJECTSELECTOR_SELF()    AGOBJECTSELECTOR(  AG_OBJECT(0,        "AG_Widget:AG_Combo:AG_ObjectSelector:*") )
#define  AG_OBJECTSELECTOR_PTR(n)    AGOBJECTSELECTOR(  AG_OBJECT((n),      "AG_Widget:AG_Combo:AG_ObjectSelector:*") )
#define  AG_OBJECTSELECTOR_NAMED(n)  AGOBJECTSELECTOR(  AG_OBJECT_NAMED((n),"AG_Widget:AG_Combo:AG_ObjectSelector:*") )
#define AG_cOBJECTSELECTOR_SELF()   AGCOBJECTSELECTOR( AG_cOBJECT(0,        "AG_Widget:AG_Combo:AG_ObjectSelector:*") )
#define AG_cOBJECTSELECTOR_PTR(n)   AGCOBJECTSELECTOR( AG_cOBJECT((n),      "AG_Widget:AG_Combo:AG_ObjectSelector:*") )
#define AG_cOBJECTSELECTOR_NAMED(n) AGCOBJECTSELECTOR( AG_cOBJECT_NAMED((n),"AG_Widget:AG_Combo:AG_ObjectSelector:*") )

__BEGIN_DECLS
extern AG_WidgetClass agObjectSelectorClass;

AG_ObjectSelector *_Nonnull AG_ObjectSelectorNew(void *_Nullable, int,
						 void *_Nullable, void *_Nullable,
						 const char *_Nonnull, ...)
						FORMAT_ATTRIBUTE(printf,5,6);

AG_TlistItem *_Nullable AG_ObjectSelectorSelect(AG_ObjectSelector *_Nonnull,
						void *_Nullable);

void AG_ObjectSelectorMaskType(AG_ObjectSelector *_Nonnull, const char *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_OBJSEL_H_ */
