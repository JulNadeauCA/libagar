/*	Public domain	*/

struct map_tool;

/*
 * Dynamic map object. This Agar object (a MAP_Object or subclass thereof)
 * is attached to the MAP(3) object and may be referenced by MAP_Node's.
 * Map objects are saved and loaded automatically along with the map.
 *
 * Contrary to MAP_Item, a MAP_Object may occupy any number of MAP_Node's
 * (i.e., any number of nodes may refer to a single MAP_Object instance).
 *
 * Map objects maintain sorted arrays of references to neighboring objects.
 * The way these references are defined and sorted allows for different map
 * topologies to be realized, and also allows for high-level logic code to
 * work in a coordinate-free manner.
 */
typedef struct map_location {
	struct map_object *_Nonnull obj;	/* Back pointer to object */
	Uint flags;
#define MAP_OBJECT_LOCATION_SELECTED	0x01	/* Selected in editor */
#define MAP_OBJECT_LOCATION_VALID       0x2000	/* Validity flag */
	int x,y;				/* Node coordinates on map */
	Uint layer;				/* Attributed layer index */
	float z;				/* Vertical coordinate */
	float h;				/* Height */

	struct map_location *_Nullable *_Nonnull neigh; /* Adjacent objects */
	Uint                                    nNeigh;
	Uint32 _pad;
} MAP_Location;

typedef struct map_object {
	struct ag_object _inherit;		/* AG_Object -> MAP_Object */
	Uint id;				/* Persistent numerical ID */
	Uint flags;
#define MAP_OBJECT_VALID	0x01		/* Object is valid */
#define MAP_OBJECT_ATTACHED     0x02		/* Object is attached */
#define MAP_OBJECT_SELECTED	0x04		/* Selected in editor */
	MAP_Location *_Nullable *_Nonnull locs; /* Locations on the map */
	Uint                             nLocs;
	Uint32 _pad;
} MAP_Object;

/* Selected view for MAP_Object's 2D rendering function. */
typedef enum map_object_view {
	/* Primary Orthographic */
	MAP_OBJECT_TOP,
	MAP_OBJECT_BOTTOM,
	MAP_OBJECT_FRONT,
	MAP_OBJECT_LEFT,
	MAP_OBJECT_RIGHT,
	MAP_OBJECT_BACK,
	/* Auxiliary Orthographic */
	MAP_OBJECT_ISOMETRIC,
	MAP_OBJECT_DIMETRIC,
	MAP_OBJECT_TRIMETRIC,
} MAP_ObjectView;

/*
 * MAP_Object class description.
 */
typedef struct map_object_class {
	struct ag_object_class _inherit;     /* [AG_Object] -> [MAP_Object] */

	/* Update status (return 0 on success or -1 to destroy instance). */
	int (*_Nonnull update)(void *_Nonnull);
	
	/* Render using AG_WidgetPrimitives(3). */
	void (*_Nonnull draw)(void *_Nonnull, struct map_view *_Nonnull,
	                      const AG_Rect *_Nonnull, MAP_ObjectView);

	/* Render to an OpenGL context. */
	void (*_Nonnull drawGL)(void *_Nonnull, struct map_view *_Nonnull);

	/* Called by MAP(3) editor when creating or selecting an object. */
	void (*_Nonnull edit)(void *_Nonnull, struct ag_widget *_Nonnull,
	                      struct map_tool *_Nonnull);
} MAP_ObjectClass;

#define MAPOBJECT(obj)            ((MAP_Object *)(obj))
#define MAPCOBJECT(obj)           ((const MAP_Object *)(obj))
#define MAP_OBJECT_SELF()         MAPOBJECT( AG_OBJECT(0,"MAP_Object:*") )
#define MAP_OBJECT_PTR(n)         MAPOBJECT( AG_OBJECT((n),"MAP_Object:*") )
#define MAP_OBJECT_NAMED(n)       MAPOBJECT( AG_OBJECT_NAMED((n),"MAP_Object:*") )
#define MAP_OBJECT_CONST_SELF()   MAPOBJECT( AG_CONST_OBJECT(0,"MAP_Object:*") )
#define MAP_OBJECT_CONST_PTR(n)   MAPOBJECT( AG_CONST_OBJECT((n),"MAP_Object:*") )
#define MAP_OBJECT_CONST_NAMED(n) MAPOBJECT( AG_CONST_OBJECT_NAMED((n),"MAP_Object:*") )

#define MAPOBJECTCLASS(cls)    ((MAP_ObjectClass *)(cls))
#define MAPOBJECTCLASS_OF(obj) ((MAP_ObjectClass *)(AGOBJECT(obj)->cls))

__BEGIN_DECLS
extern MAP_ObjectClass mapObjectClass;

MAP_Object *MAP_ObjectNew(void *_Nullable, const char *_Nonnull);
__END_DECLS
