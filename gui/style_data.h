/* File generated by agar bundlecss */
const char *agStyleDefault_Data = 
	"/*\n"
	" * Default Agar Stylesheet.\n"
	" * ex:syn=agarcss\n"
	" *\n"
	" * Widgets inherit colors and fonts from their parents by default.\n"
	" * Margins, paddings and spacings are not inherited. Selectors include:\n"
	" *\n"
	" *   Selector type                Example\n"
	" *   -------------                --------\n"
	" *   Widget(s) of class (exact)   AG_Box\n"
	" *   Widget(s) of class (pattern) AG_Widget:AG_Box:*\n"
	" *   Child widget(s) of class     AG_FontSelector > AG_Tlist\n"
	" *   Child widget named           AG_Combo > \"trigger\"\n"
	" *\n"
	" * See: AG_StyleSheet(3) and \"STYLE ATTRIBUTES\" section of AG_Widget(3).\n"
	" */\n"
	"AG_Window {\n"
	"padding: 0 3 4 3;\n"
	"background-color: rgb(70,70,70);\n"
	"text-color: rgb(200,200,200);\n"
	"text-color#hover: rgb(240,240,240);\n"
	"}\n"
	"AG_Box {\n"
	"padding: 2 3 2 3;    /* top right bottom left */\n"
	"spacing: 3 2;        /* horiz vert */\n"
	"}\n"
	"AG_Button {\n"
	"padding: 5 10 5 10;\n"
	"text-color: rgb(255,255,255);\n"
	"color: #787878;\n"
	"color#focused: rgb(128,128,129);\n"
	"color#hover: rgb(133,133,136);\n"
	"selection-color#focused: #646496;\n"
	"selection-color#hover: #969664;\n"
	"}\n"
	"AG_Checkbox {\n"
	"spacing: 5;\n"
	"background-color: rgb(80,80,120);\n"
	"background-color#hover: rgb(100,100,140);\n"
	"}\n"
	"AG_Combo > \"input\" {\n"
	"padding: 0 0 2 0;\n"
	"font-size: 95%;\n"
	"}\n"
	"AG_Combo > \"trigger\" {\n"
	"padding: 2;\n"
	"}\n"
	"AG_Console {\n"
	"font-family: monoalgue;\n"
	"padding: 4;\n"
	"spacing: 3;\n"
	"background-color: #000;\n"
	"background-color#disabled: #222;\n"
	"text-color: #f0f0f0;\n"
	"text-color#disabled: #ccc;\n"
	"}\n"
	"AG_Console > AG_Scrollbar {\n"
	"font-size: 60%;\n"
	"color: #444;\n"
	"color#hover: #40404a;\n"
	"text-color: #aaa;\n"
	"}\n"
	"AG_Editable {\n"
	"padding: 1 3 1 3;\n"
	"background-color: #0000;\n"
	"}\n"
	"AG_FileDlg {\n"
	"spacing: 2;\n"
	"padding: 3;\n"
	"}\n"
	"AG_Icon {\n"
	"spacing: 4 0;\n"
	"}\n"
	"AG_Label {\n"
	"padding: 2 6 2 6;\n"
	"background-color: #0000;\n"
	"background-color#focused: #0000;\n"
	"}\n"
	"AG_Menu {\n"
	"padding: 2 5 2 5;\n"
	"color: rgb(60,60,60);\n"
	"color#disabled: rgb(40,40,110);\n"
	"selection-color: rgb(40,40,110);\n"
	"text-color#disabled: rgb(170,170,170);\n"
	"}\n"
	"AG_Menu (zoom = 0-9) {\n"
	"font-family: league-spartan;\n"
	"font-size: 140%;\n"
	"padding: 1 2 1 2;\n"
	"}\n"
	"AG_Menu (zoom > 24) {\n"
	"font-family: fira-sans-condensed;\n"
	"padding: 2 5 10 5;\n"
	"}\n"
	"AG_MenuView {\n"
	"padding: 4 8 4 8;\n"
	"spacing: 8 0;\n"
	"color: rgb(60,60,60);\n"
	"selection-color: rgb(40,40,110);\n"
	"color#disabled: rgb(40,40,110);\n"
	"text-color#disabled: rgb(170,170,170);\n"
	"}\n"
	"AG_MenuView (zoom = 0-9) {\n"
	"font-family: league-spartan\n"
	"}\n"
	"AG_MenuView (zoom > 24) {\n"
	"font-family: fira-sans-condensed;\n"
	"}\n"
	"AG_Notebook {\n"
	"margin: 2;\n"
	"}\n"
	"AG_Numerical > AG_UCombo {\n"
	"font-size: 80%;\n"
	"}\n"
	"AG_Numerical > AG_Button {\n"
	"padding: 0;\n"
	"font-size: 55%;\n"
	"text-color: #bebebe;\n"
	"color#hover: #969590;\n"
	"}\n"
	"AG_HSVPal {\n"
	"font-family: league-gothic;\n"
	"font-size: 140%;\n"
	"}\n"
	"AG_HSVPal (width < 50) {\n"
	"font-family: league-gothic-condensed;\n"
	"font-size: 90%;\n"
	"}\n"
	"AG_HSVPal (width < 90) {\n"
	"font-family: league-gothic-condensed;\n"
	"font-size: 120%;\n"
	"}\n"
	"AG_Pane {\n"
	"color: #666;\n"
	"color#hover: #777;\n"
	"line-color#hover: rgb(200,200,180);\n"
	"}\n"
	"AG_ProgressBar {\n"
	"font-family: league-gothic;\n"
	"font-size: 90%;\n"
	"padding: 2;\n"
	"}\n"
	"AG_Radio {\n"
	"spacing: 1;\n"
	"background-color#hover: rgb(80,80,120);\n"
	"}\n"
	"AG_Textbox {\n"
	"padding: 3;\n"
	"spacing: 5;\n"
	"background-color: rgb(80,80,80);\n"
	"}\n"
	"AG_Titlebar {\n"
	"font-size: 90%;\n"
	"padding: 3;\n"
	"spacing: 1;\n"
	"color: rgb(40,50,60);\n"
	"color#disabled: rgb(35,35,35);\n"
	"}\n"
	"AG_Titlebar > AG_Label {\n"
	"background-color: #0000;\n"
	"background-color#focused: #0000;\n"
	"padding: 2 0 0 2;\n"
	"}\n"
	"AG_Titlebar > AG_Button {\n"
	"padding: 0 5 0 5;\n"
	"color: #0000;\n"
	"color#hover: #868580;\n"
	"}\n"
	"AG_Toolbar {\n"
	"padding: 0;\n"
	"spacing: 1;\n"
	"}\n"
	"AG_Scrollbar {\n"
	"font-size: 60%;\n"
	"background-color: rgb(65,65,65);\n"
	"color: rgb(80,80,80);\n"
	"color#hover: rgb(90,90,90);\n"
	"high-color: rgb(120,120,120);\n"
	"text-color: #bebebe;\n"
	"}\n"
	"AG_Separator {\n"
	"line-color: #888;\n"
	"padding: 8 10 8 10;\n"
	"}\n"
	"AG_Statusbar {\n"
	"padding: 2;\n"
	"spacing: 1;\n"
	"}\n"
	"AG_Tlist {\n"
	"padding: 1 0 2 0;\n"
	"spacing: 4 0;\n"
	"}\n"
	"AG_UCombo > \"trigger\" {\n"
	"padding: 3;\n"
	"}\n"
	"";

AG_StaticCSS agStyleDefault = {
	"agStyleDefault",
	3676,
	&agStyleDefault_Data,
	NULL
};
