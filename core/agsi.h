/*	Public domain	*/

/*
 * Common ANSI escape sequences.
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 */

#ifdef AG_ANSI_COLOR

# define AGSI_RST	 "\x1b[0m"   /* reset all attributes */
# define AGSI_BOLD     	 "\x1b[1m"   /* bold */
# define AGSI_FAINT    	 "\x1b[2m"   /* faint or semibold */
# define AGSI_ITALIC   	 "\x1b[3m"   /* italic style */
# define AGSI_UNDERLINE	 "\x1b[4m"   /* underlined */
# define AGSI_REVERSE  	 "\x1b[7m"   /* reverse video */
# define AGSI_CROSSEDOUT "\x1b[9m"   /* crossed-out */

# define AGSI_FONT1	 "\x1b[10m"  /* Algue */
# define AGSI_FONT2	 "\x1b[11m"  /* Unialgue */
# define AGSI_FONT3	 "\x1b[12m"  /* CMU Sans Serif */
# define AGSI_FONT4	 "\x1b[13m"  /* CMU Serif */
# define AGSI_FONT5	 "\x1b[14m"  /* CMU Typewriter */
# define AGSI_FONT6	 "\x1b[15m"  /* Charter */
# define AGSI_FONT7	 "\x1b[16m"  /* Courier Prime */
# define AGSI_FONT8	 "\x1b[17m"  /* Source Han Sans */
# define AGSI_FONT9	 "\x1b[18m"  /* League Spartan */
# define AGSI_FONT10	 "\x1b[19m"  /* League Gothic */
# define AGSI_FONT11	 "\x1b[20m"  /* Unifraktur Maguntia */

# define AGSI_FRAMED	 "\x1b[51m"  /* render frame */
# define AGSI_ENCIRCLED  "\x1b[52m"  /* render encircled */
# define AGSI_OVERLINED  "\x1b[53m"  /* overlined */
# define AGSI_BLK	 "\x1b[30m"  /* black */
# define AGSI_RED	 "\x1b[31m"  /* red */
# define AGSI_GRN	 "\x1b[32m"  /* green */
# define AGSI_YEL	 "\x1b[33m"  /* yellow */
# define AGSI_BLU	 "\x1b[34m"  /* blue */
# define AGSI_MAG	 "\x1b[35m"  /* magenta */
# define AGSI_CYAN	 "\x1b[36m"  /* cyan */
# define AGSI_WHT	 "\x1b[37m"  /* white */
# define AGSI_BR_BLK	 "\x1b[90m"  /* bright black */
# define AGSI_GRAY       AGSI_BR_BLK
# define AGSI_BR_RED	 "\x1b[91m"  /* bright red */
# define AGSI_BR_GRN	 "\x1b[92m"  /* bright green */
# define AGSI_BR_YEL	 "\x1b[93m"  /* bright yellow */
# define AGSI_BR_BLU	 "\x1b[94m"  /* bright blue */
# define AGSI_BR_MAG	 "\x1b[95m"  /* bright magenta */
# define AGSI_BR_CYAN	 "\x1b[96m"  /* bright cyan */
# define AGSI_BR_WHT	 "\x1b[97m"  /* bright white */
# define AGSI_BLK_BG	 "\x1b[40m"  /* black background */
# define AGSI_RED_BG	 "\x1b[41m"  /* red background */
# define AGSI_GRN_BG	 "\x1b[42m"  /* green background */
# define AGSI_YEL_BG	 "\x1b[43m"  /* yellow background */
# define AGSI_BLU_BG	 "\x1b[44m"  /* blue background */
# define AGSI_MAG_BG	 "\x1b[45m"  /* magenta background */
# define AGSI_CYAN_BG	 "\x1b[46m"  /* cyan background */
# define AGSI_WHT_BG	 "\x1b[47m"  /* white background */
# define AGSI_BR_BLK_BG	 "\x1b[100m" /* bright black background */
# define AGSI_GRAY_BG	 AGSI_BR_BLK_BG
# define AGSI_BR_RED_BG	 "\x1b[101m" /* bright red background */
# define AGSI_BR_GRN_BG	 "\x1b[102m" /* bright green background */
# define AGSI_BR_YEL_BG	 "\x1b[103m" /* bright yellow background */
# define AGSI_BR_BLU_BG	 "\x1b[104m" /* bright blue background */
# define AGSI_BR_MAG_BG	 "\x1b[105m" /* bright magenta background */
# define AGSI_BR_CYAN_BG "\x1b[106m" /* bright cyan background */
# define AGSI_BR_WHT_BG	 "\x1b[107m" /* bright white background */

#else /* !AG_ANSI_COLOR */

# define AGSI_RST 	 ""
# define AGSI_BOLD     	 ""
# define AGSI_FAINT    	 ""
# define AGSI_ITALIC   	 ""
# define AGSI_UNDERLINE	 ""
# define AGSI_REVERSE  	 ""
# define AGSI_CROSSEDOUT ""
# define AGSI_FONT1	 ""
# define AGSI_FONT2	 ""
# define AGSI_FONT3	 ""
# define AGSI_FONT4	 ""
# define AGSI_FONT5	 ""
# define AGSI_FONT6	 ""
# define AGSI_FONT7	 ""
# define AGSI_FONT8	 ""
# define AGSI_FONT9	 ""
# define AGSI_FONT10	 ""
# define AGSI_FONT11	 ""
# define AGSI_FRAMED	 ""
# define AGSI_ENCIRCLED  ""
# define AGSI_OVERLINED  ""
# define AGSI_BLK	 ""
# define AGSI_RED	 ""
# define AGSI_GRN	 ""
# define AGSI_YEL	 ""
# define AGSI_BLU	 ""
# define AGSI_MAG	 ""
# define AGSI_CYA	 ""
# define AGSI_WHT	 ""
# define AGSI_BR_BLK	 ""
# define AGSI_BR_RED	 ""
# define AGSI_BR_GRN	 ""
# define AGSI_BR_YEL	 ""
# define AGSI_BR_BLU	 ""
# define AGSI_BR_MAG	 ""
# define AGSI_BR_CYA	 ""
# define AGSI_BR_WHT	 ""
# define AGSI_BLK_BG	 ""
# define AGSI_RED_BG	 ""
# define AGSI_GRN_BG	 ""
# define AGSI_YEL_BG	 ""
# define AGSI_BLU_BG	 ""
# define AGSI_MAG_BG	 ""
# define AGSI_CYA_BG	 ""
# define AGSI_WHT_BG	 ""
# define AGSI_BR_BLK_BG	 ""
# define AGSI_BR_RED_BG	 ""
# define AGSI_BR_GRN_BG	 ""
# define AGSI_BR_YEL_BG	 ""
# define AGSI_BR_BLU_BG	 ""
# define AGSI_BR_MAG_BG	 ""
# define AGSI_BR_CYA_BG	 ""
# define AGSI_BR_WHT_BG	 ""

#endif /* AG_ANSI_COLOR */

/*
 * Map core font names to SGR sequences.
 */
#define AGSI_ALGUE               AGSI_FONT1		/* Algue */
#define AGSI_UNIALGUE            AGSI_FONT2		/* Unialgue */
#define AGSI_CMU_SANS            AGSI_FONT3		/* CMU Sans Serif */
#define AGSI_CMU_SERIF           AGSI_FONT4		/* CMU Serif */
#define AGSI_CMU_TYPEWRITER      AGSI_FONT5		/* CMU Typewriter */
#define AGSI_CHARTER             AGSI_FONT6		/* Charter */
#define AGSI_COURIER_PRIME       AGSI_FONT7		/* Courier Prime */
#define AGSI_SOURCE_HAN_SANS     AGSI_FONT8		/* Source Han Sans */
#define AGSI_LEAGUE_SPARTAN      AGSI_FONT9		/* League Spartan */
#define AGSI_LEAGUE_GOTHIC       AGSI_FONT10		/* League Gothic */
#define AGSI_UNIFRAKTUR_MAGUNTIA AGSI_FONT11		/* Unifraktur Maguntia */

#define AGSI_UNI           AGSI_UNIALGUE		/* Pan-unicode font */
#define AGSI_CM_SANS       AGSI_CMU_SANS 		/* CM Sans Serif */
#define AGSI_PATH	   AGSI_COURIER_PRIME		/* Path name font */
#define AGSI_CM_SERIF      AGSI_CMU_SERIF		/* CM Serif */
#define AGSI_CM_TYPEWRITER AGSI_CMU_TYPEWRITER		/* CM Typewriter */
#define AGSI_CODE          AGSI_COURIER_PRIME		/* Programming font */
#define AGSI_COURIER       AGSI_COURIER_PRIME		/* Courier font */
#define AGSI_SOURCE_HAN    AGSI_SOURCE_HAN_SANS		/* Source Han */
#define AGSI_CJK           AGSI_SOURCE_HAN_SANS		/* Pan-CKJ font */
#define AGSI_FRAKTUR       AGSI_UNIFRAKTUR_MAGUNTIA	/* Fraktur font */
#define AGSI_FRAK          AGSI_UNIFRAKTUR_MAGUNTIA	/* Fraktur font */

/*
 * Map AGSI_CMD to the preferred modifier key for application-global commands.
 */
#if defined(__APPLE2__) || defined(__BBC__) || defined(__PET__) || \
    defined(__VIC20__) || defined(__C64__) || defined(__C128__)
# define AGSI_CMD    "Shift-"
# define AGSI_CMD_MOD AG_KEYMOD_SHIFT
#elif defined(__APPLE__)
# define AGSI_CMD    "Command-"
# define AGSI_CMD_MOD AG_KEYMOD_META
#else
# define AGSI_CMD    "Ctrl-Shift-"
# define AGSI_CMD_MOD AG_KEYMOD_CTRL_SHIFT
#endif

/*
 * Characters available in core fonts ("W" = "WITH", "WO" = "WITHOUT").
 */

/*
 * General Punctuation.
 */
#define AGSI_THIN_SPACE              "\xE2\x80\x89"	/* U+2009 */
#define AGSI_HYPHEN                  "\xE2\x80\x90"	/* U+2010 */
#define AGSI_NON_BREAKING_HYPHEN     "\xE2\x80\x91"	/* U+2011 */
#define AGSI_FIGURE_DASH             "\xE2\x80\x92"	/* U+2012 */
#define AGSI_EN_DASH                 "\xE2\x80\x93"	/* U+2013 */
#define AGSI_EM_DASH                 "\xE2\x80\x94"	/* U+2014 */
#define AGSI_HORIZONTAL_BAR          "\xE2\x80\x95"	/* U+2015 */
#define AGSI_DOUBLE_VERTICAL_LINE    "\xE2\x80\x96"	/* U+2016 */
#define AGSI_DOUBLE_LOW_LINE         "\xE2\x80\x97"	/* U+2017 */
#define AGSI_QUOTE_LEFT              "\xE2\x80\x98"	/* U+2018 Left Single Quotation Mark */
#define AGSI_QUOTE_RIGHT             "\xE2\x80\x99"	/* U+2019 Left Single Quotation Mark */
#define AGSI_QUOTE_BASE              "\xE2\x80\x9A"	/* U+201A Single Low-9 Quotation Mark */
#define AGSI_QUOTE_REVERSED          "\xE2\x80\x9B"	/* U+201B Single High-Reversed-9 Quotation Mark */
#define AGSI_DBLQUOTE_LEFT           "\xE2\x80\x9C"	/* U+201C Left Double Quotation Mark */
#define AGSI_DBLQUOTE_RIGHT          "\xE2\x80\x9D"	/* U+201D Right Double Quotation Mark */
#define AGSI_DBLQUOTE_BASE           "\xE2\x80\x9E"	/* U+201E Double Low-9 Quotation Mark */
#define AGSI_DBLQUOTE_REVERSED       "\xE2\x80\x9F"	/* U+201F Double High-Reversed-9 Quotation Mark */
#define AGSI_DAGGER                  "\xE2\x80\xA0"	/* U+2020 */
#define AGSI_DBLDAGGER               "\xE2\x80\xA1"	/* U+2021 Double Dagger */
#define AGSI_BULLET                  "\xE2\x80\xA2"	/* U+2022 */
#define AGSI_TRIANGULAR_BULLET       "\xE2\x80\xA3"	/* U+2023 */
#define AGSI_ONE_DOT_LEADER          "\xE2\x80\xA4"	/* U+2024 */
#define AGSI_TWO_DOT_LEADER          "\xE2\x80\xA5"	/* U+2025 */
#define AGSI_ELLIPSIS                "\xE2\x80\xA6"	/* U+2026 Horizontal Ellipsis */
#define AGSI_HYPHENATION_POINT       "\xE2\x80\xA7"	/* U+2027 */
#define AGSI_PER_THOUSAND            "\xE2\x80\xB0"	/* U+2030 Per Mille Sign */
#define AGSI_PER_TEN_THOUSAND        "\xE2\x80\xB1"	/* U+2031 */
#define AGSI_PRIME                   "\xE2\x80\xB2"	/* U+2032 Minute */
#define AGSI_DOUBLE_PRIME            "\xE2\x80\xB3"	/* U+2033 Second */
#define AGSI_MINUTE                  AGSI_PRIME
#define AGSI_SECOND                  AGSI_DOUBLE_PRIME
#define AGSI_TRIPLE_PRIME            "\xE2\x80\xB4"	/* U+2034 */
#define AGSI_REVERSED_PRIME          "\xE2\x80\xB5"	/* U+2035 */
#define AGSI_REVERSED_DOUBLE_PRIME   "\xE2\x80\xB6"	/* U+2036 */
#define AGSI_REVERSED_TRIPLE_PRIME   "\xE2\x80\xB7"	/* U+2037 */
#define AGSI_CARET                   "\xE2\x80\xB8"	/* U+2038 */
#define AGSI_GUILSINGL_LEFT          "\xE2\x80\xB9"	/* U+2039 Single Left-Pointing Angle Quotation Mark */
#define AGSI_GUILSINGL_RIGHT         "\xE2\x80\xBA"	/* U+203A Single Right-Pointing Angle Quotation Mark */
#define AGSI_REFERENCE_MARK          "\xE2\x80\xBB"	/* U+203B */
#define AGSI_EXCLAM_DBL              "\xE2\x80\xBC"	/* U+203C Double Exclamation Mark */
#define AGSI_INTERROBANG             "\xE2\x80\xBD"	/* U+203D */
#define AGSI_OVERLINE                "\xE2\x80\xBE"	/* U+203E */
#define AGSI_UNDERTIE                "\xE2\x80\xBF"	/* U+203F */
#define AGSI_CHARACTER_TIE           "\xE2\x81\x80"	/* U+2040 */
#define AGSI_CARET_INSERTION_POINT   "\xE2\x81\x81"	/* U+2041 */
#define AGSI_ASTERISM                "\xE2\x81\x82"	/* U+2042 */
#define AGSI_HYPHEN_BULLET           "\xE2\x81\x83"	/* U+2043 */
#define AGSI_FRACTION_SLASH          "\xE2\x81\x84"	/* U+2044 */
#define AGSI_L_SQ_BRACKET_W_QUILL    "\xE2\x81\x85"	/* U+2045 Left Square Bracket With Quill */
#define AGSI_R_SQ_BRACKET_W_QUILL    "\xE2\x81\x86"	/* U+2046 Right Square Bracket With Quill */
#define AGSI_DOUBLE_QUESTION         "\xE2\x81\x87"	/* U+2047 Double Question Mark */
#define AGSI_QUESTION_EXCLAMATION    "\xE2\x81\x88"	/* U+2048 Question Exclamation Mark */
#define AGSI_EXCLAMATION_QUESTION    "\xE2\x81\x89"	/* U+2049 Exclamation Question Mark */
#define AGSI_TIRONIAN_SIGN_ET        "\xE2\x81\x8A"	/* U+204A */
#define AGSI_REVERSED_PILCROW_SIGN   "\xE2\x81\x8B"	/* U+204B */
#define AGSI_BLACK_LEFTWARDS_BULLET  "\xE2\x81\x8C"	/* U+204C */
#define AGSI_BLACK_RIGHTWARDS_BULLET "\xE2\x81\x8D"	/* U+204D */
#define AGSI_LOW_ASTERISK            "\xE2\x81\x8E"	/* U+204E */
#define AGSI_REVERSED_SEMICOLON      "\xE2\x81\x8F"	/* U+204F */
#define AGSI_CLOSE_UP                "\xE2\x81\x90"	/* U+2050 */
#define AGSI_TWO_ASTERISKS_VALIGNED  "\xE2\x81\x91"	/* U+2051 Two Asterisks Aligned Vertically */
#define AGSI_COMMERCIAL_MINUS_SIGN   "\xE2\x81\x92"	/* U+2052 */
#define AGSI_SWUNG_DASH              "\xE2\x81\x93"	/* U+2053 */
#define AGSI_INVERTED_UNDERTIE       "\xE2\x81\x94"	/* U+2054 */
#define AGSI_FLOWER_PUNCTUATION      "\xE2\x81\x95"	/* U+2055 Flower Punctuation Mark */
#define AGSI_THREE_DOT_PUNCTUATION   "\xE2\x81\x96"	/* U+2056 */
#define AGSI_QUADRUPLE_PRIME         "\xE2\x81\x97"	/* U+2057 */
#define AGSI_FOUR_DOT_PUNCTUATION    "\xE2\x81\x98"	/* U+2058 */
#define AGSI_FIVE_DOT_PUNCTUATION    "\xE2\x81\x99"	/* U+2059 */
#define AGSI_TWO_DOT_PUNCTUATION     "\xE2\x81\x9A"	/* U+205A */
#define AGSI_FOUR_DOT                "\xE2\x81\x9B"	/* U+205B Four Dot Mark */
#define AGSI_DOTTED_CROSS            "\xE2\x81\x9C"	/* U+205C */
#define AGSI_TRICOLON                "\xE2\x81\x9D"	/* U+205D */
#define AGSI_VERTICAL_FOUR_DOTS      "\xE2\x81\x9E"	/* U+205E */
#define AGSI_MEDIUM_MATH_SPACE       "\xE2\x81\x9F"	/* U+205F Medium Mathematical Space */
#define AGSI_WORD_JOINER             "\xE2\x81\xA0"	/* U+2060 */
#define AGSI_FUNCTION_APPLICATION    "\xE2\x81\xA1"	/* U+2061 */
#define AGSI_INVISIBLE_TIMES         "\xE2\x81\xA2"	/* U+2062 */
#define AGSI_INVISIBLE_SEPARATOR     "\xE2\x81\xA3"	/* U+2063 */
#define AGSI_INVISIBLE_PLUS          "\xE2\x81\xA4"	/* U+2064 */
#define AGSI_INH_SYMMETRIC_SWAPPING  "\xE2\x81\xAA"	/* U+206A Inhibit Symmetric Swapping */
#define AGSI_ACT_SYMMETRIC_SWAPPING  "\xE2\x81\xAB"	/* U+206B Activate Symmetric Swapping */
#define AGSI_INH_ARABIC_FORM_SHAPING "\xE2\x81\xAC"	/* U+206C Inhibit Arabic Form Shaping */
#define AGSI_ACT_ARABIC_FORM_SHAPING "\xE2\x81\xAD"	/* U+206D Activate Arabic Form Shaping */
#define AGSI_NATIONAL_DIGIT_SHAPES   "\xE2\x81\xAE"	/* U+206E */
/*
 * Superscripts and Subscripts.
 */
#define AGSI_SUPERSCRIPT_0           "\xE2\x81\xB0"	/* U+2070 Superscript Zero */
#define AGSI_SUPERSCRIPT_SMALL_I     "\xE2\x81\xB1"	/* U+2071 Superscript Latin Small Letter I */
#define AGSI_SUPERSCRIPT_4           "\xE2\x81\xB4"	/* U+2074 Superscript Four */
#define AGSI_SUPERSCRIPT_5           "\xE2\x81\xB5"	/* U+2075 Superscript Five */
#define AGSI_SUPERSCRIPT_6           "\xE2\x81\xB6"	/* U+2076 Superscript Six */
#define AGSI_SUPERSCRIPT_7           "\xE2\x81\xB7"	/* U+2077 Superscript Seven */
#define AGSI_SUPERSCRIPT_8           "\xE2\x81\xB8"	/* U+2078 Superscript Eight */
#define AGSI_SUPERSCRIPT_9           "\xE2\x81\xB9"	/* U+2079 Superscript Nine */
#define AGSI_SUPERSCRIPT_PLUS        "\xE2\x81\xBA"	/* U+207A Superscript Plus Sign */
#define AGSI_SUPERSCRIPT_MINUS       "\xE2\x81\xBB"	/* U+207B */
#define AGSI_SUPERSCRIPT_EQUALS      "\xE2\x81\xBC"	/* U+207C Superscript Equals Sign */
#define AGSI_SUPERSCRIPT_LEFT_PAREN  "\xE2\x81\xBD"	/* U+207D Superscript Left Parenthesis */
#define AGSI_SUPERSCRIPT_RIGHT_PAREN "\xE2\x81\xBE"	/* U+207E Superscript Right Parenthesis */
#define AGSI_SUPERSCRIPT_SMALL_N     "\xE2\x81\xBF"	/* U+207F Superscript Latin Small Letter N */
#define AGSI_SUBSCRIPT_0             "\xE2\x82\x80"	/* U+2080 Subscript 0 */
#define AGSI_SUBSCRIPT_1             "\xE2\x82\x81"	/* U+2081 Subscript 1 */
#define AGSI_SUBSCRIPT_2             "\xE2\x82\x82"	/* U+2082 Subscript 2 */
#define AGSI_SUBSCRIPT_3             "\xE2\x82\x83"	/* U+2083 Subscript 3 */
#define AGSI_SUBSCRIPT_4             "\xE2\x82\x84"	/* U+2084 Subscript 4 */
#define AGSI_SUBSCRIPT_5             "\xE2\x82\x85"	/* U+2085 Subscript 5 */
#define AGSI_SUBSCRIPT_6             "\xE2\x82\x86"	/* U+2086 Subscript 6 */
#define AGSI_SUBSCRIPT_7             "\xE2\x82\x87"	/* U+2087 Subscript 7 */
#define AGSI_SUBSCRIPT_8             "\xE2\x82\x88"	/* U+2088 Subscript 8 */
#define AGSI_SUBSCRIPT_9             "\xE2\x82\x89"	/* U+2089 Subscript 9 */
#define AGSI_SUBSCRIPT_PLUS          "\xE2\x82\x8A"	/* U+208A Subscript Plus Sign */
#define AGSI_SUBSCRIPT_MINUS         "\xE2\x82\x8B"	/* U+208B */
#define AGSI_SUBSCRIPT_EQUALS        "\xE2\x82\x8C"	/* U+208C Subscript Equals Sign */
#define AGSI_SUBSCRIPT_LEFT_PAREN    "\xE2\x82\x8D"	/* U+208D Subscript Left Parenthesis */
#define AGSI_SUBSCRIPT_RIGHT_PAREN   "\xE2\x82\x8E"	/* U+208E Subscript Right Parenthesis */
#define AGSI_SUBSCRIPT_SMALL_A       "\xE2\x82\x90"	/* U+2090 Latin Subscript Small Letter A */
#define AGSI_SUBSCRIPT_SMALL_E       "\xE2\x82\x91"	/* U+2091 Latin Subscript Small Letter E */
#define AGSI_SUBSCRIPT_SMALL_O       "\xE2\x82\x92"	/* U+2092 Latin Subscript Small Letter O */
#define AGSI_SUBSCRIPT_SMALL_X       "\xE2\x82\x93"	/* U+2093 Latin Subscript Small Letter X */
#define AGSI_SUBSCRIPT_SMALL_H       "\xE2\x82\x95"	/* U+2095 Latin Subscript Small Letter H */
#define AGSI_SUBSCRIPT_SMALL_K       "\xE2\x82\x96"	/* U+2096 Latin Subscript Small Letter K */
#define AGSI_SUBSCRIPT_SMALL_L       "\xE2\x82\x97"	/* U+2097 Latin Subscript Small Letter L */
#define AGSI_SUBSCRIPT_SMALL_M       "\xE2\x82\x98"	/* U+2098 Latin Subscript Small Letter M */
#define AGSI_SUBSCRIPT_SMALL_N       "\xE2\x82\x99"	/* U+2099 Latin Subscript Small Letter N */
#define AGSI_SUBSCRIPT_SMALL_P       "\xE2\x82\x9A"	/* U+209A Latin Subscript Small Letter P */
#define AGSI_SUBSCRIPT_SMALL_S       "\xE2\x82\x9B"	/* U+209B Latin Subscript Small Letter S */
#define AGSI_SUBSCRIPT_SMALL_T       "\xE2\x82\x9C"	/* U+209C Latin Subscript Small Letter T */
/*
 * Number Forms.
 */
#define AGSI_ROMAN_NUMERAL_1            "\xE2\x85\xA0"      /* U+2160 */
#define AGSI_ROMAN_NUMERAL_2            "\xE2\x85\xA1"      /* U+2161 */
#define AGSI_ROMAN_NUMERAL_3            "\xE2\x85\xA2"      /* U+2162 */
#define AGSI_ROMAN_NUMERAL_4            "\xE2\x85\xA3"      /* U+2163 */
#define AGSI_ROMAN_NUMERAL_5            "\xE2\x85\xA4"      /* U+2164 */
#define AGSI_ROMAN_NUMERAL_6            "\xE2\x85\xA5"      /* U+2165 */
#define AGSI_ROMAN_NUMERAL_7            "\xE2\x85\xA6"      /* U+2166 */
#define AGSI_ROMAN_NUMERAL_8            "\xE2\x85\xA7"      /* U+2167 */
#define AGSI_ROMAN_NUMERAL_9            "\xE2\x85\xA8"      /* U+2168 */
#define AGSI_ROMAN_NUMERAL_10           "\xE2\x85\xA9"      /* U+2169 */
#define AGSI_ROMAN_NUMERAL_11           "\xE2\x85\xAA"      /* U+216A */
#define AGSI_ROMAN_NUMERAL_12           "\xE2\x85\xAB"      /* U+216B */
#define AGSI_ROMAN_NUMERAL_50           "\xE2\x85\xAC"      /* U+216C */
#define AGSI_ROMAN_NUMERAL_100          "\xE2\x85\xAD"      /* U+216D */
#define AGSI_ROMAN_NUMERAL_500          "\xE2\x85\xAE"      /* U+216E */
#define AGSI_ROMAN_NUMERAL_1000         "\xE2\x85\xAF"      /* U+216F */
#define AGSI_SMALL_ROMAN_NUMERAL_1      "\xE2\x85\xB0"      /* U+2170 */
#define AGSI_SMALL_ROMAN_NUMERAL_2      "\xE2\x85\xB1"      /* U+2171 */
#define AGSI_SMALL_ROMAN_NUMERAL_3      "\xE2\x85\xB2"      /* U+2172 */
#define AGSI_SMALL_ROMAN_NUMERAL_4      "\xE2\x85\xB3"      /* U+2173 */
#define AGSI_SMALL_ROMAN_NUMERAL_5      "\xE2\x85\xB4"      /* U+2174 */
#define AGSI_SMALL_ROMAN_NUMERAL_6      "\xE2\x85\xB5"      /* U+2175 */
#define AGSI_SMALL_ROMAN_NUMERAL_7      "\xE2\x85\xB6"      /* U+2176 */
#define AGSI_SMALL_ROMAN_NUMERAL_8      "\xE2\x85\xB7"      /* U+2177 */
#define AGSI_SMALL_ROMAN_NUMERAL_9      "\xE2\x85\xB8"      /* U+2178 */
#define AGSI_SMALL_ROMAN_NUMERAL_10     "\xE2\x85\xB9"      /* U+2179 */
#define AGSI_SMALL_ROMAN_NUMERAL_11     "\xE2\x85\xBA"      /* U+217A */
#define AGSI_SMALL_ROMAN_NUMERAL_12     "\xE2\x85\xBB"      /* U+217B */
#define AGSI_SMALL_ROMAN_NUMERAL_50     "\xE2\x85\xBC"      /* U+217C */
#define AGSI_SMALL_ROMAN_NUMERAL_100    "\xE2\x85\xBD"      /* U+217D */
#define AGSI_SMALL_ROMAN_NUMERAL_500    "\xE2\x85\xBE"      /* U+217E */
#define AGSI_SMALL_ROMAN_NUMERAL_1000   "\xE2\x85\xBF"      /* U+217F */
#define AGSI_ROMAN_NUMERAL_1000_C_D     "\xE2\x85\x80"      /* U+2180 */
#define AGSI_ROMAN_NUMERAL_5000         "\xE2\x85\x81"      /* U+2181 */
#define AGSI_ROMAN_NUMERAL_10000        "\xE2\x85\x82"      /* U+2182 */
#define AGSI_ROMAN_NUMERAL_REVERSED_100 "\xE2\x85\x83"      /* U+2183 */
#define AGSI_LATIN_SMALL_REVERSED_C     "\xE2\x85\x84"      /* U+2184 Latin Small Letter Reversed C */
#define AGSI_ROMAN_NUMERAL_6_LATE_FORM  "\xE2\x85\x85"      /* U+2185 */
/*
 * Arrows.
 */
#define AGSI_ARROW_LEFT             "\xE2\x86\x90"	/* U+2190 Leftwards Arrow */
#define AGSI_ARROW_UP               "\xE2\x86\x91"	/* U+2191 Upwards Arrow */
#define AGSI_ARROW_RIGHT            "\xE2\x86\x92"	/* U+2192 Rightwards Arrow */
#define AGSI_ARROW_DOWN             "\xE2\x86\x93"	/* U+2193 Downwards Arrow */
#define AGSI_ARROW_LEFT_RIGHT       "\xE2\x86\x94"	/* U+2194 Left-Right Arrow */
#define AGSI_ARROW_UP_DOWN          "\xE2\x86\x95"	/* U+2195 Up-Down Arrow */
#define AGSI_ARROW_NORTH_WEST       "\xE2\x86\x96"	/* U+2196 North-West Arrow */
#define AGSI_ARROW_NORTH_EAST       "\xE2\x86\x97"	/* U+2197 North-East Arrow */
#define AGSI_ARROW_SOUTH_EAST       "\xE2\x86\x98"	/* U+2198 South East Arrow */
#define AGSI_ARROW_SOUTH_WEST       "\xE2\x86\x99"	/* U+2199 South West Arrow */
/*
 * Mathematical Operators.
 */
#define AGSI_ASYMPTOMATICALLY_EQUAL_TO              "\xE2\x89\x83" /* U+2243 */
#define AGSI_NOT_ASYMPTOMATICALLY_EQUAL_TO          "\xE2\x89\x84" /* U+2244 */
#define AGSI_APPROX_EQUAL_TO                        "\xE2\x89\x85" /* U+2245 Approximately Equal To */
#define AGSI_APPROX_BUT_NOT_ACTUALLY_EQUAL_TO       "\xE2\x89\x86" /* U+2246 Approximately But Not Actually Equal To */
#define AGSI_NEITHER_APPROX_NOR_ACTUALLY_EQUAL_TO   "\xE2\x89\x87" /* U+2247 Neither Approximately Nor Actually Equal To */
#define AGSI_ALMOST_EQUAL_TO                        "\xE2\x89\x88" /* U+2248 */
#define AGSI_NOT_ALMOST_EQUAL_TO                    "\xE2\x89\x89" /* U+2249 */
#define AGSI_ALMOST_EQUAL_OR_EQUAL_TO               "\xE2\x89\x8A" /* U+224A */
#define AGSI_TRIPLE_TILDE                           "\xE2\x89\x8B" /* U+224B */
#define AGSI_ALL_EQUAL_TO                           "\xE2\x89\x8C" /* U+224C */
#define AGSI_EQUIVALENT_TO                          "\xE2\x89\x8D" /* U+224D */
#define AGSI_GEOMETRICALLY_EQUIVALENT_TO            "\xE2\x89\x8E" /* U+224E */
#define AGSI_DIFFERENCE_BETWEEN                     "\xE2\x89\x8F" /* U+224F */
#define AGSI_APPROACHES_THE_LIMIT                   "\xE2\x89\x90" /* U+2250 */
#define AGSI_GEOMETRICALLY_EQUAL_TO                 "\xE2\x89\x91" /* U+2251 */
#define AGSI_APPROX_EQUAL_TO_OR_THE_IMAGE_OF        "\xE2\x89\x92" /* U+2252 Approximately Equal To Or The Image Of */
#define AGSI_IMAGE_OF_OR_APPROX_EQUAL_TO            "\xE2\x89\x93" /* U+2253 Image Of Or Approximately Equal To */
#define AGSI_COLON_EQUALS                           "\xE2\x89\x94" /* U+2254 */
#define AGSI_EQUALS_COLON                           "\xE2\x89\x95" /* U+2255 */
#define AGSI_RING_IN_EQUAL_TO                       "\xE2\x89\x96" /* U+2256 */
#define AGSI_RING_EQUAL_TO                          "\xE2\x89\x97" /* U+2257 */
#define AGSI_CORRESPONDS_TO                         "\xE2\x89\x98" /* U+2258 */
#define AGSI_ESTIMATES                              "\xE2\x89\x99" /* U+2259 */
#define AGSI_EQUIANGULAR_TO                         "\xE2\x89\x9A" /* U+225A */
#define AGSI_STAR_EQUALS                            "\xE2\x89\x9B" /* U+225B */
#define AGSI_DELTA_EQUAL_TO                         "\xE2\x89\x9C" /* U+225C */
#define AGSI_EQUAL_TO_BY_DEFINITION                 "\xE2\x89\x9D" /* U+225D */
#define AGSI_MEASURED_BY                            "\xE2\x89\x9E" /* U+225E */
#define AGSI_QUESTIONED_EQUAL_TO                    "\xE2\x89\x9F" /* U+225F */
#define AGSI_NOT_EQUAL_TO                           "\xE2\x89\xA0" /* U+2260 */
#define AGSI_IDENTICAL_TO                           "\xE2\x89\xA1" /* U+2261 */
#define AGSI_NOT_IDENTICAL_TO                       "\xE2\x89\xA2" /* U+2262 */
#define AGSI_STRICTLY_EQUIVALENT_TO                 "\xE2\x89\xA3" /* U+2263 */
#define AGSI_LESS_THAN_OR_EQUAL_TO                  "\xE2\x89\xA4" /* U+2264 */
#define AGSI_GREATER_THAN_OR_EQUAL_TO               "\xE2\x89\xA5" /* U+2265 */
#define AGSI_LESS_THAN_OVER_EQUAL_TO                "\xE2\x89\xA6" /* U+2266 */
#define AGSI_GREATER_THAN_OVER_EQUAL_TO             "\xE2\x89\xA7" /* U+2267 */
#define AGSI_LESS_THAN_BUT_NOT_EQUAL_TO             "\xE2\x89\xA8" /* U+2268 */
#define AGSI_GREATER_THAN_BUT_NOT_EQUAL_TO          "\xE2\x89\xA9" /* U+2269 */
#define AGSI_MUCH_LESS_THAN                         "\xE2\x89\xAA" /* U+226A */
#define AGSI_MUCH_GREATER_THAN                      "\xE2\x89\xAB" /* U+226B */
#define AGSI_BETWEEN                                "\xE2\x89\xAC" /* U+226C */
#define AGSI_NOT_EQUIVALENT_TO                      "\xE2\x89\xAD" /* U+226D */
#define AGSI_NOT_LESS_THAN                          "\xE2\x89\xAE" /* U+226E */
#define AGSI_NOT_GREATER_THAN                       "\xE2\x89\xAF" /* U+226F */
#define AGSI_NEITHER_LESS_THAN_NOR_EQUAL_TO         "\xE2\x89\xB0" /* U+2270 */
#define AGSI_NEITHER_GREATER_THAN_NOR_EQUAL_TO      "\xE2\x89\xB1" /* U+2271 */
#define AGSI_LESS_THAN_OR_EQUIVALENT_TO             "\xE2\x89\xB2" /* U+2272 */
#define AGSI_GREATER_THAN_OR_EQUIVALENT_TO          "\xE2\x89\xB3" /* U+2273 */
#define AGSI_NEITHER_LESS_THAN_NOR_EQUIVALENT_TO    "\xE2\x89\xB4" /* U+2274 */
#define AGSI_NEITHER_GREATER_THAN_NOR_EQUIVALENT_TO "\xE2\x89\xB5" /* U+2275 */
#define AGSI_LESS_THAN_OR_GREATER_THAN              "\xE2\x89\xB6" /* U+2276 */
#define AGSI_GREATER_THAN_OR_LESS_THAN              "\xE2\x89\xB7" /* U+2277 */
#define AGSI_NEITHER_LESS_THAN_NOR_GREATER_THAN     "\xE2\x89\xB8" /* U+2278 */
#define AGSI_NEITHER_GREATER_THAN_NOR_LESS_THAN     "\xE2\x89\xB9" /* U+2279 */
#define AGSI_PRECEDES                               "\xE2\x89\xBA" /* U+227A */
#define AGSI_SUCCEEDS                               "\xE2\x89\xBB" /* U+227B */
#define AGSI_PRECEDES_OR_EQUAL_TO                   "\xE2\x89\xBC" /* U+227C */
#define AGSI_SUCCEEDS_OR_EQUAL_TO                   "\xE2\x89\xBD" /* U+227D */
#define AGSI_PRECEDES_OR_EQUIVALENT_TO              "\xE2\x89\xBE" /* U+227E */
#define AGSI_SUCCEEDS_OR_EQUIVALENT_TO              "\xE2\x89\xBF" /* U+227F */
#define AGSI_DOES_NOT_PRECEDE                       "\xE2\x8A\x80" /* U+2280 */
#define AGSI_DOES_NOT_SUCCEED                       "\xE2\x8A\x81" /* U+2281 */
#define AGSI_SUBSET_OF                              "\xE2\x8A\x82" /* U+2282 */
#define AGSI_SUBSET_OF                              "\xE2\x8A\x82" /* U+2282 */
#define AGSI_SUPERSET_OF                            "\xE2\x8A\x83" /* U+2283 */
#define AGSI_NOT_A_SUBSET_OF                        "\xE2\x8A\x84" /* U+2284 */
#define AGSI_NOT_A_SUPERSET_OF                      "\xE2\x8A\x85" /* U+2285 */
#define AGSI_SUBSET_OF_OR_EQUAL_TO                  "\xE2\x8A\x86" /* U+2286 */
#define AGSI_SUPERSET_OF_OR_EQUAL_TO                "\xE2\x8A\x87" /* U+2287 */
#define AGSI_NEITHER_A_SUBSET_OF_NOR_EQUAL_TO       "\xE2\x8A\x88" /* U+2288 */
#define AGSI_NEITHER_A_SUPERSET_OF_NOR_EQUAL_TO     "\xE2\x8A\x89" /* U+2289 */
#define AGSI_SUBSET_OF_WITH_NOT_EQUAL_TO            "\xE2\x8A\x8A" /* U+228A */
#define AGSI_SUPERSET_OF_WITH_NOT_EQUAL_TO          "\xE2\x8A\x8B" /* U+228B */
#define AGSI_MULTISET                               "\xE2\x8A\x8C" /* U+228C */
#define AGSI_MULTISET_MULTIPLICATION                "\xE2\x8A\x8D" /* U+228D */
#define AGSI_MULTISET_UNION                         "\xE2\x8A\x8E" /* U+228E */
#define AGSI_SQUARE_IMAGE_OF                        "\xE2\x8A\x8F" /* U+228F */
#define AGSI_SQUARE_ORIGINAL_OF                     "\xE2\x8A\x90" /* U+2290 */
#define AGSI_SQUARE_IMAGE_OF_OR_EQUAL_TO            "\xE2\x8A\x91" /* U+2291 */
#define AGSI_SQUARE_ORIGINAL_OF_OR_EQUAL_TO         "\xE2\x8A\x92" /* U+2292 */
#define AGSI_SQUARE_CAP                             "\xE2\x8A\x93" /* U+2293 */
#define AGSI_SQUARE_CUP                             "\xE2\x8A\x94" /* U+2294 */
#define AGSI_CIRCLED_PLUS                           "\xE2\x8A\x95" /* U+2295 */
#define AGSI_CIRCLED_MINUS                          "\xE2\x8A\x96" /* U+2296 */
#define AGSI_CIRCLED_TIMES                          "\xE2\x8A\x97" /* U+2297 */
#define AGSI_CIRCLED_DIVISION_SLASH                 "\xE2\x8A\x98" /* U+2298 */
#define AGSI_CIRCLED_DOT                            "\xE2\x8A\x99" /* U+2299 Circled Dot Operator */
#define AGSI_CIRCLED_RING                           "\xE2\x8A\x9A" /* U+229A Circled Ring Operator */
#define AGSI_CIRCLED_ASTERISK                       "\xE2\x8A\x9B" /* U+229B Circled Asterisk Operator */
#define AGSI_CIRCLED_EQUALS                         "\xE2\x8A\x9C" /* U+229C */
#define AGSI_CIRCLED_DASH                           "\xE2\x8A\x9D" /* U+229D */
#define AGSI_SQUARED_PLUS                           "\xE2\x8A\x9E" /* U+229E */
#define AGSI_SQUARED_MINUS                          "\xE2\x8A\x9F" /* U+229F */
#define AGSI_SQUARED_TIMES                          "\xE2\x8A\xA0" /* U+22A0 */
#define AGSI_SQUARED_DOT                            "\xE2\x8A\xA1" /* U+22A1 Squared Dot Operator */
#define AGSI_RIGHT_TACK                             "\xE2\x8A\xA2" /* U+22A2 */
#define AGSI_LEFT_TACK                              "\xE2\x8A\xA3" /* U+22A3 */
#define AGSI_DOWN_TACK                              "\xE2\x8A\xA4" /* U+22A4 */
#define AGSI_UP_TACK                                "\xE2\x8A\xA5" /* U+22A5 */
#define AGSI_PERPENDICULAR AGSI_UP_TACK
#define AGSI_ASSERTION                              "\xE2\x8A\xA6" /* U+22A6 */
#define AGSI_MODELS                                 "\xE2\x8A\xA7" /* U+22A7 */
#define AGSI_TRUE_SYMBOL                            "\xE2\x8A\xA8" /* U+22A8 True */
#define AGSI_FORCES_SYMBOL                          "\xE2\x8A\xA9" /* U+22A9 Forces */
#define AGSI_TRIPLE_VERTICAL_BAR_RIGHT_TURNSTILE    "\xE2\x8A\xAA" /* U+22AA */
#define AGSI_DOUBLE_VERTICAL_BAR_DOUBLE_RIGHT_TURNSTILE "\xE2\x8A\xAB" /* U+22AB */
#define AGSI_DOES_NOT_PROVE                         "\xE2\x8A\xAC" /* U+22AC */
#define AGSI_NOT_TRUE_SYMBOL                        "\xE2\x8A\xAD" /* U+22AD Not True */
#define AGSI_DOES_NOT_FORCE                         "\xE2\x8A\xAE" /* U+22AE */
#define AGSI_NEGATED_DOUBLE_VERTICAL_BAR_DOUBLE_RIGHT_TURNSTILE "\xE2\x8A\xAF" /* U+22AF */
#define AGSI_PRECEDES_UNDER_RELATION                "\xE2\x8A\xB0" /* U+22B0 */
#define AGSI_SUCCEEDS_UNDER_RELATION                "\xE2\x8A\xB1" /* U+22B1 */
#define AGSI_NORMAL_SUBGROUP_OF                     "\xE2\x8A\xB2" /* U+22B2 */
#define AGSI_CONTAINS_AS_NORMAL_SUBGROUP            "\xE2\x8A\xB3" /* U+22B3 */
#define AGSI_NORMAL_SUBGROUP_OF_OR_EQUAL_TO         "\xE2\x8A\xB4" /* U+22B4 */
#define AGSI_CONTAINS_AS_NORMAL_SUBGROUP_OR_EQUAL_TO "\xE2\x8A\xB5" /* U+22B5 */
#define AGSI_ORIGINAL_OF                            "\xE2\x8A\xB6" /* U+22B6 */
#define AGSI_IMAGE_OF                               "\xE2\x8A\xB7" /* U+22B7 */
#define AGSI_MULTIMAP                               "\xE2\x8A\xB8" /* U+22B8 */
#define AGSI_HERMITIAN_CONJUGATE_MATRIX             "\xE2\x8A\xB9" /* U+22B9 */
#define AGSI_INTERCALATE                            "\xE2\x8A\xBA" /* U+22BA */
#define AGSI_XOR_SYMBOL                             "\xE2\x8A\xBB" /* U+22BB Xor */
#define AGSI_NAND_SYMBOL                            "\xE2\x8A\xBC" /* U+22BC Nand */
#define AGSI_NOR_SYMBOL                             "\xE2\x8A\xBD" /* U+22BD Nor */
#define AGSI_RIGHT_ANGLE_WITH_ARC                   "\xE2\x8A\xBE" /* U+22BE */
#define AGSI_RIGHT_TRIANGLE                         "\xE2\x8A\xBF" /* U+22BF */
#define AGSI_N_ARY_LOGICAL_AND                      "\xE2\x8B\x80" /* U+22C0 */
#define AGSI_N_ARY_LOGICAL_OR                       "\xE2\x8B\x81" /* U+22C1 */
#define AGSI_N_ARY_INTERSECTION                     "\xE2\x8B\x82" /* U+22C2 */
#define AGSI_N_ARY_UNION                            "\xE2\x8B\x83" /* U+22C3 */
#define AGSI_DIAMOND_OPERATOR                       "\xE2\x8B\x84" /* U+22C4 */
#define AGSI_DOT_OPERATOR                           "\xE2\x8B\x85" /* U+22C5 */
#define AGSI_STAR_OPERATOR                          "\xE2\x8B\x86" /* U+22C6 */
#define AGSI_DIVISION_TIMES                         "\xE2\x8B\x87" /* U+22C7 */
#define AGSI_BOWTIE                                 "\xE2\x8B\x88" /* U+22C8 */
#define AGSI_LEFT_NORMAL_FACTOR_SEMIDIRECT_PRODUCT  "\xE2\x8B\x89" /* U+22C9 */
#define AGSI_RIGHT_NORMAL_FACTOR_SEMIDIRECT_PRODUCT "\xE2\x8B\x8A" /* U+22CA */
#define AGSI_LEFT_SEMIDIRECT_PRODUCT                "\xE2\x8B\x8B" /* U+22CB */
#define AGSI_RIGHT_SEMIDIRECT_PRODUCT               "\xE2\x8B\x8C" /* U+22CC */
#define AGSI_REVERSED_TILDE_EQUALS                  "\xE2\x8B\x8D" /* U+22CD */
#define AGSI_CURLY_LOGICAL_OR                       "\xE2\x8B\x8E" /* U+22CE */
#define AGSI_CURLY_LOGICAL_AND                      "\xE2\x8B\x8F" /* U+22CF */
#define AGSI_DOUBLE_SUBSET                          "\xE2\x8B\x90" /* U+22D0 */
#define AGSI_DOUBLE_SUPERSET                        "\xE2\x8B\x91" /* U+22D1 */
#define AGSI_DOUBLE_INTERSECTION                    "\xE2\x8B\x92" /* U+22D2 */
#define AGSI_DOUBLE_UNION                           "\xE2\x8B\x93" /* U+22D3 */
#define AGSI_PITCHFORK                              "\xE2\x8B\x94" /* U+22D4 */
#define AGSI_EQUAL_AND_PARALLEL_TO                  "\xE2\x8B\x95" /* U+22D5 */
#define AGSI_LESS_THAN_WITH_DOT                     "\xE2\x8B\x96" /* U+22D6 */
#define AGSI_GREATER_THAN_WITH_DOT                  "\xE2\x8B\x97" /* U+22D7 */
#define AGSI_VERY_MUCH_LESS_THAN                    "\xE2\x8B\x98" /* U+22D8 */
#define AGSI_VERY_MUCH_GREATER_THAN                 "\xE2\x8B\x99" /* U+22D9 */
#define AGSI_LESS_THAN_EQUAL_TO_OR_GREATER_THAN     "\xE2\x8B\x9A" /* U+22DA */
#define AGSI_GREATER_THAN_EQUAL_TO_OR_LESS_THAN     "\xE2\x8B\x9B" /* U+22DB */
#define AGSI_EQUAL_TO_OR_LESS_THAN                  "\xE2\x8B\x9C" /* U+22DC */
#define AGSI_EQUAL_TO_OR_GREATER_THAN               "\xE2\x8B\x9D" /* U+22DD */
#define AGSI_EQUAL_TO_OR_PRECEDES                   "\xE2\x8B\x9E" /* U+22DE */
#define AGSI_EQUAL_TO_OR_SUCCEEDS                   "\xE2\x8B\x9F" /* U+22DF */
#define AGSI_DOES_NOT_PRECEDE_OR_EQUAL              "\xE2\x8B\xA0" /* U+22E0 */
#define AGSI_DOES_NOT_SUCCEED_OR_EQUAL              "\xE2\x8B\xA1" /* U+22E1 */
#define AGSI_NOT_SQUARE_IMAGE_OF_OR_EQUAL_TO        "\xE2\x8B\xA2" /* U+22E2 */
#define AGSI_NOT_SQUARE_ORIGINAL_OF_OR_EQUAL_TO     "\xE2\x8B\xA3" /* U+22E3 */
#define AGSI_SQUARE_IMAGE_OF_OR_NOT_EQUAL_TO        "\xE2\x8B\xA4" /* U+22E4 */
#define AGSI_SQUARE_ORIGINAL_OF_OR_NOT_EQUAL_TO     "\xE2\x8B\xA5" /* U+22E5 */
#define AGSI_LESS_THAN_BUT_NOT_EQUIVALENT_TO        "\xE2\x8B\xA6" /* U+22E6 */
#define AGSI_GREATER_THAN_BUT_NOT_EQUIVALENT_TO     "\xE2\x8B\xA7" /* U+22E7 */
#define AGSI_PRECEDES_BUT_NOT_EQUIVALENT_TO         "\xE2\x8B\xA8" /* U+22E8 */
#define AGSI_SUCCEEDS_BUT_NOT_EQUIVALENT_TO         "\xE2\x8B\xA9" /* U+22E9 */
#define AGSI_NOT_NORMAL_SUBGROUP_OF                 "\xE2\x8B\xAA" /* U+22EA */
#define AGSI_DOES_NOT_CONTAIN_AS_NORMAL_SUBGROUP    "\xE2\x8B\xAB" /* U+22EB */
#define AGSI_NOT_NORMAL_SUBGROUP_OF_OR_EQUAL_TO     "\xE2\x8B\xAC" /* U+22EC */
#define AGSI_DOES_NOT_CONTAIN_AS_NORMAL_SUBGROUP_OR_EQUAL "\xE2\x8B\xAD" /* U+22ED */
#define AGSI_VERTICAL_ELLIPSIS                      "\xE2\x8B\xAE" /* U+22EE */
#define AGSI_MIDLINE_HORIZONTAL_ELLIPSIS            "\xE2\x8B\xAF" /* U+22EF */
#define AGSI_UP_RIGHT_DIAGONAL_ELLIPSIS             "\xE2\x8B\xB0" /* U+22F0 */
#define AGSI_DOWN_RIGHT_DIAGONAL_ELLIPSIS           "\xE2\x8B\xB1" /* U+22F1 */
#define AGSI_ELEMENT_OF_WITH_LONG_HORIZONTAL_STROKE "\xE2\x8B\xB2" /* U+22F2 */
#define AGSI_ELEMENT_OF_WITH_VERTICAL_BAR_AT_END_OF_HORIZONTAL_STROKE "\xE2\x8B\xB3" /* U+22F3 */
#define AGSI_SMALL_ELEMENT_OF_WITH_VERTICAL_BAR_AT_END_OF_HORIZONTAL_STROKE "\xE2\x8B\xB4" /* U+22F4 */
#define AGSI_ELEMENT_OF_WITH_DOT_ABOVE              "\xE2\x8B\xB5" /* U+22F5 */
#define AGSI_ELEMENT_OF_WITH_OVERBAR                "\xE2\x8B\xB6" /* U+22F6 */
#define AGSI_SMALL_ELEMENT_OF_WITH_OVERBAR          "\xE2\x8B\xB7" /* U+22F7 */
#define AGSI_ELEMENT_OF_WITH_UNDERBAR               "\xE2\x8B\xB8" /* U+22F8 */
#define AGSI_ELEMENT_OF_WITH_TWO_HORIZONTAL_STROKES "\xE2\x8B\xB9" /* U+22F9 */
#define AGSI_CONTAINS_WITH_LONG_HORIZONTAL_STROKE   "\xE2\x8B\xBA" /* U+22FA */
#define AGSI_CONTAINS_WITH_VERTICAL_BAR_AT_END_OF_HORIZONTAL_STROKE "\xE2\x8B\xBB" /* U+22FB */
#define AGSI_SMALL_CONTAINS_WITH_VERTICAL_BAR_AT_END_OF_HORIZONTAL_STROKE "\xE2\x8B\xBC" /* U+22FC */
#define AGSI_CONTAINS_WITH_OVERBAR                  "\xE2\x8B\xBD" /* U+22FD */
#define AGSI_SMALL_CONTAINS_WITH_OVERBAR            "\xE2\x8B\xBE" /* U+22FE */
#define AGSI_Z_NOTATION_BAG_MEMBERSHIP              "\xE2\x8B\xBF" /* U+22FF */
/*
 * Miscellaneous Technical.
 */
#define AGSI_DIAMETER_SIGN                          "\xE2\x8C\x80" /* U+2300 */
#define AGSI_ELECTRIC_ARROW                         "\xE2\x8C\x81" /* U+2301 */
#define AGSI_HOUSE                                  "\xE2\x8C\x82" /* U+2302 */
#define AGSI_UP_ARROWHEAD                           "\xE2\x8C\x83" /* U+2303 */
#define AGSI_DOWN_ARROWHEAD                         "\xE2\x8C\x84" /* U+2304 */
#define AGSI_PROJECTIVE                             "\xE2\x8C\x85" /* U+2305 */
#define AGSI_PERSPECTIVE                            "\xE2\x8C\x86" /* U+2306 */
#define AGSI_WAVY_LINE                              "\xE2\x8C\x87" /* U+2307 */
#define AGSI_LEFT_CEILING                           "\xE2\x8C\x88" /* U+2308 */
#define AGSI_RIGHT_CEILING                          "\xE2\x8C\x89" /* U+2309 */
#define AGSI_LEFT_FLOOR                             "\xE2\x8C\x8A" /* U+230A */
#define AGSI_RIGHT_FLOOR                            "\xE2\x8C\x8B" /* U+230B */
#define AGSI_BOTTOM_RIGHT_CROP                      "\xE2\x8C\x8C" /* U+230C */
#define AGSI_BOTTOM_LEFT_CROP                       "\xE2\x8C\x8D" /* U+230D */
#define AGSI_TOP_RIGHT_CROP                         "\xE2\x8C\x8E" /* U+230E */
#define AGSI_TOP_LEFT_CROP                          "\xE2\x8C\x8F" /* U+230F */
#define AGSI_REVERSED_NOT_SIGN                      "\xE2\x8C\x90" /* U+2310 */
#define AGSI_SQUARE_LOZENGE                         "\xE2\x8C\x91" /* U+2311 */
#define AGSI_PLACE_OF_INTEREST_SIGN                 "\xE2\x8C\x98" /* U+2318 */
#define AGSI_TURNED_NOT_SIGN                        "\xE2\x8C\x99" /* U+2319 */
#define AGSI_TOP_LEFT_CORNER                        "\xE2\x8C\x9C" /* U+231C */
#define AGSI_TOP_RIGHT_CORNER                       "\xE2\x8C\x9D" /* U+231D */
#define AGSI_BOTTOM_LEFT_CORNER                     "\xE2\x8C\x9E" /* U+231E */
#define AGSI_BOTTOM_RIGHT_CORNER                    "\xE2\x8C\x9F" /* U+231F */
#define AGSI_TOP_HALF_INTEGRAL                      "\xE2\x8C\xA0" /* U+2320 */
#define AGSI_BOTTOM_HALF_INTEGRAL                   "\xE2\x8C\xA1" /* U+2321 */
#define AGSI_UP_ARROWHEAD_BETWEEN_TWO_HORIZONTAL_BARS "\xE2\x8C\xA4" /* U+2324 */
#define AGSI_OPTION_KEY                             "\xE2\x8C\xA5" /* U+2325 */
#define AGSI_ERASE_TO_THE_RIGHT                     "\xE2\x8C\xA6" /* U+2326 */
#define AGSI_X_IN_A_RECTANGLE_BOX                   "\xE2\x8C\xA7" /* U+2327 */
#define AGSI_KEYBOARD                               "\xE2\x8C\xA8" /* U+2328 */
#define AGSI_ERASE_TO_THE_LEFT                      "\xE2\x8C\xAB" /* U+232B */
#define AGSI_BENZENE_RING                           "\xE2\x8C\xAC" /* U+232C */
#define AGSI_APL_FUNCTIONAL_SYMBOL_IOTA             "\xE2\x8D\xB3" /* U+2373 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_RHO              "\xE2\x8D\xB4" /* U+2374 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_OMEGA            "\xE2\x8D\xB5" /* U+2375 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_ALPHA            "\xE2\x8D\xBA" /* U+237A */
#define AGSI_SHOULDERED_OPEN_BOX                    "\xE2\x8D\xBD" /* U+237D */
#define AGSI_ALTERNATIVE_KEY_SYMBOL                 "\xE2\x8E\x87" /* U+2387 */
#define AGSI_SOFTWARE_FUNCTION_SYMBOL               "\xE2\x8E\x94" /* U+2394 */
#define AGSI_LEFT_PARENTHESIS_UPPER_HOOK            "\xE2\x8E\x9B" /* U+239B */
#define AGSI_LEFT_PARENTHESIS_EXTENSION             "\xE2\x8E\x9C" /* U+239C */
#define AGSI_LEFT_PARENTHESIS_LOWER_HOOK            "\xE2\x8E\x9D" /* U+239D */
#define AGSI_RIGHT_PARENTHESIS_UPPER_HOOK           "\xE2\x8E\x9E" /* U+239E */
#define AGSI_RIGHT_PARENTHESIS_EXTENSION            "\xE2\x8E\x9F" /* U+239F */
#define AGSI_RIGHT_PARENTHESIS_LOWER_HOOK           "\xE2\x8E\xA0" /* U+23A0 */
#define AGSI_LEFT_SQUARE_BRACKET_UPPER_CORNER       "\xE2\x8E\xA1" /* U+23A1 */
#define AGSI_LEFT_SQUARE_BRACKET_EXTENSION          "\xE2\x8E\xA2" /* U+23A2 */
#define AGSI_LEFT_SQUARE_BRACKET_LOWER_CORNER       "\xE2\x8E\xA3" /* U+23A3 */
#define AGSI_RIGHT_SQUARE_BRACKET_UPPER_CORNER      "\xE2\x8E\xA4" /* U+23A4 */
#define AGSI_RIGHT_SQUARE_BRACKET_EXTENSION         "\xE2\x8E\xA5" /* U+23A5 */
#define AGSI_RIGHT_SQUARE_BRACKET_LOWER_CORNER      "\xE2\x8E\xA6" /* U+23A6 */
#define AGSI_LEFT_CURLY_BRACKET_UPPER_HOOK          "\xE2\x8E\xA7" /* U+23A7 */
#define AGSI_LEFT_CURLY_BRACKET_MIDDLE_PIECE        "\xE2\x8E\xA8" /* U+23A8 */
#define AGSI_LEFT_CURLY_BRACKET_LOWER_HOOK          "\xE2\x8E\xA9" /* U+23A9 */
#define AGSI_CURLY_BRACKET_EXTENSION                "\xE2\x8E\xAA" /* U+23AA */
#define AGSI_RIGHT_CURLY_BRACKET_UPPER_HOOK         "\xE2\x8E\xAB" /* U+23AB */
#define AGSI_RIGHT_CURLY_BRACKET_MIDDLE_PIECE       "\xE2\x8E\xAC" /* U+23AC */
#define AGSI_RIGHT_CURLY_BRACKET_LOWER_HOOK         "\xE2\x8E\xAD" /* U+23AD */
#define AGSI_INTEGRAL_EXTENSION                     "\xE2\x8E\xAE" /* U+23AE */
#define AGSI_RETURN_SYMBOL                          "\xE2\x8F\x8E" /* U+23CE */
#define AGSI_EJECT_SYMBOL                           "\xE2\x8F\x8F" /* U+23CF */
#define AGSI_BENZENE_RING_WITH_CIRCLE               "\xE2\x8F\xA3" /* U+23E3 */
#define AGSI_FLATNESS                               "\xE2\x8F\xA5" /* U+23E5 */
#define AGSI_DECIMAL_EXPONENT_SYMBOL                "\xE2\x8F\xA8" /* U+23E8 */
#define AGSI_STOPWATCH                              "\xE2\x8F\xB1" /* U+23F1 */
/*
 * Control Pictures.
 */
#define AGSI_BLANK_SYMBOL                           "\xE2\x90\xA2" /* U+2422 */
#define AGSI_OPEN_BOX                               "\xE2\x90\xA3" /* U+2423 */
/*
 * Enclosed Alphanumerics.
 */
#define AGSI_CIRCLED_DIGIT_1                        "\xE2\x91\xA0" /* U+2460 */
#define AGSI_CIRCLED_DIGIT_2                        "\xE2\x91\xA1" /* U+2461 */
#define AGSI_CIRCLED_DIGIT_3                        "\xE2\x91\xA2" /* U+2462 */
#define AGSI_CIRCLED_DIGIT_4                        "\xE2\x91\xA3" /* U+2463 */
#define AGSI_CIRCLED_DIGIT_5                        "\xE2\x91\xA4" /* U+2464 */
#define AGSI_CIRCLED_DIGIT_6                        "\xE2\x91\xA5" /* U+2465 */
#define AGSI_CIRCLED_DIGIT_7                        "\xE2\x91\xA6" /* U+2466 */
#define AGSI_CIRCLED_DIGIT_8                        "\xE2\x91\xA7" /* U+2467 */
#define AGSI_CIRCLED_DIGIT_9                        "\xE2\x91\xA8" /* U+2468 */
#define AGSI_CIRCLED_DIGIT_10                       "\xE2\x91\xA9" /* U+2469 */

#define AGSI_DOTTED_CIRCLE                          "\xE2\x97\x8C" /* U+25CC */

#define AGSI_WHEEL_OF_DHARMA                        "\xE2\x98\xB8" /* U+2638 */
#define AGSI_GEAR                                   "\xE2\x9A\x99" /* U+2699 */

#define AGSI_VICTORY_HAND                           "\xE2\x9C\x8C" /* U+270C */
#define AGSI_WRITING_HAND                           "\xE2\x9C\x8D" /* U+270D */
#define AGSI_LOWER_RIGHT_PENCIL                     "\xE2\x9C\x8E" /* U+270E */
#define AGSI_PENCIL                                 "\xE2\x9C\x8F" /* U+270F */
#define AGSI_UPPER_RIGHT_PENCIL                     "\xE2\x9C\x90" /* U+2710 */
#define AGSI_WHITE_NIB                              "\xE2\x9C\x91" /* U+2711 */
#define AGSI_BLACK_NIB                              "\xE2\x9C\x92" /* U+2712 */
#define AGSI_CHECK_MARK                             "\xE2\x9C\x93" /* U+2713 */
#define AGSI_HEAVY_CHECK_MARK                       "\xE2\x9C\x94" /* U+2714 */
#define AGSI_MULTIPLICATION_X                       "\xE2\x9C\x95" /* U+2715 */
#define AGSI_HEAVY_MULTIPLICATION_X                 "\xE2\x9C\x96" /* U+2716 */
#define AGSI_BALLOT_X                               "\xE2\x9C\x97" /* U+2717 */
#define AGSI_HEAVY_BALLOT_X                         "\xE2\x9C\x98" /* U+2718 */
#define AGSI_CLOSE_X AGSI_MULTIPLICATION_X
#define AGSI_BLK_4_POINTED_STAR                     "\xE2\x9C\xA6" /* U+2726 */
#define AGSI_WHT_4_POINTED_STAR                     "\xE2\x9C\xA7" /* U+2727 */
#define AGSI_INVERTED_LAZY_S                        "\xE2\x88\xBE" /* U+223E */
#define AGSI_SINE_WAVE                              "\xE2\x88\xBF" /* U+223F */
#define AGSI_WREATH_PRODUCT                         "\xE2\x89\x80" /* U+2240 */
#define AGSI_NOT_TILDE                              "\xE2\x89\x81" /* U+2241 */
#define AGSI_MINUS_TILDE                            "\xE2\x89\x82" /* U+2242 */




/*
 * Miscellaneous Symbols and Pictographs.
 */
#define AGSI_NEW_MOON             "\xF0\x9F\x8C\x91"	/* U+1F311 */
#define AGSI_WAXING_CRESCENT_MOON "\xF0\x9F\x8C\x92"	/* U+1F312 */
#define AGSI_FIRST_QUARTER_MOON   "\xF0\x9F\x8C\x93"	/* U+1F313 */
#define AGSI_WAXING_GIBBOUS_MOON  "\xF0\x9F\x8C\x94"	/* U+1F314 */
#define AGSI_FULL_MOON            "\xF0\x9F\x8C\x95"	/* U+1F315 */
#define AGSI_WANING_GIBBOUS_MOON  "\xF0\x9F\x8C\x96"	/* U+1F316 */
#define AGSI_LAST_QUARTER_MOON    "\xF0\x9F\x8C\x97"	/* U+1F317 */
#define AGSI_WANING_CRESCENT_MOON "\xF0\x9F\x8C\x98"	/* U+1F318 */

#define AGSI_ANT         "\xF0\x9F\x90\x9C"		/* U+1F41C */
#define AGSI_MOUSE_FACE  "\xF0\x9F\x90\xAD"		/* U+1F42D */
#define AGSI_COW_FACE    "\xF0\x9F\x90\xAE"		/* U+1F42E */
#define AGSI_CAT_FACE    "\xF0\x9F\x90\xB1"		/* U+1F431 */
#define AGSI_MONKEY_FACE "\xF0\x9F\x90\xB5"		/* U+1F435 */
#define AGSI_PILE_OF_POO "\xF0\x9F\x92\xA9"		/* U+1F4A9 */
/*
 * Emoticons ("F" = "FACE", "SM" = "SMILING").
 */
#define AGSI_GRINNING_F "\xF0\x9F\x98\x80"			/* U+1F600 */
#define AGSI_GRINNING_F_W_SM_EYES "\xF0\x9F\x98\x81"		/* U+1F601 */
#define AGSI_F_W_TEARS_OF_JOY "\xF0\x9F\x98\x82"		/* U+1F602 */
#define AGSI_SM_F_W_OPEN_MOUTH "\xF0\x9F\x98\x83"		/* U+1F603 */
#define AGSI_SM_F_W_OPEN_MOUTH_AND_SM_EYES "\xF0\x9F\x98\x84"	/* U+1F604 */
#define AGSI_SM_F_W_OPEN_MOUTH_AND_COLD_SWEAT "\xF0\x9F\x98\x85"/* U+1F605 */
#define AGSI_SM_F_W_OPEN_MOUTH_AND_TIGHTLY_CLOSED_EYES "\xF0\x9F\x98\x86" /* U+1F606 */
#define AGSI_SM_F_W_HALO "\xF0\x9F\x98\x87"			/* U+1F607 */
#define AGSI_SM_F_W_HORNS "\xF0\x9F\x98\x88"			/* U+1F608 */
#define AGSI_WINKING_F "\xF0\x9F\x98\x89"			/* U+1F609 */
#define AGSI_SM_F_W_SM_EYES "\xF0\x9F\x98\x8A"			/* U+1F60A */
#define AGSI_F_SAVORING_DELICIOUS_FOOD "\xF0\x9F\x98\x8B"	/* U+1F60B */
#define AGSI_RELIEVED_F "\xF0\x9F\x98\x8C"			/* U+1F60C */
#define AGSI_SM_F_W_HEART_SHAPED_EYES "\xF0\x9F\x98\x8D"	/* U+1F60D */
#define AGSI_SM_F_W_SUNGLASSES "\xF0\x9F\x98\x8E"		/* U+1F60E */
#define AGSI_SMIRKING_F "\xF0\x9F\x98\x8F"			/* U+1F60F */
#define AGSI_NEUTRAL_F "\xF0\x9F\x98\x90"			/* U+1F610 */
#define AGSI_EXPRESSIONLESS_F "\xF0\x9F\x98\x91"		/* U+1F611 */
#define AGSI_UNAMUSED_F "\xF0\x9F\x98\x92"			/* U+1F612 */
#define AGSI_F_W_COLD_SWEAT "\xF0\x9F\x98\x93"			/* U+1F613 */
#define AGSI_PENSIVE_F "\xF0\x9F\x98\x94"			/* U+1F614 */
#define AGSI_CONFUSED_F "\xF0\x9F\x98\x95"			/* U+1F615 */
#define AGSI_CONFOUNDED_F "\xF0\x9F\x98\x96"			/* U+1F616 */
#define AGSI_KISSING_F "\xF0\x9F\x98\x97"			/* U+1F617 */
#define AGSI_F_THROWING_A_KISS "\xF0\x9F\x98\x98"		/* U+1F618 */
#define AGSI_KISSING_F_W_SM_EYES "\xF0\x9F\x98\x99"		/* U+1F619 */
#define AGSI_KISSING_F_W_CLOSED_EYES "\xF0\x9F\x98\x9A"		/* U+1F61A */
#define AGSI_F_W_STUCK_OUT_TONGUE "\xF0\x9F\x98\x9B"		/* U+1F61B */
#define AGSI_F_W_STUCK_OUT_TONGUE_AND_WINKING_EYE "\xF0\x9F\x98\x9C" /* U+1F61C */
#define AGSI_F_W_STUCK_OUT_TONGUE_AND_TIGHTLY_CLOSED_EYES "\xF0\x9F\x98\x9D" /* U+1F61D */
#define AGSI_DISAPPOINTED_F "\xF0\x9F\x98\x9E"			/* U+1F61E */
#define AGSI_WORRIED_F "\xF0\x9F\x98\x9F"			/* U+1F61F */
#define AGSI_ANGRY_F "\xF0\x9F\x98\xA0"				/* U+1F620 */
#define AGSI_POUTING_F "\xF0\x9F\x98\xA1"			/* U+1F621 */
#define AGSI_CRYING_F "\xF0\x9F\x98\xA2"			/* U+1F622 */
#define AGSI_PERSEVERING_F "\xF0\x9F\x98\xA3"			/* U+1F623 */
#define AGSI_F_W_LOOK_OF_TRIUMPH "\xF0\x9F\x98\xA4"		/* U+1F624 */
#define AGSI_DISAPPOINTED_BUT_RELIEVED_F "\xF0\x9F\x98\xA5"	/* U+1F625 */
#define AGSI_FROWNING_F_W_OPEN_MOUTH "\xF0\x9F\x98\xA6"		/* U+1F626 */
#define AGSI_ANGUISHED_F "\xF0\x9F\x98\xA7"			/* U+1F627 */
#define AGSI_FEARFUL_F "\xF0\x9F\x98\xA8"			/* U+1F628 */
#define AGSI_WEARY_F "\xF0\x9F\x98\xA9"				/* U+1F629 */
#define AGSI_SLEEPY_F "\xF0\x9F\x98\xAA"			/* U+1F62A */
#define AGSI_TIRED_F "\xF0\x9F\x98\xAB"				/* U+1F62B */
#define AGSI_GRIMACING_F "\xF0\x9F\x98\xAC"			/* U+1F62C */
#define AGSI_LOUDLY_CRYING_F "\xF0\x9F\x98\xAD"			/* U+1F62D */
#define AGSI_F_W_OPEN_MOUTH "\xF0\x9F\x98\xAE"			/* U+1F62E */
#define AGSI_HUSHED_F "\xF0\x9F\x98\xAF"			/* U+1F62F */
#define AGSI_F_W_OPEN_MOUTH_AND_COLD_SWEAT "\xF0\x9F\x98\xB0"	/* U+1F630 */
#define AGSI_F_SCREAMING_IN_FEAR "\xF0\x9F\x98\xB1"		/* U+1F631 */
#define AGSI_ASTONISHED_F "\xF0\x9F\x98\xB2"			/* U+1F632 */
#define AGSI_FLUSHED_F "\xF0\x9F\x98\xB3"			/* U+1F633 */
#define AGSI_SLEEPING_F "\xF0\x9F\x98\xB4"			/* U+1F634 */
#define AGSI_DIZZY_F "\xF0\x9F\x98\xB5"				/* U+1F635 */
#define AGSI_F_WO_MOUTH "\xF0\x9F\x98\xB6"			/* U+1F636 */
#define AGSI_F_MEDICAL_MASK "\xF0\x9F\x98\xB7"			/* U+1F637 */
#define AGSI_GRINNING_CAT_F_W_SM_EYES "\xF0\x9F\x98\xB8"	/* U+1F638 */
#define AGSI_CAT_F_W_TEARS_OF_JOY "\xF0\x9F\x98\xB9"		/* U+1F639 */
#define AGSI_SM_CAT_F_W_OPEN_MOUTH "\xF0\x9F\x98\xBA"		/* U+1F63A */
#define AGSI_SM_CAT_F_W_HEART_SHAPED_EYES "\xF0\x9F\x98\xBB"	/* U+1F63B */
#define AGSI_CAT_F_W_WRY_SMILE "\xF0\x9F\x98\xBC"		/* U+1F63C */
#define AGSI_KISSING_CAT_F_W_CLOSED_EYES "\xF0\x9F\x98\xBD"	/* U+1F63D */
#define AGSI_POUTING_CAT_F "\xF0\x9F\x98\xBE"			/* U+1F63E */
#define AGSI_CRYING_CAT_F "\xF0\x9F\x98\xBF"			/* U+1F63F */
#define AGSI_WEARY_CAT_F "\xF0\x9F\x99\x80"			/* U+1F640 */
#define AGSI_UPSIDE_DOWN_F "\xF0\x9F\x99\x83"			/* U+1F643 */

/*
 * Private Use Area.
 */
#define AGSI_BLACK_AGAR     "\xEE\x80\x80"  /* U+E000 Agar Logo Filled */
#define AGSI_WHITE_AGAR     "\xEE\x80\x81"  /* U+E001 Agar Logo Outline */
#define AGSI_MENUBOOL_TRUE  "\xEE\x80\x82"  /* U+E002 AG_Menu(3) Boolean True */
#define AGSI_MENUBOOL_FALSE "\xEE\x80\x83"  /* U+E003 AG_Menu(3) Boolean False */
#define AGSI_KEYMOD_HYPHEN  "\xEE\x80\x84"  /* U+E004 Keyboard-Modifier Hyphen */
#define AGSI_MENU_EXPANDER  "\xEE\x80\x85"  /* U+E005 Menu Expansion Arrow */
#define AGSI_BOX_VERT       "\xEE\x80\x90"  /* U+E010 Vertical AG_Box(3) */
#define AGSI_BOX_HORIZ      "\xEE\x80\x91"  /* U+E011 Horizontal AG_Box(3) */
#define AGSI_BUTTON         "\xEE\x80\x92"  /* U+E012 AG_Button(3) */

