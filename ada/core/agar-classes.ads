------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                         A G A R  . C L A S S E S                         --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces.C;

package Agar.Classes is
  package C renames Interfaces.C;

  use type C.int;
  use type C.unsigned;

  -------------------------------------------
  -- Numerical IDs for Agar Object classes --
  -------------------------------------------
  type AG_Class is
    (AGC_OBJECT,
     AGC_CONFIG,
     AGC_DB,
     AGC_DB_HASH,
     AGC_DB_BTREE,
     AGC_DB_OBJECT,
     AGC_DRIVER,
     AGC_DRIVER_SW,
     AGC_DRIVER_SDLFB,
     AGC_DRIVER_SDLGL,
     AGC_DRIVER_SDL2FB,
     AGC_DRIVER_SDL2GL,
     AGC_DRIVER_MW,
     AGC_DRIVER_DUMMY,
     AGC_DRIVER_GLX,
     AGC_DRIVER_COCOA,
     AGC_DRIVER_WGL,
     AGC_DRIVER_SDL2MW,
     AGC_INPUT_DEVICE,
     AGC_MOUSE,
     AGC_KEYBOARD,
     AGC_JOYSTICK,
     AGC_CONTROLLER,
     AGC_FONT,
     AGC_FONT_BF,
     AGC_FONT_FT,
     AGC_WIDGET,
     AGC_WINDOW,
     AGC_GLVIEW,
     AGC_MFSPINBUTTON,
     AGC_MSPINBUTTON,
     AGC_BOX,
     AGC_MPANE,
     AGC_NOTEBOOK_TAB,
     AGC_STATUSBAR,
     AGC_TITLEBAR,
     AGC_TOOLBAR,
     AGC_CHECKBOX,
     AGC_BUTTON,
     AGC_COMBO,
     AGC_OBJECT_SELECTOR,
     AGC_CONSOLE,
     AGC_EDITABLE,
     AGC_FONT_SELECTOR,
     AGC_DIR_DLG,
     AGC_FILE_DLG,
     AGC_FIXED,
     AGC_FIXED_PLOTTER,
     AGC_GRAPH,
     AGC_HSVPAL,
     AGC_ICON,
     AGC_LABEL,
     AGC_MENU,
     AGC_MENU_VIEW,
     AGC_NOTEBOOK,
     AGC_NUMERICAL,
     AGC_PANE,
     AGC_PIXMAP,
     AGC_PROGRESSBAR,
     AGC_RADIO,
     AGC_SCROLLBAR,
     AGC_SCROLLVIEW,
     AGC_SEPARATOR,
     AGC_SLIDER,
     AGC_SOCKET,
     AGC_TABLE,
     AGC_TREETBL,
     AGC_TEXTBOX,
     AGC_TLIST,
     AGC_UCOMBO,
     AGC_VG_VIEW,
     AGC_MAP_VIEW,
     AGC_RG_TILEVIEW,
     AGC_SK_VIEW,
     AGC_SG_VIEW,
     AGC_M_PLOTTER,
     AGC_M_MATVIEW,
     AGC_WIDGET_LAST,
     AGC_VG,
     AGC_MAP,
     AGC_MAP_OBJECT,
     AGC_RG_TILESET,
     AGC_SG,
     AGC_SG_SCRIPT,
     AGC_SG_PROGRAM,
     AGC_SG_CG_PROGRAM,
     AGC_SG_TEXTURE,
     AGC_SG_NODE,
     AGC_SG_DUMMY,
     AGC_SG_CAMERA,
     AGC_SG_LIGHT,
     AGC_SG_GEOM,
     AGC_SG_WIDGET,
     AGC_SG_POINT,
     AGC_SG_LINE,
     AGC_SG_CIRCLE,
     AGC_SG_SPHERE,
     AGC_SG_PLANE,
     AGC_SG_POLYGON,
     AGC_SG_TRIANGLE,
     AGC_SG_RECTANGLE,
     AGC_SG_OBJECT,
     AGC_SG_POLYBALL,
     AGC_SG_POLYBOX,
     AGC_SG_VOXEL,
     AGC_SG_IMAGE,
     AGC_SK)
      with Convention => C;

  for AG_Class use (
     AGC_OBJECT          => 16#01_000001#,
     AGC_CONFIG          => 16#02_000001#,
     AGC_DB              => 16#03_000001#,
     AGC_DB_HASH         => 16#0301_0001#,
     AGC_DB_BTREE        => 16#0302_0001#,
     AGC_DB_OBJECT       => 16#04_000001#,
     AGC_DRIVER          => 16#05_000001#,
     AGC_DRIVER_SW       => 16#0501_0000#,
     AGC_DRIVER_SDLFB    => 16#0501_0001#,
     AGC_DRIVER_SDLGL    => 16#0501_0002#,
     AGC_DRIVER_SDL2FB   => 16#0501_0003#,
     AGC_DRIVER_SDL2GL   => 16#0501_0004#,
     AGC_DRIVER_MW       => 16#0502_0000#,
     AGC_DRIVER_DUMMY    => 16#0502_0001#,
     AGC_DRIVER_GLX      => 16#0502_0002#,
     AGC_DRIVER_COCOA    => 16#0502_0003#,
     AGC_DRIVER_WGL      => 16#0502_0004#,
     AGC_DRIVER_SDL2MW   => 16#0502_0005#,
     AGC_INPUT_DEVICE    => 16#06_000000#,
     AGC_MOUSE           => 16#0601_0001#,
     AGC_KEYBOARD        => 16#0602_0001#,
     AGC_JOYSTICK        => 16#0603_0001#,
     AGC_CONTROLLER      => 16#060301_01#,
     AGC_FONT            => 16#07_000000#,
     AGC_FONT_BF         => 16#0701_0001#,
     AGC_FONT_FT         => 16#0702_0001#,
     AGC_WIDGET          => 16#08_000001#,
     AGC_WINDOW          => 16#08_000002#,
     AGC_GLVIEW          => 16#08_000003#,  -- Deprecated
     AGC_MFSPINBUTTON    => 16#08_000004#,  -- Deprecated
     AGC_MSPINBUTTON     => 16#08_000005#,  -- Deprecated
     AGC_BOX             => 16#09_000001#,
     AGC_MPANE           => 16#0901_0001#,
     AGC_NOTEBOOK_TAB    => 16#0902_0001#,
     AGC_STATUSBAR       => 16#0903_0001#,
     AGC_TITLEBAR        => 16#0904_0001#,
     AGC_TOOLBAR         => 16#0905_0001#,
     AGC_CHECKBOX        => 16#0A_000001#,
     AGC_BUTTON          => 16#0B_000001#,
     AGC_COMBO           => 16#0C_000001#,
     AGC_OBJECT_SELECTOR => 16#0C01_0001#,
     AGC_CONSOLE         => 16#0D_000001#,
     AGC_EDITABLE        => 16#0E_000001#,
     AGC_FONT_SELECTOR   => 16#0F_000001#,
     AGC_DIR_DLG         => 16#10_000001#,
     AGC_FILE_DLG        => 16#11_000001#,
     AGC_FIXED           => 16#12_000001#,
     AGC_FIXED_PLOTTER   => 16#13_000001#,
     AGC_GRAPH           => 16#14_000001#,
     AGC_HSVPAL          => 16#15_000001#,
     AGC_ICON            => 16#16_000001#,
     AGC_LABEL           => 16#17_000001#,
     AGC_MENU            => 16#18_000001#,
     AGC_MENU_VIEW       => 16#19_000001#,
     AGC_NOTEBOOK        => 16#1A_000001#,
     AGC_NUMERICAL       => 16#1B_000001#,
     AGC_PANE            => 16#1C_000001#,
     AGC_PIXMAP          => 16#1D_000001#,
     AGC_PROGRESSBAR     => 16#1E_000001#,
     AGC_RADIO           => 16#1F_000001#,
     AGC_SCROLLBAR       => 16#20_000001#,
     AGC_SCROLLVIEW      => 16#21_000001#,
     AGC_SEPARATOR       => 16#22_000001#,
     AGC_SLIDER          => 16#23_000001#,
     AGC_SOCKET          => 16#24_000001#,
     AGC_TABLE           => 16#25_000001#,
     AGC_TREETBL         => 16#26_000001#,
     AGC_TEXTBOX         => 16#27_000001#,
     AGC_TLIST           => 16#28_000001#,
     AGC_UCOMBO          => 16#29_000001#,
     AGC_VG_VIEW         => 16#2A_000001#,
     AGC_MAP_VIEW        => 16#2B_000001#,
     AGC_RG_TILEVIEW     => 16#2C_000001#,
     AGC_SK_VIEW         => 16#2D_000001#,
     AGC_SG_VIEW         => 16#2E_000001#,
     AGC_M_PLOTTER       => 16#2F_000001#,
     AGC_M_MATVIEW       => 16#30_000001#,
     AGC_WIDGET_LAST     => 16#30_000002#,

     AGC_VG              => 16#71_000001#,
     AGC_MAP             => 16#72_000001#,
     AGC_MAP_OBJECT      => 16#73_000001#,
     AGC_RG_TILESET      => 16#74_000001#,
     AGC_SG              => 16#75_000001#,
     AGC_SG_SCRIPT       => 16#76_000001#,
     AGC_SG_PROGRAM      => 16#77_000001#,
     AGC_SG_CG_PROGRAM   => 16#7701_0001#,
     AGC_SG_TEXTURE      => 16#78_000001#,
     AGC_SG_NODE         => 16#7A_000001#,
     AGC_SG_DUMMY        => 16#7A01_0001#,
     AGC_SG_CAMERA       => 16#7A02_0001#,
     AGC_SG_LIGHT        => 16#7A03_0001#,
     AGC_SG_GEOM         => 16#7A04_0001#,
     AGC_SG_WIDGET       => 16#7A0401_01#,
     AGC_SG_POINT        => 16#7A0402_01#,
     AGC_SG_LINE         => 16#7A0403_01#,
     AGC_SG_CIRCLE       => 16#7A0404_01#,
     AGC_SG_SPHERE       => 16#7A0405_01#,
     AGC_SG_PLANE        => 16#7A0406_01#,
     AGC_SG_POLYGON      => 16#7A0407_01#,
     AGC_SG_TRIANGLE     => 16#7A0408_01#,
     AGC_SG_RECTANGLE    => 16#7A0409_01#,
     AGC_SG_OBJECT       => 16#7A05_0001#,
     AGC_SG_POLYBALL     => 16#7A0501_01#,
     AGC_SG_POLYBOX      => 16#7A0502_01#,
     AGC_SG_VOXEL        => 16#7A06_0001#,
     AGC_SG_IMAGE        => 16#7A07_0001#,
     AGC_SK              => 16#7B_000001#);

  for AG_Class'Size use C.int'Size;

end Agar.Classes;
