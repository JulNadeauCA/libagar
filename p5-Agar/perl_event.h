/*	Public domain	*/

extern SV  *AP_RetrieveEventPV(const AG_Event *);
extern void AP_StoreEventPV(AG_Event *, SV *);
extern void AP_EventHandlerDecRef(AG_Event *);
extern void AP_DecRefEventPV(AG_Event *);
extern void AP_EventHandler(AG_Event *);

typedef AG_Event * Agar__Event;
