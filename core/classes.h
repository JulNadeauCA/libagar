/*	Public domain	*/

/*
 * Numerical Identifiers for AG_Object(3) Classes.
 *
 * If identifiers related to ag_core or ag_gui are changed, the Agar minor
 * version must be bumped so that serialization routines can handle
 * versioned datafiles with identifiers encoded in them.
 * 
 * Class-membership tests can be performed using the *_ISA() macro which
 * should be defined in the class's header file.
 * 
 * To add your own class IDs to this file, contact the maintainer by e-mail
 * at <vedge@csoft.net>.
 */

#ifndef _AG_CORE_CLASSES_H_
#define _AG_CORE_CLASSES_H_

#if AG_MODEL != AG_SMALL

typedef enum ag_class {
	/* Agar-Core */
	AGC_OBJECT          = 0x01000001,  /* AG_Object (base class) */
	AGC_CONFIG          = 0x02000001,  /* AG_Config */
	AGC_DB              = 0x03000001,  /* AG_Db */
	AGC_DB_HASH         = 0x03010001,      /* AG_DbHash */
	AGC_DB_BTREE        = 0x03020001,      /* AG_DbBtree */
	AGC_DB_MYSQL        = 0x03040001,      /* AG_DbMySQL */
	AGC_DB_OBJECT       = 0x04000001,  /* AG_DbObject */
	/* Agar-GUI */
	AGC_DRIVER          = 0x05000001,  /* AG_Driver (non-instantiatable) */
	AGC_DRIVER_SW       = 0x05010000,  /* AG_Driver -> AG_DriverSw (non-instantiatable) */
	AGC_DRIVER_SDLFB    = 0x05010001,      /* AG_DriverSDLFB */
	AGC_DRIVER_SDLGL    = 0x05010002,      /* AG_DriverSDLGL */
	AGC_DRIVER_SDL2FB   = 0x05010003,      /* AG_DriverSDL2FB */
	AGC_DRIVER_SDL2GL   = 0x05010004,      /* AG_DriverSDL2GL */
	AGC_DRIVER_MW       = 0x05020000,  /* AG_Driver -> AG_DriverMw (non-instantiatable) */
	AGC_DRIVER_DUMMY    = 0x05020001,      /* AG_DriverDUMMY */
	AGC_DRIVER_GLX      = 0x05020002,      /* AG_DriverGLX */
	AGC_DRIVER_COCOA    = 0x05020003,      /* AG_DriverCOCOA */
	AGC_DRIVER_WGL      = 0x05020004,      /* AG_DriverWGL */
	AGC_DRIVER_SDL2MW   = 0x05020005,      /* AG_DriverSDL2MW */
	AGC_INPUT_DEVICE    = 0x06000000,  /* AG_InputDevice (non-instantiatable) */
	AGC_MOUSE           = 0x06010001,      /* AG_Mouse */
	AGC_KEYBOARD        = 0x06020001,      /* AG_Keyboard */
	AGC_JOYSTICK        = 0x06030001,      /* AG_Joystick */
	AGC_CONTROLLER      = 0x06030101,          /* AG_Controller */
	AGC_FONT            = 0x07000000,  /* AG_Font (non-instantiatable) */
	AGC_FONT_BF         = 0x07010001,      /* AG_FontBf */
	AGC_FONT_FT         = 0x07020001,      /* AG_FontFt */
	AGC_WIDGET          = 0x08000001,  /* AG_Widget */
	AGC_WINDOW          = 0x08000002,  /* AG_Widget -> AG_Window (non-subclassable) */
	AGC_GLVIEW          = 0x08000003,  /* AG_Widget -> AG_GLView (deprecated) */
	AGC_MFSPINBUTTON    = 0x08000004,  /* AG_Widget -> AG_MFSpinbutton (deprecated) */
	AGC_MSPINBUTTON     = 0x08000005,  /* AG_Widget -> AG_MSpinbutton (deprecated) */
	AGC_BOX             = 0x09000001,  /* AG_Widget -> AG_Box */
	AGC_MPANE           = 0x09010001,      /* AG_MPane */
	AGC_NOTEBOOK_TAB    = 0x09020001,      /* AG_NotebookTab */
	AGC_STATUSBAR       = 0x09030001,      /* AG_Statusbar */
	AGC_TITLEBAR        = 0x09040001,      /* AG_Titlebar */
	AGC_TOOLBAR         = 0x09050001,      /* AG_Toolbar */
	AGC_CHECKBOX        = 0x0A000001,  /* AG_Widget -> AG_Checkbox */
	AGC_BUTTON          = 0x0B000001,  /* AG_Widget -> AG_Button */
	AGC_COMBO           = 0x0C000001,  /* AG_Widget -> AG_Combo */
	AGC_OBJECT_SELECTOR = 0x0C010001,      /* ObjectSelector */
	AGC_CONSOLE         = 0x0D000001,  /* AG_Widget -> AG_Console */
	AGC_EDITABLE        = 0x0E000001,  /* AG_Widget -> AG_Editable */
	AGC_FONT_SELECTOR   = 0x0F000001,  /* AG_Widget -> AG_FontSelector */
	AGC_DIR_DLG         = 0x10000001,  /* AG_Widget -> AG_DirDlg */
	AGC_FILE_DLG        = 0x11000001,  /* AG_Widget -> AG_FileDlg */
	AGC_FIXED           = 0x12000001,  /* AG_Widget -> AG_Fixed */
	AGC_FIXED_PLOTTER   = 0x13000001,  /* AG_Widget -> AG_FixedPlotter */
	AGC_GRAPH           = 0x14000001,  /* AG_Widget -> AG_Graph */
	AGC_HSVPAL          = 0x15000001,  /* AG_Widget -> AG_HSVPal */
	AGC_ICON            = 0x16000001,  /* AG_Widget -> AG_Icon */
	AGC_LABEL           = 0x17000001,  /* AG_Widget -> AG_Label */
	AGC_MENU            = 0x18000001,  /* AG_Widget -> AG_Menu */
	AGC_MENUVIEW        = 0x19000001,  /* AG_Widget -> AG_MenuView */
	AGC_NOTEBOOK        = 0x1A000001,  /* AG_Widget -> AG_Notebook */
	AGC_NUMERICAL       = 0x1B000001,  /* AG_Widget -> AG_Numerical */
	AGC_PANE            = 0x1C000001,  /* AG_Widget -> AG_Pane */
	AGC_PIXMAP          = 0x1D000001,  /* AG_Widget -> AG_Pixmap */
	AGC_PROGRESSBAR     = 0x1E000001,  /* AG_Widget -> AG_ProgressBar */
	AGC_RADIO           = 0x1F000001,  /* AG_Widget -> AG_Radio */
	AGC_SCROLLBAR       = 0x20000001,  /* AG_Widget -> AG_Scrollbar */
	AGC_SCROLLVIEW      = 0x21000001,  /* AG_Widget -> AG_Scrollview */
	AGC_SEPARATOR       = 0x22000001,  /* AG_Widget -> AG_Separator */
	AGC_SLIDER          = 0x23000001,  /* AG_Widget -> AG_Slider */
	AGC_SOCKET          = 0x24000001,  /* AG_Widget -> AG_Socket */
	AGC_TABLE           = 0x25000001,  /* AG_Widget -> AG_Table */
	AGC_TREETBL         = 0x26000001,  /* AG_Widget -> AG_Treetbl */
	AGC_TEXTBOX         = 0x27000001,  /* AG_Widget -> AG_Textbox */
	AGC_TLIST           = 0x28000001,  /* AG_Widget -> AG_Tlist */
	AGC_UCOMBO          = 0x29000001,  /* AG_Widget -> AG_UCombo */
	AGC_VG_VIEW         = 0x2A000001,  /* AG_Widget -> VG_View (ag_vg) */
	AGC_MAP_VIEW        = 0x2B000001,  /* AG_Widget -> MAP_View (ag_map) */
	AGC_RG_TILEVIEW     = 0x2C000001,  /* AG_Widget -> RG_Tileview (ag_map) */
	AGC_SK_VIEW         = 0x2D000001,  /* AG_Widget -> SK_View (ag_sk) */
	AGC_SG_VIEW         = 0x2E000001,  /* AG_Widget -> SG_View (ag_sg) */
	AGC_M_PLOTTER       = 0x2F000001,  /* AG_Widget -> M_Plotter */
	AGC_M_MATVIEW       = 0x30000001,  /* AG_Widget -> M_Matview */
	AGC_WIDGET_LAST     = 0x30000002,

	AGC_VG              = 0x71000001,  /* VG (in ag_vg) */
	AGC_MAP             = 0x72000001,  /* MAP (in ag_map) */
	AGC_MAP_OBJECT      = 0x73000001,  /* MAP_Object */
	AGC_RG_TILESET      = 0x74000001,  /* RG_Tileset */
	AGC_SG              = 0x75000001,  /* SG (in ag_sg) */
	AGC_SG_SCRIPT       = 0x76000001,  /* SG_Script */
	AGC_SG_PROGRAM      = 0x77000001,  /* SG_Program */
	AGC_SG_CG_PROGRAM   = 0x77010001,      /* SG_CgProgram */
	AGC_SG_TEXTURE      = 0x78000001,  /* SG_Texture */
	                   /* 0x79000001, */
	AGC_SG_NODE         = 0x7A000001,  /* SG_Node */
	AGC_SG_DUMMY        = 0x7A010001,      /* SG_Dummy */
	AGC_SG_CAMERA       = 0x7A020001,      /* SG_Camera */
	AGC_SG_LIGHT        = 0x7A030001,      /* SG_Light */
	AGC_SG_GEOM         = 0x7A040001,      /* SG_Geom */
	AGC_SG_WIDGET       = 0x7A040101,          /* SG_Widget */
	AGC_SG_POINT        = 0x7A040201,          /* SG_Point */
	AGC_SG_LINE         = 0x7A040301,          /* SG_Line */
	AGC_SG_CIRCLE       = 0x7A040401,          /* SG_Circle */
	AGC_SG_SPHERE       = 0x7A040501,          /* SG_Sphere */
	AGC_SG_PLANE        = 0x7A040601,          /* SG_Plane */
	AGC_SG_POLYGON      = 0x7A040701,          /* SG_Polygon */
	AGC_SG_TRIANGLE     = 0x7A040801,          /* SG_Triangle */
	AGC_SG_RECTANGLE    = 0x7A040901,          /* SG_Rectangle */
	AGC_SG_OBJECT       = 0x7A050001,      /* SG_Object */
	AGC_SG_POLYBALL     = 0x7A050101,          /* SG_Polyball */
	AGC_SG_POLYBOX      = 0x7A050201,          /* SG_Polybox */
	AGC_SG_VOXEL        = 0x7A060001,      /* SG_Voxel */
	AGC_SG_IMAGE        = 0x7A070001,      /* SG_Image */
	AGC_SK              = 0x7B000001   /* SK */
} AG_Class;

#else /* AG_SMALL */

typedef enum ag_class {
	/* Agar-Core (SM mode) */
	AGC_OBJECT     = 0x0101,  /* AG_Object (base class) */
	AGC_CONFIG     = 0x0201,  /* AG_Config */
	              /* 0x0301 */
	              /* 0x0401 */
	/* Micro-Agar */
	AGC_DRIVER     = 0x0501,  /* MA_Driver */
	AGC_DRIVER_C64 = 0x0502,      /* MA_DriverC64 */
	AGC_WIDGET     = 0x0501,  /* MA_Widget */
	AGC_WINDOW     = 0x0601,      /* MA_Window */
	AGC_BOX        = 0x0701,      /* MA_Box */
	AGC_BOX_HORIZ  = 0x0702,          /* MA_BoxHoriz */
	AGC_BOX_VERT   = 0x0703,          /* MA_BoxVert */
	AGC_SCROLLVIEW = 0x0800,      /* MA_Scrollview */
	AGC_NOTEBOOK   = 0x0900,      /* MA_Notebook */
	AGC_HSVPAL     = 0x0A01,      /* MA_HSVPal */
	AGC_BUTTON     = 0x0B01,      /* MA_Button */
	AGC_CHECKBOX   = 0x0B02,          /* MA_Checkbox */
	AGC_RADIO      = 0x0B03,          /* MA_Radio */
	AGC_COMBO      = 0x0C01,      /* MA_Combo */
	AGC_PIXMAP     = 0x0D01,      /* MA_Pixmap */
	AGC_EDITABLE   = 0x0E01       /* MA_Editable */

} AG_Class;

#endif /* AG_SMALL */

#endif /* _AG_CORE_CLASSES_H_ */
