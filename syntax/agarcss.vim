" Vim syntax file
" Language:     Agar GUI (LibAgar) Cascading Style Sheets
" Previous Contributor List:
"               Jules Wang      <w.jq0722@gmail.com>
"               Claudio Fleiner <claudio@fleiner.com>
"               Yeti            (Add full CSS2, HTML4 support)
"               Nikolai Weibull (Add CSS2 support)
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarcss.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 February 18

" quit when a syntax file was already loaded
if !exists("main_syntax")
  if exists("b:current_syntax")
    finish
  endif
  let main_syntax = 'agarcss'
elseif exists("b:current_syntax") && b:current_syntax == "agarcss"
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn case match

" Add dash to allowed keyword characters.
syn iskeyword @,48-57,_,192-255,-

" Standard Agar Widget classes.
syn keyword cssTagName AG_Box AG_Button AG_Checkbox AG_Combo AG_Console
syn keyword cssTagName AG_Box AG_DirDlg AG_Editable AG_FileDlg AG_Fixed
syn keyword cssTagName AG_FixedPlotter AG_FontSelector AG_GLView AG_Graph
syn keyword cssTagName AG_HSVPal AG_Icon AG_Label AG_Menu AG_MenuView
syn keyword cssTagName AG_MFSpinButton AG_MPane AG_MSpinButton AG_Notebook
syn keyword cssTagName AG_Numerical AG_ObjectSelector AG_Pane AG_Pixmap
syn keyword cssTagName AG_ProgressBar AG_Radio AG_Scrollbar AG_Scrollview
syn keyword cssTagName AG_Separator AG_Slider AG_Socket AG_Statusbar
syn keyword cssTagName AG_Table AG_Textbox AG_Titlebar AG_Tlist AG_Toolbar
syn keyword cssTagName AG_Treetbl AG_UCombo AG_Widget AG_Window

" Agar Widget classes from extra libraries.
syn keyword cssTagName MAP_View RG_TextureSelector RG_Tileview
syn keyword cssTagName M_Matview M_Plotter
syn keyword cssTagName SG_PaletteView SG_View
syn keyword cssTagName SK_View
syn keyword cssTagName VG_View

" Your own Agar Widget classes
"syn keyword cssTagName MY_Widget

" selectors
syn match cssSelectorOp "[,>+~]"
syn match cssSelectorOp2 "[~|^$*]\?=" contained
syn region cssAttributeSelector matchgroup=cssSelectorOp start="\[" end="]" contains=cssUnicodeEscape,cssSelectorOp2,cssStringQ,cssStringQQ

"try
"syn match cssIdentifier "#[A-Za-zÀ-ÿ_@][A-Za-zÀ-ÿ0-9_@-]*"
"catch /^.*/
"syn match cssIdentifier "#[A-Za-z_@][A-Za-z0-9_@-]*"
"endtry

" digits
syn match cssValueInteger contained "[-+]\=\d\+" contains=cssUnitDecorators
syn match cssValueNumber contained "[-+]\=\d\+\(\.\d*\)\=" contains=cssUnitDecorators
syn match cssValueLength contained "[-+]\=\d\+\(\.\d*\)\=\(mm\|cm\|in\|pt\|pc\|em\|ex\|px\|rem\|dpi\|dppx\|dpcm\|fr\|vw\|vh\|vmin\|vmax\|ch\)\>" contains=cssUnitDecorators
syn match cssValueLength contained "[-+]\=\d\+\(\.\d*\)\=%" contains=cssUnitDecorators
syn match cssValueAngle contained "[-+]\=\d\+\(\.\d*\)\=\(deg\|grad\|rad\)\>" contains=cssUnitDecorators
syn match cssValueTime contained "+\=\d\+\(\.\d*\)\=\(ms\|s\)\>" contains=cssUnitDecorators
syn match cssValueFrequency contained "+\=\d\+\(\.\d*\)\=\(Hz\|kHz\)\>" contains=cssUnitDecorators

" The 16 basic color names
syn keyword cssColor contained aqua black blue fuchsia gray green lime maroon navy olive purple red silver teal yellow white

" 130 more color names
syn keyword cssColor contained aliceblue antiquewhite aquamarine azure
syn keyword cssColor contained beige bisque blanchedalmond blueviolet brown burlywood
syn keyword cssColor contained cadetblue chartreuse chocolate coral cornflowerblue cornsilk crimson cyan
syn match cssColor contained /\<dark\(blue\|cyan\|goldenrod\|gray\|green\|grey\|khaki\)\>/
syn match cssColor contained /\<dark\(magenta\|olivegreen\|orange\|orchid\|red\|salmon\|seagreen\)\>/
syn match cssColor contained /\<darkslate\(blue\|gray\|grey\)\>/
syn match cssColor contained /\<dark\(turquoise\|violet\)\>/
syn keyword cssColor contained deeppink deepskyblue dimgray dimgrey dodgerblue firebrick
syn keyword cssColor contained floralwhite forestgreen gainsboro ghostwhite gold
syn keyword cssColor contained goldenrod greenyellow grey honeydew hotpink
syn keyword cssColor contained indianred indigo ivory khaki lavender lavenderblush lawngreen
syn keyword cssColor contained lemonchiffon limegreen linen magenta
syn match cssColor contained /\<light\(blue\|coral\|cyan\|goldenrodyellow\|gray\|green\)\>/
syn match cssColor contained /\<light\(grey\|pink\|salmon\|seagreen\|skyblue\|yellow\)\>/
syn match cssColor contained /\<light\(slategray\|slategrey\|steelblue\)\>/
syn match cssColor contained /\<medium\(aquamarine\|blue\|orchid\|purple\|seagreen\)\>/
syn match cssColor contained /\<medium\(slateblue\|springgreen\|turquoise\|violetred\)\>/
syn keyword cssColor contained midnightblue mintcream mistyrose moccasin navajowhite
syn keyword cssColor contained oldlace olivedrab orange orangered orchid
syn match cssColor contained /\<pale\(goldenrod\|green\|turquoise\|violetred\)\>/
syn keyword cssColor contained papayawhip peachpuff peru pink plum powderblue
syn keyword cssColor contained rosybrown royalblue rebeccapurple saddlebrown salmon
syn keyword cssColor contained sandybrown seagreen seashell sienna skyblue slateblue
syn keyword cssColor contained slategray slategrey snow springgreen steelblue tan
syn keyword cssColor contained thistle tomato turquoise violet wheat
syn keyword cssColor contained whitesmoke yellowgreen

syn match cssCustomProp contained "\<--[a-zA-Z0-9-_]*\>"

syn match cssColor contained "#\x\{3,4\}\>" contains=cssUnitDecorators
syn match cssColor contained "#\x\{6\}\>" contains=cssUnitDecorators
syn match cssColor contained "#\x\{16\}\>" contains=cssUnitDecorators

syn region cssMathGroup contained matchgroup=cssMathParens start="(" end=")" containedin=cssFunction,cssMathGroup contains=cssCustomProp,cssValue.*,cssFunction,cssColor,cssStringQ,cssStringQQ oneline

syn region cssFunction contained matchgroup=cssFunctionName start="\<\(rgb\|rgb16\|hsv\)(" end=")" oneline contains=cssValueInteger,cssValueNumber,cssFunctionComma
syn match cssFunctionComma contained ","

syn keyword cssCommonAttr contained auto none inherit all default normal

syn keyword cssBoxProp contained padding spacing margin

syn match cssColorProp contained "\<\(color\|background-color\|text-color\|line-color\|high-color\|low-color\|selection-color\)\(#disabled\|#focused\|#hover\|\)\>"

syn match cssFontProp contained "\<\(font-family\|font-size\|font-stretch\|font-style\|font-weight\)\=\>"

" font-family attributes (supported fonts)
syn case ignore
syn match cssFontAttr contained "\(Bitstream Charter\|Bitstream Vera\|Courier\|DejaVu Sans\|DejaVu Sans Mono\|DejaVu Serif\|Droid Sans \(Arabic\|Armenian\|Japanese\|Hebrew\|Mono\)\|Gentium Basic\|Gentium Book Basic\|Goha-Tibeb Zemen\|League Gothic\|League Spartan\|Noto Sans \(CJK SC\|SC\|Symbols\|Mono CJK SC\)\|Noto Mono\|Serto \(Jerusalem Outline\|Jerusalem\|Kharput\|Malankara\|Mardin\|Urhoy\)\|Utopia\)"

" font-family attributes (core fonts)
syn case match
syn keyword cssFontAttr contained algue unialgue monoalgue charter
syn keyword cssFontAttr contained league-spartan league-gothic fraktur
syn keyword cssFontAttr contained agar-minimal agar-ideograms

" font-stretch attributes
syn case ignore
syn keyword cssFontAttr contained ultracondensed condensed semicondensed
syn keyword cssFontAttr contained semiexpanded expanded ultraexpanded
" font-style attributes
syn keyword cssFontAttr contained normal italic oblique
" font-weight attributes
syn keyword cssFontAttr contained thin extralight light regular semibold bold
syn keyword cssFontAttr contained extrabold black
syn case match

syn match cssBraces contained "[{}]"
syn match cssError contained "{@<>"
syn region cssDefinition transparent matchgroup=cssBraces start='{' end='}' contains=cssTagName,cssAttributeSelector,cssAttrRegion,css.*Prop,cssComment,cssValue.*,cssColor,cssCustomProp,cssError,cssStringQ,cssStringQQ,cssFunction,cssUnicodeEscape,cssDefinition,cssNoise fold
syn match cssBraceError "}"
syn match cssAttrComma ","

" Numerical units
syn match cssUnitDecorators /\(#\|-\|+\|%\|mm\|cm\|in\|pt\|pc\|em\|ex\|px\|ch\|rem\|vh\|vw\|vmin\|vmax\|dpi\|dppx\|dpcm\|Hz\|kHz\|s\|ms\|deg\|grad\|rad\)/ contained
syn match cssNoise contained /\(:\|;\|\/\)/

" Comment
syn region cssComment start="/\*" end="\*/" contains=@Spell fold

syn match cssUnicodeEscape "\\\x\{1,6}\s\?"
syn match cssSpecialCharQQ +\\\\\|\\"+ contained
syn match cssSpecialCharQ +\\\\\|\\'+ contained
syn region cssStringQQ start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=cssUnicodeEscape,cssSpecialCharQQ
syn region cssStringQ start=+'+ skip=+\\\\\|\\'+ end=+'+ contains=cssUnicodeEscape,cssSpecialCharQ

" Some keywords are both Prop and Attr, so we have to handle them
syn region cssAttrRegion start=/:/ end=/\ze\(;\|)\|}\|{\)/ contained contains=css.*Attr,cssColor,cssValue.*,cssFunction,cssString.*,cssComment,cssUnicodeEscape,cssError,cssAttrComma,cssNoise

if main_syntax == "agarcss"
  syn sync minlines=10
endif

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

hi def link cssComment Comment
hi def link cssTagName Statement
"hi def link cssSelectorOp Special
"hi def link cssSelectorOp2 Special
hi def link cssAttrComma Special

hi def link cssBorderProp cssProp
hi def link cssBoxProp cssProp
hi def link cssColorProp cssProp
hi def link cssFontProp cssProp
hi def link cssListProp cssProp
hi def link cssTextProp cssProp

hi def link cssFontAttr cssAttr
hi def link cssGradientAttr cssAttr
hi def link cssCommonAttr cssAttr

hi def link cssValueLength Number
hi def link cssValueInteger Number
hi def link cssValueNumber Number
hi def link cssValueAngle Number
hi def link cssValueTime Number
hi def link cssValueFrequency Number
hi def link cssFunction Function
hi def link cssFunctionName Function
hi def link cssFunctionComma Function
hi def link cssColor Constant
"hi def link cssIdentifier Function
hi def link cssCustomProp Special
hi def link cssBraces Function
hi def link cssBraceError Error
hi def link cssError Error
hi def link cssUnicodeEscape Special
hi def link cssStringQQ String
hi def link cssStringQ String
hi def link cssAttributeSelector String
hi def link cssProp StorageClass
hi def link cssAttr Constant
hi def link cssUnitDecorators Number
hi def link cssNoise Noise

let b:current_syntax = "agarcss"

if main_syntax == 'agarcss'
  unlet main_syntax
endif

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=8
