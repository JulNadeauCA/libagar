" Vim syntax file
" Language:     LibAgar - Agar-Core C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarcore.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

source $VIMRUNTIME/syntax/c.vim

if !exists("c_no_agar_attributes") || exists("c_agar_attributes_typedefs")
  syn keyword cStorageClass _Nonnull _Nullable _Null_unspecified
  syn keyword cStorageClass _Nonnull_Mutex _Nonnull_Cond _Nonnull_Thread
  syn keyword cStorageClass _Nullable_Mutex _Nullable_Cond _Nullable_Thread
  syn keyword cStorageClass _Null_unspecified_Mutex _Null_unspecified_Cond
  syn keyword cStorageClass _Null_unspecified_Thread
  syn keyword cStorageClass _Aligned_Attribute _Alloc_Align_Attribute
  syn keyword cStorageClass _Alloc_Size_Attribute _Alloc_Size2_Attribute
  syn keyword cStorageClass _Const_Attribute DEPRECATED_ATTRIBUTE
  syn keyword cStorageClass FORMAT_ATTRIBUTE _Malloc_Like_Attribute
  syn keyword cStorageClass _Packed_Attribute _Pure_Attribute
  syn keyword cStorageClass _Pure_Attribute_If_Unthreaded _Section_Attribute
  syn keyword cStorageClass _Unused_Variable_Attribute _Used_Variable_Attribute
  syn keyword cStorageClass _Warn_Unused_Result _Weak_Attribute _Noreturn_Attribute
  syn keyword cStorageClass _Restrict
endif

if !exists("c_no_agar_core") || exists("c_agar_core_typedefs")
  " core/agsi.h (SGR attributes)
  syn keyword cConstant AGSI_RST AGSI_BOLD AGSI_FAINT AGSI_ITALIC AGSI_UNDERLINE
  syn keyword cConstant AGSI_REVERSE AGSI_CROSSEDOUT AGSI_FONT1 AGSI_FONT2
  syn keyword cConstant AGSI_FONT3 AGSI_FONT4 AGSI_FONT5 AGSI_FONT6 AGSI_FONT7
  syn keyword cConstant AGSI_FONT8 AGSI_FONT9 AGSI_FONT10 AGSI_FONT11
  syn keyword cConstant AGSI_FRAMED AGSI_ENCIRCLED AGSI_OVERLINED
  syn keyword cConstant AGSI_BLK AGSI_RED AGSI_GRN AGSI_YEL AGSI_BLU AGSI_MAG
  syn keyword cConstant AGSI_CYAN AGSI_WHT AGSI_BR_BLK AGSI_GRAY AGSI_BR_RED
  syn keyword cConstant AGSI_BR_GRN AGSI_BR_YEL AGSI_BR_BLU AGSI_BR_MAG
  syn keyword cConstant AGSI_BR_CYAN AGSI_BR_WHT AGSI_BLK_BG AGSI_RED_BG
  syn keyword cConstant AGSI_GRN_BG AGSI_YEL_BG AGSI_BLU_BG AGSI_MAG_BG
  syn keyword cConstant AGSI_CYAN_BG AGSI_WHT_BG AGSI_BR_BLK_BG AGSI_GRAY_BG
  syn keyword cConstant AGSI_BR_RED_BG AGSI_BR_GRN_BG AGSI_BR_YEL_BG AGSI_BR_BLU_BG 
  syn keyword cConstant AGSI_BR_MAG_BG AGSI_BR_CYAN_BG AGSI_BR_WHT_BG
  " core/agsi.h (core fonts)
  syn keyword cConstant AGSI_ALGUE AGSI_UNIALGUE AGSI_CMU_SANS AGSI_CMU_SERIF
  syn keyword cConstant AGSI_CMU_TYPEWRITER AGSI_CHARTER AGSI_COURIER_PRIME
  syn keyword cConstant AGSI_SOURCE_HAN_SANS AGSI_LEAGUE_SPARTAN
  syn keyword cConstant AGSI_LEAGUE_GOTHIC AGSI_UNIFRAKTUR_MAGUNTIA
  syn keyword cConstant AGSI_UNI AGSI_CM_SANS AGSI_PATH AGSI_CM_SERIF
  syn keyword cConstant AGSI_CM_TYPEWRITER AGSI_CODE AGSI_COURIER AGSI_SOURCE_HAN 
  syn keyword cConstant AGSI_CJK AGSI_FRAKTUR AGSI_FRAK AGSI_CMD AGSI_CMD_MOD
  " core/agsi.h (general punctuation)
  syn keyword cConstant AGSI_THIN_SPACE AGSI_HYPHEN AGSI_NON_BREAKING_HYPHEN
  syn keyword cConstant AGSI_FIGURE_DASH AGSI_EN_DASH AGSI_EM_DASH
  syn keyword cConstant AGSI_HORIZONTAL_BAR AGSI_DOUBLE_VERTICAL_LINE
  syn keyword cConstant AGSI_DOUBLE_LOW_LINE AGSI_QUOTE_LEFT AGSI_QUOTE_RIGHT
  syn keyword cConstant AGSI_QUOTE_BASE AGSI_QUOTE_REVERSED
  syn keyword cConstant AGSI_DBLQUOTE_LEFT AGSI_DBLQUOTE_RIGHT AGSI_DBLQUOTE_REVERSED
  syn keyword cConstant AGSI_DAGGER AGSI_DBLDAGGER AGSI_TRIANGULAR_BULLET
  syn keyword cConstant AGSI_ONE_DOT_LEADER AGSI_TWO_DOT_LEADER AGSI_ELLIPSIS
  syn keyword cConstant AGSI_HYPHENATION_POINT AGSI_PER_THOUSAND AGSI_PER_TEN_THOUSAND
  syn keyword cConstant AGSI_PRIME AGSI_DOUBLE_PRIME AGSI_MINUTE AGSI_SECOND
  syn keyword cConstant AGSI_TRIPLE_PRIME AGSI_REVERSED_PRIME
  syn keyword cConstant AGSI_REVERSED_DOUBLE_PRIME AGSI_REVERSED_TRIPLE_PRIME
  syn keyword cConstant AGSI_CARET AGSI_GUILSINGL_LEFT AGSI_GUILSINGL_RIGHT
  syn keyword cConstant AGSI_REFERENCE_MARK AGSI_EXCLAM_DBL AGSI_INTERROBANG
  syn keyword cConstant AGSI_OVERLINE AGSI_UNDERTIE AGSI_CHARACTER_TIE
  syn keyword cConstant AGSI_CARET_INSERTION_POINT AGSI_ASTERISM AGSI_HYPHEN_BULLET
  syn keyword cConstant AGSI_FRACTION_SLASH AGSI_L_SQ_BRACKET_W_QUILL AGSI_R_SQ_BRACKET_W_QUILL
  syn keyword cConstant AGSI_DOUBLE_QUESTION AGSI_QUESTION_EXCLAMATION AGSI_EXCLAMATION_QUESTION
  syn keyword cConstant AGSI_TIRONIAN_SIGN_ET AGSI_REVERSED_PILCROW_SIGN
  syn keyword cConstant AGSI_BLACK_LEFTWARDS_BULLET AGSI_BLACK_RIGHTWARDS_BULLET
  syn keyword cConstant AGSI_LOW_ASTERISK AGSI_REVERSED_SEMICOLON AGSI_CLOSE_UP
  syn keyword cConstant AGSI_TWO_ASTERISKS_VALIGNED AGSI_COMMERCIAL_MINUS_SIGN
  syn keyword cConstant AGSI_SWUNG_DASH AGSI_INVERTED_UNDERTIE AGSI_FLOWER_PUNCTUATION
  syn keyword cConstant AGSI_THREE_DOT_PUNCTUATION AGSI_QUADRUPLE_PRIME
  syn keyword cConstant AGSI_FOUR_DOT_PUNCTUATION AGSI_FIVE_DOT_PUNCTUATION
  syn keyword cConstant AGSI_TWO_DOT_PUNCTUATION AGSI_FOUR_DOTS AGSI_DOTTED_CROSS
  syn keyword cConstant AGSI_TRICOLON AGSI_VERTICAL_FOUR_DOTS AGSI_MEDIUM_MATH_SPACE
  syn keyword cConstant AGSI_WORD_JOINER AGSI_FUNCTION_APPLICATION AGSI_INVISIBLE_TIMES
  syn keyword cConstant AGSI_INVISIBLE_SEPARATOR AGSI_INVISIBLE_PLUS
  syn keyword cConstant AGSI_INH_SYMMETRIC_SWAPPING AGSI_ACT_SYMMETRIC_SWAPPING
  syn keyword cConstant AGSI_INH_ARABIC_FORM_SHAPING AGSI_ACT_ARABIC_FORM_SHAPING
  syn keyword cConstant AGSI_NATIONAL_DIGIT_SHAPES
  " core/agsi.h (superscripts and subscripts)
  syn keyword cConstant AGSI_SUPERSCRIPT_0 AGSI_SUPERSCRIPT_SMALL_I
  syn keyword cConstant AGSI_SUPERSCRIPT_4 AGSI_SUPERSCRIPT_5 AGSI_SUPERSCRIPT_6
  syn keyword cConstant AGSI_SUPERSCRIPT_7 AGSI_SUPERSCRIPT_8 AGSI_SUPERSCRIPT_9
  syn keyword cConstant AGSI_SUPERSCRIPT_PLUS AGSI_SUPERSCRIPT_MINUS
  syn keyword cConstant AGSI_SUPERSCRIPT_EQUALS AGSI_SUPERSCRIPT_LEFT_PAREN
  syn keyword cConstant AGSI_SUPERSCRIPT_RIGHT_PAREN AGSI_SUPERSCRIPT_SMALL_N
  syn keyword cConstant AGSI_SUBSCRIPT_0 AGSI_SUBSCRIPT_1 AGSI_SUBSCRIPT_2
  syn keyword cConstant AGSI_SUBSCRIPT_3 AGSI_SUBSCRIPT_4 AGSI_SUBSCRIPT_5
  syn keyword cConstant AGSI_SUBSCRIPT_6 AGSI_SUBSCRIPT_7 AGSI_SUBSCRIPT_8
  syn keyword cConstant AGSI_SUBSCRIPT_9 AGSI_SUBSCRIPT_PLUS AGSI_SUBSCRIPT_MINUS
  syn keyword cConstant AGSI_SUBSCRIPT_EQUALS AGSI_SUBSCRIPT_LEFT_PAREN
  syn keyword cConstant AGSI_SUBSCRIPT_RIGHT_PAREN AGSI_SUBSCRIPT_SMALL_A
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_E AGSI_SUBSCRIPT_SMALL_O
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_X AGSI_SUBSCRIPT_SMALL_H
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_K AGSI_SUBSCRIPT_SMALL_L
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_M AGSI_SUBSCRIPT_SMALL_N
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_P AGSI_SUBSCRIPT_SMALL_S
  syn keyword cConstant AGSI_SUBSCRIPT_SMALL_T
  " core/agsi.h (number forms)
  syn keyword cConstant AGSI_ROMAN_NUMERAL_1 AGSI_ROMAN_NUMERAL_2
  syn keyword cConstant AGSI_ROMAN_NUMERAL_3 AGSI_ROMAN_NUMERAL_4
  syn keyword cConstant AGSI_ROMAN_NUMERAL_5 AGSI_ROMAN_NUMERAL_6
  syn keyword cConstant AGSI_ROMAN_NUMERAL_7 AGSI_ROMAN_NUMERAL_8
  syn keyword cConstant AGSI_ROMAN_NUMERAL_9 AGSI_ROMAN_NUMERAL_10
  syn keyword cConstant AGSI_ROMAN_NUMERAL_11 AGSI_ROMAN_NUMERAL_12
  syn keyword cConstant AGSI_ROMAN_NUMERAL_50 AGSI_ROMAN_NUMERAL_100
  syn keyword cConstant AGSI_ROMAN_NUMERAL_500 AGSI_ROMAN_NUMERAL_1000
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_1 AGSI_SMALL_ROMAN_NUMERAL_2
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_3 AGSI_SMALL_ROMAN_NUMERAL_4
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_5 AGSI_SMALL_ROMAN_NUMERAL_6
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_7 AGSI_SMALL_ROMAN_NUMERAL_8
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_9 AGSI_SMALL_ROMAN_NUMERAL_10
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_11 AGSI_SMALL_ROMAN_NUMERAL_12
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_50 AGSI_SMALL_ROMAN_NUMERAL_100
  syn keyword cConstant AGSI_SMALL_ROMAN_NUMERAL_500 AGSI_SMALL_ROMAN_NUMERAL_1000
  syn keyword cConstant AGSI_ROMAN_NUMERAL_1000_C_D AGSI_ROMAN_NUMERAL_5000
  syn keyword cConstant AGSI_ROMAN_NUMERAL_10000 AGSI_ROMAN_NUMERAL_REVERSED_100
  syn keyword cConstant AGSI_LATIN_SMALL_REVERSED_C AGSI_ROMAN_NUMERAL_6_LATE_FORM
  " core/agsi.h (arrows)
  syn keyword cConstant AGSI_ARROW_LEFT AGSI_ARROW_UP AGSI_ARROW_RIGHT
  syn keyword cConstant AGSI_ARROW_DOWN AGSI_ARROW_LEFT_RIGHT AGSI_ARROW_UP_DOWN
  syn keyword cConstant AGSI_ARROW_NORTH_WEST AGSI_ARROW_NORTH_EAST
  syn keyword cConstant AGSI_ARROW_SOUTH_EAST AGSI_ARROW_SOUTH_WEST
  " core/agsi.h (mathematical operators)
  syn keyword cConstant AGSI_ASYMPTOMATICALLY_EQUAL_TO
  syn keyword cConstant AGSI_NOT_ASYMPTOMATICALLY_EQUAL_TO
  " TODO
 
  " core/agsi.h (agar glyphs)
  syn keyword cConstant AGSI_BLACK_AGAR AGSI_WHITE_AGAR AGSI_MENUBOOL_TRUE
  syn keyword cConstant AGSI_MENUBOOL_FALSE AGSI_KEYMOD_HYPHEN AGSI_MENU_EXPANDER
  syn keyword cConstant AGSI_BOX_VERT AGSI_BOX_HORIZ AGSI_BUTTON
  " core/agtime.h
  syn keyword cType AG_Timer AG_TimerFn AG_TimerPvt AG_TimeOps
  syn keyword cConstant AG_TIMER_NAME_MAX AG_TIMER_SURVIVE_DETACH
  syn keyword cConstant AG_TIMER_AUTO_FREE AG_TIMER_EXECD AG_TIMER_RESTART
  " core/begin.h
  syn keyword cConstant AG_INLINE AG_NULL
  " core/config.h
  syn keyword cType AG_Config AG_ConfigPathGroup AG_ConfigPath AG_ConfigPathQ
  syn keyword cConstant AG_CONFIG_PATH_DATA AG_CONFIG_PATH_FONTS
  syn keyword cConstant AG_CONFIG_PATH_TEMP AG_CONFIG_PATH_LAST
  " core/core.h
  syn keyword cConstant AG_BYTEORDER AG_BIG_ENDIAN AG_LITTLE_ENDIAN
  " core/core_init.h
  syn keyword cConstant AG_VERBOSE AG_CREATE_DATADIR AG_SOFT_TIMERS
  syn keyword cConstant AG_POSIX_USERS AG_MEMORY_MODEL_NAME
  syn keyword cConstant AG_PATHSEP AG_PATHSEPCHAR AG_PATHSEPMULTI
  " core/cpuinfo.h
  syn keyword cType AG_CPUInfo
  syn keyword cConstant AG_EXT_CPUID AG_EXT_MMX AG_EXT_MMX_EXT AG_EXT_3DNOW
  syn keyword cConstant AG_EXT_3DNOW_EXT AG_EXT_ALTIVEC AG_EXT_SSE AG_EXT_SSE2 
  syn keyword cConstant AG_EXT_SSE3 AG_EXT_LONG_MODE AG_EXT_RDTSCP AG_EXT_FXSR 
  syn keyword cConstant AG_EXT_PAGE_NX AG_EXT_SSE5A AG_EXT_3DNOW_PREFETCH
  syn keyword cConstant AG_EXT_SSE_MISALIGNED AG_EXT_SSE4A AG_EXT_ONCHIP_FPU
  syn keyword cConstant AG_EXT_TSC AG_EXT_CMOV AG_EXT_CLFLUSH AG_EXT_HTT
  syn keyword cConstant AG_EXT_MON AG_EXT_VMX AG_EXT_SSSE3 AG_EXT_SSE41
  syn keyword cConstant AG_EXT_SSE42
  " core/data_source.h
  syn keyword cType AG_ByteOrder AG_DataSource AG_FileSource AG_CoreSource
  syn keyword cType AG_ConstCoreSource AG_NetSocketSource
  syn keyword cConstant AG_BYTEORDER_BE AG_BYTEORDER_LE AG_SEEK_SET AG_SEEK_CUR
  syn keyword cConstant AG_SEEK_END AG_SOURCE_UINT8 AG_SOURCE_SINT8
  syn keyword cConstant AG_SOURCE_UINT16 AG_SOURCE_SINT16 AG_SOURCE_UINT32
  syn keyword cConstant AG_SOURCE_SINT32 AG_SOURCE_UINT64 AG_SOURCE_SINT64 
  syn keyword cConstant AG_SOURCE_FLOAT AG_SOURCE_DOUBLE AG_SOURCE_LONG_DOUBLE 
  syn keyword cConstant AG_SOURCE_STRING AG_SOURCE_COLOR_RGBA AG_SOURCE_STRING_PAD
  " core/db.h
  syn keyword cType AG_Db AG_DbClass AG_Dbt AG_DbHashBT AG_DbMySQL AG_DbIterateFn
  syn keyword cConstant AG_DB_KEY_DATA AG_DB_KEY_NUMBER AG_DB_KEY_STRING
  syn keyword cConstant AG_DB_REC_VARIABLE AG_DB_REC_FIXED AG_DB_OPEN AG_DB_READONLY
  " core/dbobject.h
  syn keyword cType AG_DbObject
  " core/dir.h
  syn keyword cType AG_Dir
  " core/dso.h
  syn keyword cType AG_DSOSym AG_DSO AG_DSO_BeOS AG_DSO_OS2 AG_DSO_Generic
  syn keyword cConstant AG_DSONAME_MAX
  " core/error.h
  syn keyword cType AG_ErrorCode
  syn keyword cConstant AG_EUNDEFINED AG_EPERM AG_ENOENT AG_EINTR AG_EIO AG_E2BIG 
  syn keyword cConstant AG_EACCESS AG_EBUSY AG_EEXIST AG_ENOTDIR AG_EISDIR 
  syn keyword cConstant AG_EMFILE AG_EFBIG AG_ENOSPC AG_EROFS AG_EAGAIN
  " core/event.h
  syn keyword cType AG_Event AG_EventFn AG_Function AG_EventSource AG_EventSink
  syn keyword cType AG_VecVoid AG_VecString AG_VecInt AG_VecChar AG_VecFloat 
  syn keyword cType AG_VecDouble
  syn keyword cConstant AG_SINK_NONE AG_SINK_PROLOGUE AG_SINK_EPILOGUE
  syn keyword cConstant AG_SINK_SPINNER AG_SINK_TERMINATOR AG_SINK_TIMER
  syn keyword cConstant AG_SINK_READ AG_SINK_WRITE AG_SINK_FSEVENT AG_SINK_PROCEVENT
  syn keyword cConstant AG_SINK_LAST AG_EVENT_ARGS_MAX AG_EVENT_NAME_MAX
  syn keyword cConstant AG_FSEVENT_DELETE AG_FSEVENT_WRITE AG_FSEVENT_EXTEND
  syn keyword cConstant AG_FSEVENT_ATTRIB AG_FSEVENT_LINK AG_FSEVENT_RENAME
  syn keyword cConstant AG_FSEVENT_REVOKE AG_PROCEVENT_EXIT AG_PROCEVENT_FORK
  syn keyword cConstant AG_PROCEVENT_EXEC
  " core/event.c
  syn keyword cType AG_EventSourceKQUEUE
  " core/exec.h
  syn keyword cConstant AG_EXEC_WAIT_IMMEDIATE AG_EXEC_WAIT_INFINITE
  " core/file.h
  syn keyword cType AG_FileInfo AG_FileExtMapping
  syn keyword cConstant AG_FILE_REGULAR AG_FILE_DIRECTORY AG_FILE_DEVICE
  syn keyword cConstant AG_FILE_FIFO AG_FILE_SYMLINK AG_FILE_SOCKET
  syn keyword cConstant AG_FILE_READABLE AG_FILE_WRITEABLE AG_FILE_EXECUTABLE
  syn keyword cConstant AG_FILE_SUID AG_FILE_SGID AG_FILE_ARCHIVE 
  syn keyword cConstant AG_FILE_HIDDEN AG_FILE_TEMPORARY AG_FILE_SYSTEM
  " core/limits.h
  syn keyword cConstant AG_FILENAME_MAX AG_PATHNAME_MAX AG_ARG_MAX AG_BUFFER_MIN 
  syn keyword cConstant AG_BUFFER_MAX AG_INT_MIN AG_INT_MAX AG_UINT_MIN
  syn keyword cConstant AG_UINT_MAX AG_FLT_MIN AG_FLT_MAX AG_DBL_MIN AG_DBL_MAX
  syn keyword cConstant AG_LDBL_MIN AG_LDBL_MAX AG_LONG_MIN AG_LONG_MAX AG_ULONG_MIN 
  syn keyword cConstant AG_ULONG_MAX AG_ULLONG_MAX AG_LLONG_MIN AG_LLONG_MAX
  syn keyword cConstant AG_FLT_EPSILON AG_DBL_EPSILON
  " core/load_string.h
  syn keyword cType AG_FmtStringExtFn AG_FmtStringExt
  syn keyword cConstant AG_LOAD_STRING_MAX
  " core/load_version.h
  syn keyword cType AG_Version
  syn keyword cConstant AG_VERSION_NAME_MAX AG_VERSION_MAX
  " core/object.h
  syn keyword cType AG_Object AG_ObjectHeader AG_ObjectPvt AG_ObjectClass
  syn keyword cType AG_ObjectClassPvt AG_ObjectClassSpec AG_ObjectInitFn
  syn keyword cType AG_ObjectResetFn AG_ObjectDestroyFn AG_ObjectLoadFn
  syn keyword cType AG_ObjectSaveFn AG_ObjectEditFn AG_Namespace
  syn keyword cConstant AG_OBJECT_NAME_MAX AG_OBJECT_TYPE_MAX AG_OBJECT_HIER_MAX 
  syn keyword cConstant AG_OBJECT_PATH_MAX AG_OBJECT_LIBS_MAX AG_OBJECT_CLASSTBLSIZE
  syn keyword cConstant AG_OBJECT_MAX_VARIABLES AG_OBJECT_TYPE_TAG AG_OBJECT_TYPE_TAG_LEN
  syn keyword cConstant AG_OBJECT_FLOATING_VARS AG_OBJECT_NON_PERSISTENT
  syn keyword cConstant AG_OBJECT_INDESTRUCTIBLE AG_OBJECT_RESIDENT
  syn keyword cConstant AG_OBJECT_STATIC AG_OBJECT_READONLY AG_OBJECT_WAS_RESIDENT
  syn keyword cConstant AG_OBJECT_REOPEN_ONLOAD AG_OBJECT_REMAIN_DATA
  syn keyword cConstant AG_OBJECT_DEBUG AG_OBJECT_NAME_ONATTACH
  syn keyword cConstant AG_OBJECT_CHLD_AUTOSAVE AG_OBJECT_DEBUG_DATA
  syn keyword cConstant AG_OBJECT_INATTACH AG_OBJECT_INDETACH
  syn keyword cConstant AG_OBJECT_BOUND_EVENTS AG_OBJECT_SAVED_FLAGS
  " core/queue.h
  syn keyword cConstant AG_SLIST_HEAD AG_SLIST_HEAD_ AG_SLIST_ENTRY
  syn keyword cConstant AG_LIST_HEAD AG_LIST_HEAD_ AG_LIST_ENTRY
  syn keyword cConstant AG_SIMPLEQ_HEAD AG_SIMPLEQ_HEAD_ AG_SIMPLEQ_ENTRY
  syn keyword cConstant AG_TAILQ_HEAD AG_TAILQ_HEAD_ AG_TAILQ_ENTRY
  syn keyword cConstant AG_CIRCLEQ_HEAD AG_CIRCLEQ_HEAD_ AG_CIRCLEQ_ENTRY
  syn keyword cConstant SLIST_HEAD SLIST_HEAD_ SLIST_ENTRY
  syn keyword cConstant LIST_HEAD LIST_HEAD_ LIST_ENTRY
  syn keyword cConstant SIMPLEQ_HEAD SIMPLEQ_HEAD_ SIMPLEQ_ENTRY
  syn keyword cConstant TAILQ_HEAD TAILQ_HEAD_ TAILQ_ENTRY
  syn keyword cConstant CIRCLEQ_HEAD CIRCLEQ_HEAD_ CIRCLEQ_ENTRY
  " core/options.h
  syn keyword cConstant AG_ANSI_COLOR AG_DEBUG AG_ENABLE_DSO AG_ENABLE_EXEC
  syn keyword cConstant AG_ENABLE_STRING AG_EVENT_LOOP AG_LEGACY AG_NAMED_ARGS
  syn keyword cConstant AG_NAMESPACES AG_SERIALIZATION AG_THREADS AG_TIMERS
  syn keyword cConstant AG_TYPE_SAFETY AG_UNICODE AG_USER AG_VERBOSITY
  syn keyword cConstant AG_WIDGETS AG_WM_HINTS
  " core/string.h
  syn keyword cType AG_NewlineFormat AG_FmtString AG_FmtExtension
  syn keyword cConstant AG_STRING_BUFFERS_MAX AG_STRING_POINTERS_MAX
  syn keyword cConstant AG_FMTSTRING_BUFFER_INIT AG_FMTSTRING_BUFFER_GROW
  syn keyword cConstant AG_NEWLINE_NATIVE AG_NEWLINE_LF AG_NEWLINE_CR_LF 
  syn keyword cConstant AG_NEWLINE_CR AG_NEWLINE_LF_CR AG_NEWLINE_ATA_CR 
  syn keyword cConstant AG_NEWLINE_EBCDIC AG_NEWLINE_LAST AG_NEWLINE_DOS 
  syn keyword cConstant AG_NEWLINE_UNIX
  " core/tbl.h
  syn keyword cType AG_Tbl AG_TblBucket
  syn keyword cConstant AG_TBL_DUPLICATES
  " core/text.h
  syn keyword cType AG_Language AG_Text AG_TextElement AG_TextEnt
  syn keyword cConstant AG_LANG_NONE AG_LANG_AA AG_LANG_AB AG_LANG_AF AG_LANG_AM
  syn keyword cConstant AG_LANG_AR AG_LANG_AS AG_LANG_AY AG_LANG_AZ AG_LANG_BA
  syn keyword cConstant AG_LANG_BE AG_LANG_BG AG_LANG_BH AG_LANG_BI AG_LANG_BN
  syn keyword cConstant AG_LANG_BO AG_LANG_BR AG_LANG_CA AG_LANG_CO AG_LANG_CS
  syn keyword cConstant AG_LANG_CY AG_LANG_DA AG_LANG_DE AG_LANG_DZ AG_LANG_EL
  syn keyword cConstant AG_LANG_EN AG_LANG_EO AG_LANG_ES AG_LANG_ET AG_LANG_EU
  syn keyword cConstant AG_LANG_FA AG_LANG_FI AG_LANG_FJ AG_LANG_FO AG_LANG_FR
  syn keyword cConstant AG_LANG_FY AG_LANG_GA AG_LANG_GD AG_LANG_GL AG_LANG_GN
  syn keyword cConstant AG_LANG_GU AG_LANG_HA AG_LANG_HE AG_LANG_HI AG_LANG_HR
  syn keyword cConstant AG_LANG_HU AG_LANG_HY AG_LANG_IA AG_LANG_ID AG_LANG_IE
  syn keyword cConstant AG_LANG_IK AG_LANG_IS AG_LANG_IT AG_LANG_IU AG_LANG_JA
  syn keyword cConstant AG_LANG_JW AG_LANG_KA AG_LANG_KK AG_LANG_KL AG_LANG_KM
  syn keyword cConstant AG_LANG_KN AG_LANG_KO AG_LANG_KS AG_LANG_KU AG_LANG_KY
  syn keyword cConstant AG_LANG_LA AG_LANG_LN AG_LANG_LO AG_LANG_LT AG_LANG_LV
  syn keyword cConstant AG_LANG_MG AG_LANG_MI AG_LANG_MK AG_LANG_ML AG_LANG_MN
  syn keyword cConstant AG_LANG_MO AG_LANG_MR AG_LANG_MS AG_LANG_MT AG_LANG_MY
  syn keyword cConstant AG_LANG_NA AG_LANG_NE AG_LANG_NL AG_LANG_NO AG_LANG_OC
  syn keyword cConstant AG_LANG_OM AG_LANG_OR AG_LANG_PA AG_LANG_PL AG_LANG_PS
  syn keyword cConstant AG_LANG_PT AG_LANG_QU AG_LANG_RM AG_LANG_RN AG_LANG_RO
  syn keyword cConstant AG_LANG_RU AG_LANG_RW AG_LANG_SA AG_LANG_SD AG_LANG_SG
  syn keyword cConstant AG_LANG_SH AG_LANG_SI AG_LANG_SK AG_LANG_SL AG_LANG_SM
  syn keyword cConstant AG_LANG_SN AG_LANG_SO AG_LANG_SQ AG_LANG_SR AG_LANG_SS
  syn keyword cConstant AG_LANG_ST AG_LANG_SU AG_LANG_SV AG_LANG_SW AG_LANG_TA
  syn keyword cConstant AG_LANG_TE AG_LANG_TG AG_LANG_TH AG_LANG_TI AG_LANG_TK
  syn keyword cConstant AG_LANG_TL AG_LANG_TN AG_LANG_TO AG_LANG_TR AG_LANG_TS
  syn keyword cConstant AG_LANG_TT AG_LANG_TW AG_LANG_UG AG_LANG_UK AG_LANG_UR
  syn keyword cConstant AG_LANG_UZ AG_LANG_VI AG_LANG_VO AG_LANG_WO AG_LANG_XH
  syn keyword cConstant AG_LANG_YI AG_LANG_YO AG_LANG_ZA AG_LANG_ZH AG_LANG_ZU
  syn keyword cConstant AG_LANG_LAST AG_TEXT_SAVED_FLAGS
  " core/threads.h
  syn keyword cType AG_Mutex AG_MutexAttr AG_Thread AG_Cond AG_ThreadKey
  syn keyword cConstant AG_MUTEX_INITIALIZER
  " core/types.h
  syn keyword cType AG_Size AG_Offset AG_Char Uchar Uint Ulong
  syn keyword cConstant AG_MODEL AG_SMALL AG_MEDIUM AG_LARGE AG_HAVE_64BIT 
  syn keyword cConstant AG_HAVE_FLOAT AG_CHAR_MAX AG_SIZE_MAX AG_OFFS_MAX
  syn keyword cConstant AG_SIZE_PADDING AG_OFFSET_PADDING
  " core/user.h
  syn keyword cType AG_User AG_UserOps AG_UserList
  syn keyword cConstant AG_USER_NAME_MAX AG_USER_NO_ACCOUNT
  " core/variable.h
  syn keyword cType AG_Variable AG_VariableType AG_VariableTypeInfo
  syn keyword cType AG_VoidFn AG_UintFn AG_IntFn AG_Uint8Fn AG_Sint8Fn 
  syn keyword cType AG_Uint16Fn AG_Sint16Fn AG_Uint32Fn AG_Sint32Fn 
  syn keyword cType AG_Uint64Fn AG_Sint64Fn AG_FloatFn AG_DoubleFn AG_LongDoubleFn 
  syn keyword cType AG_StringFn AG_LongFn AG_UlongFn AG_PointerFn AG_ConstPointerFn
  syn keyword cConstant AG_VARIABLE_NULL AG_VARIABLE_UINT AG_VARIABLE_P_UINT
  syn keyword cConstant AG_VARIABLE_INT AG_VARIABLE_P_INT AG_VARIABLE_ULONG
  syn keyword cConstant AG_VARIABLE_P_ULONG AG_VARIABLE_LONG AG_VARIABLE_P_LONG
  syn keyword cConstant AG_VARIABLE_UINT8 AG_VARIABLE_P_UINT8 AG_VARIABLE_SINT8 AG_VARIABLE_P_SINT8
  syn keyword cConstant AG_VARIABLE_UINT16 AG_VARIABLE_P_UINT16 AG_VARIABLE_SINT16 AG_VARIABLE_P_SINT16
  syn keyword cConstant AG_VARIABLE_UINT32 AG_VARIABLE_P_UINT32 AG_VARIABLE_SINT32 AG_VARIABLE_P_SINT32
  syn keyword cConstant AG_VARIABLE_UINT64 AG_VARIABLE_P_UINT64 AG_VARIABLE_SINT64 AG_VARIABLE_P_SINT64
  syn keyword cConstant AG_VARIABLE_FLOAT AG_VARIABLE_P_FLOAT AG_VARIABLE_DOUBLE AG_VARIABLE_P_DOUBLE
  syn keyword cConstant AG_VARIABLE_STRING AG_VARIABLE_P_STRING AG_VARIABLE_POINTER AG_VARIABLE_P_POINTER
  syn keyword cConstant AG_VARIABLE_P_FLAG AG_VARIABLE_P_FLAG8 AG_VARIABLE_P_FLAG16 AG_VARIABLE_P_FLAG32
  syn keyword cConstant AG_VARIABLE_P_OBJECT AG_VARIABLE_P_VARIABLE
  syn keyword cConstant AG_VARIABLE_FUNCTION AG_VARIABLE_TYPE_LAST AG_VARIABLE_BOOL
  syn keyword cConstant AG_VARIABLE_NAME_MAX AG_VARIABLE_P_READONLY
  syn keyword cConstant AG_VARIABLE_P_FREE AG_VARIABLE_P_SENDER
  " core/vec.h
  syn keyword cConstant AG_VEC_HEAD
  " core/version.h
  syn keyword cType AG_AgarVersion
  syn keyword cConstant AGAR_MAJOR_VERSION AGAR_MINOR_VERSION AGAR_PATCHLEVEL
  " tests/agartest.h
  syn keyword cType AG_TestCase AG_TestInstance
endif
