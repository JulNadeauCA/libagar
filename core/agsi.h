/*	Public domain	*/

/*
 * Agar Extended ANSI Escape Sequences.
 *
 * See: https://en.wikipedia.org/wiki/ANSI_escape_code
 */

#ifdef AG_ANSI_COLOR

# define AGSI_RST	 "\x1b[0m"   /* reset all attributes */
# define AGSI_BOLD     	 "\x1b[1m"   /* bold */
# define AGSI_FAINT    	 "\x1b[2m"   /* faint or semibold */
# define AGSI_ITALIC   	 "\x1b[3m"   /* italic style */
# define AGSI_UNDERLINE	 "\x1b[4m"   /* underlined */
# define AGSI_REVERSE  	 "\x1b[7m"   /* reverse video */
# define AGSI_CROSSEDOUT "\x1b[9m"   /* crossed-out */
# define AGSI_SUP        "\x1b[73m"  /* superscript */
# define AGSI_SUB        "\x1b[74m"  /* subscript */

# define AGSI_FONT1	 "\x1b[10m"  /* Core Font #1 */
# define AGSI_FONT2	 "\x1b[11m"  /* Core Font #2 */ 
# define AGSI_FONT3	 "\x1b[12m"  /* Core Font #3 */
# define AGSI_FONT4	 "\x1b[13m"  /* Core Font #4 */
# define AGSI_FONT5	 "\x1b[14m"  /* Core Font #5 */
# define AGSI_FONT6	 "\x1b[15m"  /* Core Font #6 */
# define AGSI_FONT7	 "\x1b[16m"  /* Core Font #7 */
# define AGSI_FONT8	 "\x1b[17m"  /* Core Font #8 */
# define AGSI_FONT9	 "\x1b[18m"  /* Core Font #9 */
# define AGSI_FONT10	 "\x1b[19m"  /* Core Font #10 */
# define AGSI_FONT11	 "\x1b[20m"  /* Core Font #11 */
# define AGSI_FONT12	 "\x1b[66m"  /* Core Font #12 */
# define AGSI_FONT13	 "\x1b[67m"  /* Core Font #13 */
# define AGSI_FONT14	 "\x1b[68m"  /* Core Font #14 */
# define AGSI_FONT15	 "\x1b[69m"  /* Core Font #15 */
# define AGSI_FONT16	 "\x1b[70m"  /* Core Font #16 */
# define AGSI_FONT17	 "\x1b[71m"  /* Core Font #17 */

# define AGSI_FRAMED	 "\x1b[51m"  /* framed */
# define AGSI_ENCIRCLED  "\x1b[52m"  /* encircled */
# define AGSI_OVERLINED  "\x1b[53m"  /* overlined */
# define AGSI_NOTFRAMED  "\x1b[54m"  /* neither framed nor encircled */
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
# define AGSI_SUP	 ""
# define AGSI_SUB	 ""
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
# define AGSI_FONT12	 ""
# define AGSI_FONT13	 ""
# define AGSI_FONT14	 ""
# define AGSI_FRAMED	 ""
# define AGSI_ENCIRCLED  ""
# define AGSI_OVERLINED  ""
# define AGSI_NOTFRAMED  ""
# define AGSI_BLK	 ""
# define AGSI_RED	 ""
# define AGSI_GRN	 ""
# define AGSI_YEL	 ""
# define AGSI_BLU	 ""
# define AGSI_MAG	 ""
# define AGSI_CYA	 ""
# define AGSI_WHT	 ""
# define AGSI_BR_BLK	 ""
# define AGSI_GRAY	 ""
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
# define AGSI_GRAY_BG	 ""
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
#define AGSI_ALGUE               AGSI_FONT1     /* Algue */
#define AGSI_UNIALGUE            AGSI_FONT2     /* Unialgue */
#define AGSI_AGAR_MINIMAL        AGSI_FONT3     /* Agar Minimal */
#define AGSI_AGAR_IDEOGRAMS      AGSI_FONT4     /* Agar Ideograms */
#define AGSI_MONOALGUE           AGSI_FONT5     /* Monoalgue */
#define AGSI_BITSTREAM_CHARTER   AGSI_FONT6     /* Bitstream Charter */
#define AGSI_NOTO_SERIF          AGSI_FONT7     /* Noto Serif */
#define AGSI_NOTO_SANS           AGSI_FONT8     /* Noto Sans */
#define AGSI_LEAGUE_SPARTAN      AGSI_FONT9     /* League Spartan */
#define AGSI_LEAGUE_GOTHIC       AGSI_FONT10    /* League Gothic */
#define AGSI_UNIFRAKTUR_MAGUNTIA AGSI_FONT11    /* Unifraktur Maguntia */
/* #define AGSI_UNUSED_FONT12    AGSI_FONT12 */
/* #define AGSI_UNUSED_FONT13    AGSI_FONT13 */
/* #define AGSI_UNUSED_FONT14    AGSI_FONT14 */
/* #define AGSI_UNUSED_FONT15    AGSI_FONT15 */
/* #define AGSI_UNUSED_FONT16    AGSI_FONT16 */
/* #define AGSI_UNUSED_FONT17    AGSI_FONT17 */

#define AGSI_MONOSPACE AGSI_MONOALGUE            /* A monospace font */
#define AGSI_UNI       AGSI_UNIALGUE             /* An extended unicode font */
#define AGSI_PATH      AGSI_MONOALGUE            /* A font for pathnames */
#define AGSI_CODE      AGSI_MONOALGUE            /* A programming font */
#define AGSI_COURIER   AGSI_MONOALGUE            /* A courier font */
#define AGSI_FRAKTUR   AGSI_UNIFRAKTUR_MAGUNTIA  /* A fraktur font */
#define AGSI_IDEOGRAM  AGSI_AGAR_IDEOGRAMS       /* A font with ideograms */
#define AGSI_CHARTER   AGSI_BITSTREAM_CHARTER    /* Bitstream Charter */
#define AGSI_NOTO      AGSI_NOTO_SANS            /* Noto Sans */
#define AGSI_CJK       AGSI_NOTO_SANS            /* A pan-CJK font */

/*
 * Map AGSI_APPCMD and AGSI_WINCMD to the preferred modifier keys for
 * application-global and window-global commands, respectively.
 */
#if defined(__APPLE2__) || defined(__BBC__) || defined(__PET__) || \
    defined(__VIC20__) || defined(__C64__) || defined(__C128__)
# define AGSI_APPCMD      AG_KEYMOD_SHIFT
# define AGSI_APPCMD_NAME "Shift-"
# define AGSI_WINCMD      AG_KEYMOD_SHIFT
# define AGSI_WINCMD_NAME "Shift-"
#elif defined(__APPLE__)
# define AGSI_APPCMD      AG_KEYMOD_META
# define AGSI_APPCMD_NAME "Command-"
# define AGSI_WINCMD      AG_KEYMOD_META
# define AGSI_WINCMD_NAME "Command-"
#else
# define AGSI_APPCMD      AG_KEYMOD_CTRL_SHIFT
# define AGSI_APPCMD_NAME "Ctrl-Shift-"
# define AGSI_WINCMD      AG_KEYMOD_CTRL
# define AGSI_WINCMD_NAME "Ctrl-"
#endif

/*
 * Characters available in core fonts ("W" = "WITH", "WO" = "WITHOUT").
 */

/*
 * General Punctuation ("SQ" = "SQUARE").
 */
#define AGSI_THIN_SPACE              "\xE2\x80\x89"	/* U+2009 */
#define AGSI_HYPHEN                  "\xE2\x80\x90"	/* U+2010 */
#define AGSI_NON_BREAKING_HYPHEN     "\xE2\x80\x91"	/* U+2011 */
#define AGSI_FIGURE_DASH             "\xE2\x80\x92"	/* U+2012 */
#define AGSI_EN_DASH                 "\xE2\x80\x93"	/* U+2013 */
#define AGSI_EM_DASH                 "\xE2\x80\x94"	/* U+2014 */
#define AGSI_HBAR                    "\xE2\x80\x95"	/* U+2015 Horizontal Bar */
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
#define AGSI_SUPERSCRIPT_L_PAREN     "\xE2\x81\xBD"	/* U+207D Superscript Left Parenthesis */
#define AGSI_SUPERSCRIPT_R_PAREN     "\xE2\x81\xBE"	/* U+207E Superscript Right Parenthesis */
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
#define AGSI_SUBSCRIPT_L_PAREN       "\xE2\x82\x8D"	/* U+208D Subscript Left Parenthesis */
#define AGSI_SUBSCRIPT_R_PAREN       "\xE2\x82\x8E"	/* U+208E Subscript Right Parenthesis */
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
 * Arrows ("L" = "LEFT", "U" = "UP", "R" = "RIGHT", "D" = "DOWN").
 */
#define AGSI_L_ARROW                  "\xE2\x86\x90" /* U+2190 Leftwards Arrow */
#define AGSI_UP_ARROW                 "\xE2\x86\x91" /* U+2191 Upwards Arrow */
#define AGSI_R_ARROW                  "\xE2\x86\x92" /* U+2192 Rightwards Arrow */
#define AGSI_DN_ARROW                 "\xE2\x86\x93" /* U+2193 Downwards Arrow */
#define AGSI_LR_ARROW                 "\xE2\x86\x94" /* U+2194 Left-Right Arrow */
#define AGSI_UPDN_ARROW               "\xE2\x86\x95" /* U+2195 Up-Down Arrow */
#define AGSI_NW_ARROW                 "\xE2\x86\x96" /* U+2196 North-West Arrow */
#define AGSI_NE_ARROW                 "\xE2\x86\x97" /* U+2197 North-East Arrow */
#define AGSI_SE_ARROW                 "\xE2\x86\x98" /* U+2198 South East Arrow */
#define AGSI_SW_ARROW                 "\xE2\x86\x99" /* U+2199 South West Arrow */
#define AGSI_L_ARROW_W_STROKE         "\xE2\x86\x9A" /* U+219A Leftwards Arrow With Stroke */
#define AGSI_R_ARROW_W_STROKE         "\xE2\x86\x9B" /* U+219B Rightwards Arrow With Stroke */
#define AGSI_L_WAVE_ARROW             "\xE2\x86\x9C" /* U+219C Leftwards Wave Arrow */
#define AGSI_R_WAVE_ARROW             "\xE2\x86\x9D" /* U+219D Rightwards Wave Arrow */
#define AGSI_L_TWO_HEADED_ARROW       "\xE2\x86\x9E" /* U+219E Leftwards Two Headed Arrow */
#define AGSI_UP_TWO_HEADED_ARROW      "\xE2\x86\x9F" /* U+219F Upwards Two Headed Arrow */
#define AGSI_R_TWO_HEADED_ARROW       "\xE2\x86\xA0" /* U+21A0 Rightwards Two Headed Arrow */
#define AGSI_DN_TWO_HEADED_ARROW      "\xE2\x86\xA1" /* U+21A1 Downwards Two Headed Arrow */
#define AGSI_L_ARROW_W_TAIL           "\xE2\x86\xA2" /* U+21A2 Leftwards Arrow With Tail */
#define AGSI_R_ARROW_W_TAIL           "\xE2\x86\xA3" /* U+21A3 Rightwards Arrow With Tail */
#define AGSI_L_ARROW_FROM_BAR         "\xE2\x86\xA4" /* U+21A4 Leftwards Arrow From Bar */
#define AGSI_UP_ARROW_FROM_BAR        "\xE2\x86\xA5" /* U+21A5 Upwards Arrow From Bar */
#define AGSI_R_ARROW_FROM_BAR         "\xE2\x86\xA6" /* U+21A6 Rightwards Arrow From Bar */
#define AGSI_DN_ARROW_FROM_BAR        "\xE2\x86\xA7" /* U+21A7 Downwards Arrow From Bar */
#define AGSI_UPDN_ARROW_W_BASE        "\xE2\x86\xA8" /* U+21A8 Up Down Arrow With Base */
#define AGSI_L_ARROW_W_HOOK           "\xE2\x86\xA9" /* U+21A9 Leftwards Arrow With Hook */
#define AGSI_R_ARROW_W_HOOK           "\xE2\x86\xAA" /* U+21AA Rightwards Arrow With Hook */
#define AGSI_L_ARROW_W_LOOP           "\xE2\x86\xAB" /* U+21AB Leftwards Arrow With Loop */
#define AGSI_R_ARROW_W_LOOP           "\xE2\x86\xAC" /* U+21AC Rightwards Arrow With Loop */
#define AGSI_LR_WAVE_ARROW            "\xE2\x86\xAD" /* U+21AD Left Right Wave Arrow */
#define AGSI_LR_ARROW_W_STROKE        "\xE2\x86\xAE" /* U+21AE Left Right Arrow With Stroke */
#define AGSI_DN_ZIGZAG_ARROW          "\xE2\x86\xAF" /* U+21AF Downwards Zigzag Arrow */
#define AGSI_UP_ARROW_W_TIP_L         "\xE2\x86\xB0" /* U+21B0 Upwards Arrow With Tip Leftwards */
#define AGSI_UP_ARROW_W_TIP_R         "\xE2\x86\xB1" /* U+21B1 Upwards Arrow With Tip Rightwards */
#define AGSI_DN_ARROW_W_TIP_L         "\xE2\x86\xB2" /* U+21B2 Downwards Arrow With Tip Leftwards */
#define AGSI_DN_ARROW_W_TIP_R         "\xE2\x86\xB3" /* U+21B3 Downwards Arrow With Tip Rightwards */
#define AGSI_R_ARROW_W_CORNER_DN      "\xE2\x86\xB4" /* U+21B4 Rightwards Arrow With Corner Downwards */
#define AGSI_DN_ARROW_W_CORNER_L      "\xE2\x86\xB5" /* U+21B5 Downwards Arrow With Corner Leftwards */
#define AGSI_CCW_TOP_SEMICIRCLE_ARROW "\xE2\x86\xB6" /* U+21B6 Anticlockwise Top Semicircle Arrow */
#define AGSI_CW_TOP_SEMICIRCLE_ARROW  "\xE2\x86\xB7" /* U+21B7 Clockwise Top Semicircle Arrow */
#define AGSI_NW_ARROW_TO_LONG_BAR     "\xE2\x86\xB8" /* U+21B8 North West Arrow To Long Bar */
#define AGSI_L_ARR2BAR_OVER_R_ARR2BAR "\xE2\x86\xB9" /* U+21B9 Leftwards Arrow To Bar Over Rightwards Arrow To Bar */
#define AGSI_CCW_OPEN_CIRCLE_ARROW    "\xE2\x86\xBA" /* U+21BA Anticlockwise Open Circle Arrow */
#define AGSI_CW_OPEN_CIRCLE_ARROW     "\xE2\x86\xBB" /* U+21BB Clockwise Open Circle Arrow */
#define AGSI_L_HARPOON_W_BARB_UP      "\xE2\x86\xBC" /* U+21BC Leftwards Harpoon With Barb Upwards */
#define AGSI_L_HARPOON_W_BARB_DN      "\xE2\x86\xBD" /* U+21BD Leftwards Harpoon With Barb Downwards */
#define AGSI_UP_HARPOON_W_BARB_R      "\xE2\x86\xBE" /* U+21BE Upwards Harpoon With Barb Rightwards */
#define AGSI_UP_HARPOON_W_BARB_L      "\xE2\x86\xBF" /* U+21BF Upwards Harpoon With Barb Leftwards */
#define AGSI_R_HARPOON_W_BARB_UP      "\xE2\x86\xC0" /* U+21C0 Rightwards Harpoon With Barb Upwards */
#define AGSI_R_HARPOON_W_BARB_DN      "\xE2\x86\xC1" /* U+21C1 Rightwards Harpoon With Barb Downwards */
#define AGSI_DN_HARPOON_W_BARB_R      "\xE2\x86\xC2" /* U+21C2 Downwards Harpoon With Barb Rightwards */
#define AGSI_DN_HARPOON_W_BARB_L      "\xE2\x86\xC3" /* U+21C3 Downwards Harpoon With Barb Leftwards */
#define AGSI_R_ARROW_OVER_L_ARROW     "\xE2\x86\xC4" /* U+21C4 Rightwards Arrow Over Leftwards Arrow */
#define AGSI_UP_ARROW_L_OF_DN_ARROW   "\xE2\x86\xC5" /* U+21C5 Upwards Arrow Leftwards Of Downwards Arrow */
#define AGSI_L_ARROW_OVER_R_ARROW     "\xE2\x86\xC6" /* U+21C6 Leftwards Arrow Over Rightwards Arrow */
#define AGSI_L_PAIRED_ARROWS          "\xE2\x86\xC7" /* U+21C7 Leftwards Paired Arrows */
#define AGSI_UP_PAIRED_ARROWS         "\xE2\x86\xC8" /* U+21C8 Upwards Paired Arrows */
#define AGSI_R_PAIRED_ARROWS          "\xE2\x86\xC9" /* U+21C9 Rightwards Paired Arrows */
#define AGSI_DN_PAIRED_ARROWS         "\xE2\x86\xCA" /* U+21CA Downwards Paired Arrows */
#define AGSI_L_HARPOON_OVER_R_HARPOON "\xE2\x86\xCB" /* U+21CB Leftwards Harpoon Over Rightwards Harpoon */
#define AGSI_R_HARPOON_OVER_L_HARPOON "\xE2\x86\xCC" /* U+21CC Rightwards Harpoon Over Leftwards Harpoon */
#define AGSI_L_DOUBLE_ARROW_W_STROKE  "\xE2\x86\xCD" /* U+21CD Leftwards Double Arrow With Stroke */
#define AGSI_LR_DOUBLE_ARROW_W_STROKE "\xE2\x86\xCE" /* U+21CE Left Right Double Arrow With Stroke */
#define AGSI_R_DOUBLE_ARROW_W_STROKE  "\xE2\x86\xCF" /* U+21CF Right Double Arrow With Stroke */
#define AGSI_L_DOUBLE_ARROW           "\xE2\x86\xD0" /* U+21D0 Leftwards Double Arrow */
#define AGSI_UP_DOUBLE_ARROW          "\xE2\x86\xD1" /* U+21D1 Upwards Double Arrow */
#define AGSI_R_DOUBLE_ARROW           "\xE2\x86\xD2" /* U+21D2 Rightwards Double Arrow */
#define AGSI_DN_DOUBLE_ARROW          "\xE2\x86\xD3" /* U+21D3 Downwards Double Arrow */
#define AGSI_LR_DOUBLE_ARROW          "\xE2\x86\xD4" /* U+21D4 Left Right Double Arrow */
#define AGSI_UPDN_DOUBLE_ARROW        "\xE2\x86\xD5" /* U+21D5 Up Down Double Arrow */
#define AGSI_NW_DOUBLE_ARROW          "\xE2\x86\xD6" /* U+21D6 North West Double Arrow */
#define AGSI_NE_DOUBLE_ARROW          "\xE2\x86\xD7" /* U+21D7 North East Double Arrow */
#define AGSI_SE_DOUBLE_ARROW          "\xE2\x86\xD8" /* U+21D8 South East Double Arrow */
#define AGSI_SW_DOUBLE_ARROW          "\xE2\x86\xD9" /* U+21D9 South West Double Arrow */
#define AGSI_L_TRIPLE_ARROW           "\xE2\x86\xDA" /* U+21DA Leftwards Triple Arrow */
#define AGSI_R_TRIPLE_ARROW           "\xE2\x86\xDB" /* U+21DB Rightwards Triple Arrow */
#define AGSI_L_SQUIGGLE_ARROW         "\xE2\x86\xDC" /* U+21DC Leftwards Squiggle Arrow */
#define AGSI_R_SQUIGGLE_ARROW         "\xE2\x86\xDD" /* U+21DD Rightwards Squiggle Arrow */
#define AGSI_UP_ARROW_W_DOUBLE_STROKE "\xE2\x86\xDE" /* U+21DE Upwards Arrow With Double Stroke */
#define AGSI_DN_ARROW_W_DOUBLE_STROKE "\xE2\x86\xDF" /* U+21DF Downwards Arrow With Double Stroke */
#define AGSI_L_DASHED_ARROW           "\xE2\x86\xE0" /* U+21E0 Leftwards Dashed Arrow */
#define AGSI_UP_DASHED_ARROW          "\xE2\x86\xE1" /* U+21E1 Upwards Dashed Arrow */
#define AGSI_R_DASHED_ARROW           "\xE2\x86\xE2" /* U+21E2 Rightwards Dashed Arrow */
#define AGSI_DN_DASHED_ARROW          "\xE2\x86\xE3" /* U+21E3 Downwards Dashed Arrow */
#define AGSI_L_ARR2BAR                "\xE2\x86\xE4" /* U+21E4 Leftwards Arrow To Bar */
#define AGSI_R_ARR2BAR                "\xE2\x86\xE5" /* U+21E5 Rightwards Arrow To Bar */
#define AGSI_L_WHT_ARROW              "\xE2\x86\xE6" /* U+21E6 Leftwards White Arrow */
#define AGSI_UP_WHT_ARROW             "\xE2\x86\xE7" /* U+21E7 Upwards White Arrow */
#define AGSI_R_WHT_ARROW              "\xE2\x86\xE8" /* U+21E8 Rightwards White Arrow */
#define AGSI_DN_WHT_ARROW             "\xE2\x86\xE9" /* U+21E9 Downwards White Arrow */
#define AGSI_UP_WHT_ARROW_FROM_BAR    "\xE2\x86\xEA" /* U+21EA Upwards White Arrow From Bar */
#define AGSI_UP_WHT_ARROW_PED         "\xE2\x86\xEB" /* U+21EB Upwards White Arrow On Pedestal */
#define AGSI_UP_WHT_ARROW_PED_W_HBAR  "\xE2\x86\xEC" /* U+21EC Upwards White Arrow On Pedestal With Horizontal Bar */
#define AGSI_UP_WHT_ARROW_PED_W_VBAR  "\xE2\x86\xED" /* U+21ED Upwards White Arrow On Pedestal With Vertical Bar */
#define AGSI_UP_WHT_DOUBLE_ARROW      "\xE2\x86\xEE" /* U+21EE Upwards White Double Arrow */
#define AGSI_UP_WHT_DOUBLE_ARROW_PED  "\xE2\x86\xEF" /* U+21EF Upwards White Double Arrow On Pedestal */
#define AGSI_R_WHT_ARROW_FROM_WALL    "\xE2\x86\xF0" /* U+21F0 Rightwards White Arrow From Wall */
#define AGSI_NW_ARROW_TO_CORNER       "\xE2\x86\xF1" /* U+21F1 North West Arrow To Corner */
#define AGSI_SE_ARROW_TO_CORNER       "\xE2\x86\xF2" /* U+21F2 South East Arrow To Corner */
#define AGSI_UPDN_WHT_ARROW           "\xE2\x86\xF3" /* U+21F3 Up Down White Arrow */
#define AGSI_R_ARROW_W_SMALL_CIRCLE   "\xE2\x86\xF4" /* U+21F4 Right Arrow With Small Circle */
#define AGSI_DN_ARROW_L_OF_UP_ARROW   "\xE2\x86\xF5" /* U+21F5 Downwards Arrow Leftwards Of Upwards Arrow */
#define AGSI_THREE_R_ARROWS           "\xE2\x86\xF6" /* U+21F6 Three Rightwards Arrows */
#define AGSI_L_ARROW_W_VSTROKE        "\xE2\x86\xF7" /* U+21F7 Leftwards Arrow With Vertical Stroke */
#define AGSI_R_ARROW_W_VSTROKE        "\xE2\x86\xF8" /* U+21F8 Rightwards Arrow With Vertical Stroke */
#define AGSI_LR_ARROW_W_VSTROKE       "\xE2\x86\xF9" /* U+21F9 Left Right Arrow With Vertical Stroke */
#define AGSI_L_ARROW_W_DBL_VSTROKE    "\xE2\x86\xFA" /* U+21FA Leftwards Arrow With Double Vertical Stroke */
#define AGSI_R_ARROW_W_DBL_VSTROKE    "\xE2\x86\xFB" /* U+21FB Rightwards Arrow With Double Vertical Stroke */
#define AGSI_LR_ARROW_W_DBL_VSTROKE   "\xE2\x86\xFC" /* U+21FC Left Right Arrow With Double Vertical Stroke */
#define AGSI_L_OPEN_HEADED_ARROW      "\xE2\x86\xFD" /* U+21FD Leftwards Open-Headed Arrow */
#define AGSI_R_OPEN_HEADED_ARROW      "\xE2\x86\xFE" /* U+21FE Rightwards Open-Headed Arrow */
#define AGSI_LR_OPEN_HEADED_ARROW     "\xE2\x86\xFF" /* U+21FF Left Right Open-Headed Arrow */

/*
 * Mathematical Operators ("GT" = "GREATER_THAN", "LT" = "LESS_THAN",
 * "SM" = "SMALL", "APPROX" = "APPROXIMATELY", "HSTROKE" = "HORIZONTAL_STROKE").
 */
#define AGSI_FOR_ALL                                "\xE2\x88\x80" /* U+2200 */
#define AGSI_COMPLEMENT                             "\xE2\x88\x81" /* U+2201 */
#define AGSI_PARTIAL_DIFFERENTIAL                   "\xE2\x88\x82" /* U+2202 */
#define AGSI_THERE_EXISTS                           "\xE2\x88\x83" /* U+2203 */
#define AGSI_THERE_DOES_NOT_EXIST                   "\xE2\x88\x84" /* U+2204 */
#define AGSI_EMPTY_SET                              "\xE2\x88\x85" /* U+2205 */
#define AGSI_INCREMENT                              "\xE2\x88\x86" /* U+2206 */
#define AGSI_NABLA                                  "\xE2\x88\x87" /* U+2207 */
#define AGSI_ELEMENT_OF                             "\xE2\x88\x88" /* U+2208 */
#define AGSI_NOT_AN_ELEMENT_OF                      "\xE2\x88\x89" /* U+2209 */
#define AGSI_SM_ELEMENT_OF                          "\xE2\x88\x8A" /* U+220A Small Element Of */
#define AGSI_CONTAINS_AS_MEMBER                     "\xE2\x88\x8B" /* U+220B */
#define AGSI_DOES_NOT_CONTAIN_AS_MEMBER             "\xE2\x88\x8C" /* U+220C */
#define AGSI_SM_CONTAINS_AS_MEMBER                  "\xE2\x88\x8D" /* U+220D Small Contains As Member */
#define AGSI_END_OF_PROOF                           "\xE2\x88\x8E" /* U+220E */
#define AGSI_N_ARY_PRODUCT                          "\xE2\x88\x8F" /* U+220F */
#define AGSI_N_ARY_COPRODUCT                        "\xE2\x88\x90" /* U+2210 */
#define AGSI_N_ARY_SUMMATION                        "\xE2\x88\x91" /* U+2211 */
#define AGSI_MINUS_SIGN                             "\xE2\x88\x92" /* U+2212 */
#define AGSI_MINUS_OR_PLUS_SIGN                     "\xE2\x88\x93" /* U+2213 */
#define AGSI_DOT_PLUS                               "\xE2\x88\x94" /* U+2214 */
#define AGSI_DIVISION_SLASH                         "\xE2\x88\x95" /* U+2215 */
#define AGSI_SET_MINUS                              "\xE2\x88\x96" /* U+2216 */
#define AGSI_ASTERISK_OPERATOR                      "\xE2\x88\x97" /* U+2217 */
#define AGSI_RING_OPERATOR                          "\xE2\x88\x98" /* U+2218 */
#define AGSI_BULLET_OPERATOR                        "\xE2\x88\x99" /* U+2219 */
#define AGSI_SQUARE_ROOT                            "\xE2\x88\x9A" /* U+221A */
#define AGSI_CUBE_ROOT                              "\xE2\x88\x9B" /* U+221B */
#define AGSI_FOURTH_ROOT                            "\xE2\x88\x9C" /* U+221C */
#define AGSI_PROPORTIONAL_TO                        "\xE2\x88\x9D" /* U+221D */
#define AGSI_INFINITY                               "\xE2\x88\x9E" /* U+221E */
#define AGSI_RIGHT_ANGLE                            "\xE2\x88\x9F" /* U+221F */
#define AGSI_ANGLE                                  "\xE2\x88\xA0" /* U+2220 */
#define AGSI_MEASURED_ANGLE                         "\xE2\x88\xA1" /* U+2221 */
#define AGSI_SPHERICAL_ANGLE                        "\xE2\x88\xA2" /* U+2222 */
#define AGSI_DIVIDES                                "\xE2\x88\xA3" /* U+2223 */
#define AGSI_DOES_NOT_DIVIDE                        "\xE2\x88\xA4" /* U+2224 */
#define AGSI_PARALLEL_TO                            "\xE2\x88\xA5" /* U+2225 */
#define AGSI_NOT_PARALLEL_TO                        "\xE2\x88\xA6" /* U+2226 */
#define AGSI_LOGICAL_AND                            "\xE2\x88\xA7" /* U+2227 */
#define AGSI_LOGICAL_OR                             "\xE2\x88\xA8" /* U+2228 */
#define AGSI_INTERSECTION                           "\xE2\x88\xA9" /* U+2229 */
#define AGSI_UNION                                  "\xE2\x88\xAA" /* U+222A */
#define AGSI_INTEGRAL                               "\xE2\x88\xAB" /* U+222B */
#define AGSI_DOUBLE_INTEGRAL                        "\xE2\x88\xAC" /* U+222C */
#define AGSI_TRIPLE_INTEGRAL                        "\xE2\x88\xAD" /* U+222D */
#define AGSI_CONTOUR_INTEGRAL                       "\xE2\x88\xAE" /* U+222E */
#define AGSI_SURFACE_INTEGRAL                       "\xE2\x88\xAF" /* U+222F */
#define AGSI_VOLUME_INTEGRAL                        "\xE2\x88\xB0" /* U+2230 */
#define AGSI_CW_INTEGRAL                            "\xE2\x88\xB1" /* U+2231 Clockwise Integral */
#define AGSI_CW_CONTOUR_INTEGRAL                    "\xE2\x88\xB2" /* U+2232 Clockwise Contour Integral */
#define AGSI_CCW_CONTOUR_INTEGRAL                   "\xE2\x88\xB3" /* U+2233 Anticlockwise Contour Integral */
#define AGSI_THEREFORE                              "\xE2\x88\xB4" /* U+2234 */
#define AGSI_BECAUSE                                "\xE2\x88\xB5" /* U+2235 */
#define AGSI_RATIO                                  "\xE2\x88\xB6" /* U+2236 */
#define AGSI_PROPORTION                             "\xE2\x88\xB7" /* U+2237 */
#define AGSI_DOT_MINUS                              "\xE2\x88\xB8" /* U+2238 */
#define AGSI_EXCESS                                 "\xE2\x88\xB9" /* U+2239 */
#define AGSI_GEOMETRIC_PROPORTION                   "\xE2\x88\xBA" /* U+223A */
#define AGSI_HOMOTHETIC                             "\xE2\x88\xBB" /* U+223B */
#define AGSI_TILDE_OPERATOR                         "\xE2\x88\xBC" /* U+223C */
#define AGSI_REVERSED_TILDE                         "\xE2\x88\xBD" /* U+223D */
#define AGSI_INVERTED_LAZY_S                        "\xE2\x88\xBD" /* U+223D */
#define AGSI_SINE_WAVE                              "\xE2\x88\xBF" /* U+223F */
#define AGSI_WREATH_PRODUCT                         "\xE2\x89\x80" /* U+2240 */
#define AGSI_NOT_TILDE                              "\xE2\x89\x81" /* U+2241 */
#define AGSI_MINUS_TILDE                            "\xE2\x89\x82" /* U+2242 */
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
#define AGSI_LT_OR_EQUAL_TO                         "\xE2\x89\xA4" /* U+2264 Less Than Or Equal To */
#define AGSI_GT_OR_EQUAL_TO                         "\xE2\x89\xA5" /* U+2265 Greater Than Or Equal To */
#define AGSI_LT_OVER_EQUAL_TO                       "\xE2\x89\xA6" /* U+2266 Less Than Over Equal To */
#define AGSI_GT_OVER_EQUAL_TO                       "\xE2\x89\xA7" /* U+2267 Greater Than Over Equal To */
#define AGSI_LT_BUT_NOT_EQUAL_TO                    "\xE2\x89\xA8" /* U+2268 Less Than But Not Equal To */
#define AGSI_GT_BUT_NOT_EQUAL_TO                    "\xE2\x89\xA9" /* U+2269 Greater Than But Not Equal To */
#define AGSI_MUCH_LT                                "\xE2\x89\xAA" /* U+226A Much Less Than */
#define AGSI_MUCH_GT                                "\xE2\x89\xAB" /* U+226B Much Greater Than */
#define AGSI_BETWEEN                                "\xE2\x89\xAC" /* U+226C */
#define AGSI_NOT_EQUIVALENT_TO                      "\xE2\x89\xAD" /* U+226D */
#define AGSI_NOT_LT                                 "\xE2\x89\xAE" /* U+226E Not Less Than */
#define AGSI_NOT_GT                                 "\xE2\x89\xAF" /* U+226F Not Greater Than */
#define AGSI_NEITHER_LT_NOR_EQUAL_TO                "\xE2\x89\xB0" /* U+2270 Neither Less Than Nor Equal To */
#define AGSI_NEITHER_GT_NOR_EQUAL_TO                "\xE2\x89\xB1" /* U+2271 Neither Greater Than Nor Equal To */
#define AGSI_LT_OR_EQUIVALENT_TO                    "\xE2\x89\xB2" /* U+2272 Less Than Or Equivalent To */
#define AGSI_GT_OR_EQUIVALENT_TO                    "\xE2\x89\xB3" /* U+2273 Greater Than Or Equivalent To */
#define AGSI_NEITHER_LT_NOR_EQUIVALENT_TO           "\xE2\x89\xB4" /* U+2274 Neither Less Than Nor Equivalent To */
#define AGSI_NEITHER_GT_NOR_EQUIVALENT_TO           "\xE2\x89\xB5" /* U+2275 Neither Greater Than Nor Equivalent To */
#define AGSI_LT_OR_GT                               "\xE2\x89\xB6" /* U+2276 Less Than Or Greater Than */
#define AGSI_GT_OR_LT                               "\xE2\x89\xB7" /* U+2277 Greater Than Or Less Than */
#define AGSI_NEITHER_LT_NOR_GT                      "\xE2\x89\xB8" /* U+2278 Neither Less Than Nor Greater Than */
#define AGSI_NEITHER_GT_NOR_LT                      "\xE2\x89\xB9" /* U+2279 Neither Greater Than Nor Less Than */
#define AGSI_PRECEDES                               "\xE2\x89\xBA" /* U+227A */
#define AGSI_SUCCEEDS                               "\xE2\x89\xBB" /* U+227B */
#define AGSI_PRECEDES_OR_EQUAL_TO                   "\xE2\x89\xBC" /* U+227C */
#define AGSI_SUCCEEDS_OR_EQUAL_TO                   "\xE2\x89\xBD" /* U+227D */
#define AGSI_PRECEDES_OR_EQUIVALENT_TO              "\xE2\x89\xBE" /* U+227E */
#define AGSI_SUCCEEDS_OR_EQUIVALENT_TO              "\xE2\x89\xBF" /* U+227F */
#define AGSI_DOES_NOT_PRECEDE                       "\xE2\x8A\x80" /* U+2280 */
#define AGSI_DOES_NOT_SUCCEED                       "\xE2\x8A\x81" /* U+2281 */
#define AGSI_SUBSET_OF                              "\xE2\x8A\x82" /* U+2282 */
#define AGSI_SUPERSET_OF                            "\xE2\x8A\x83" /* U+2283 */
#define AGSI_NOT_A_SUBSET_OF                        "\xE2\x8A\x84" /* U+2284 */
#define AGSI_NOT_A_SUPERSET_OF                      "\xE2\x8A\x85" /* U+2285 */
#define AGSI_SUBSET_OF_OR_EQUAL_TO                  "\xE2\x8A\x86" /* U+2286 */
#define AGSI_SUPERSET_OF_OR_EQUAL_TO                "\xE2\x8A\x87" /* U+2287 */
#define AGSI_NEITHER_A_SUBSET_OF_NOR_EQUAL_TO       "\xE2\x8A\x88" /* U+2288 */
#define AGSI_NEITHER_A_SUPERSET_OF_NOR_EQUAL_TO     "\xE2\x8A\x89" /* U+2289 */
#define AGSI_SUBSET_OF_W_NOT_EQUAL_TO               "\xE2\x8A\x8A" /* U+228A Subset Of With Not Equal To */
#define AGSI_SUPERSET_OF_W_NOT_EQUAL_TO             "\xE2\x8A\x8B" /* U+228B Superset Of With Not Equal To */
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
#define AGSI_R_TACK                                 "\xE2\x8A\xA2" /* U+22A2 Right Tack */
#define AGSI_L_TACK                                 "\xE2\x8A\xA3" /* U+22A3 Left Tack */
#define AGSI_DN_TACK                                "\xE2\x8A\xA4" /* U+22A4 Down Tack */
#define AGSI_UP_TACK                                "\xE2\x8A\xA5" /* U+22A5 */
#define AGSI_PERPENDICULAR AGSI_UP_TACK
#define AGSI_ASSERTION                              "\xE2\x8A\xA6" /* U+22A6 */
#define AGSI_MODELS                                 "\xE2\x8A\xA7" /* U+22A7 */
#define AGSI_TRUE_SYMBOL                            "\xE2\x8A\xA8" /* U+22A8 True */
#define AGSI_FORCES_SYMBOL                          "\xE2\x8A\xA9" /* U+22A9 Forces */
#define AGSI_TRIPLE_VBAR_R_TURNSTILE                "\xE2\x8A\xAA" /* U+22AA Triple Vertical Bar Right Turnstile */
#define AGSI_DOUBLE_VBAR_DOUBLE_R_TURNSTILE         "\xE2\x8A\xAB" /* U+22AB Double Vertical Bar Double Right Turnstile */
#define AGSI_DOES_NOT_PROVE                         "\xE2\x8A\xAC" /* U+22AC */
#define AGSI_NOT_TRUE_SYMBOL                        "\xE2\x8A\xAD" /* U+22AD Not True */
#define AGSI_DOES_NOT_FORCE                         "\xE2\x8A\xAE" /* U+22AE */
#define AGSI_NEGATED_DOUBLE_VBAR_DOUBLE_R_TURNSTILE "\xE2\x8A\xAF" /* U+22AF Negated Double Vertical Bar Double Right Turnstile */
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
#define AGSI_RIGHT_ANGLE_W_ARC                      "\xE2\x8A\xBE" /* U+22BE Right Triangle With Arc */
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
#define AGSI_L_NORMAL_FACTOR_SEMIDIRECT_PRODUCT     "\xE2\x8B\x89" /* U+22C9 Left Normal Factor Semidirect Product */
#define AGSI_R_NORMAL_FACTOR_SEMIDIRECT_PRODUCT     "\xE2\x8B\x8A" /* U+22CA Right Normal Factor Semidirect Product */
#define AGSI_L_SEMIDIRECT_PRODUCT                   "\xE2\x8B\x8B" /* U+22CB Left Semidirect Product */
#define AGSI_R_SEMIDIRECT_PRODUCT                   "\xE2\x8B\x8C" /* U+22CC Right Semidirect Product */
#define AGSI_REVERSED_TILDE_EQUALS                  "\xE2\x8B\x8D" /* U+22CD */
#define AGSI_CURLY_LOGICAL_OR                       "\xE2\x8B\x8E" /* U+22CE */
#define AGSI_CURLY_LOGICAL_AND                      "\xE2\x8B\x8F" /* U+22CF */
#define AGSI_DOUBLE_SUBSET                          "\xE2\x8B\x90" /* U+22D0 */
#define AGSI_DOUBLE_SUPERSET                        "\xE2\x8B\x91" /* U+22D1 */
#define AGSI_DOUBLE_INTERSECTION                    "\xE2\x8B\x92" /* U+22D2 */
#define AGSI_DOUBLE_UNION                           "\xE2\x8B\x93" /* U+22D3 */
#define AGSI_PITCHFORK                              "\xE2\x8B\x94" /* U+22D4 */
#define AGSI_EQUAL_AND_PARALLEL_TO                  "\xE2\x8B\x95" /* U+22D5 */
#define AGSI_LT_W_DOT                               "\xE2\x8B\x96" /* U+22D6 Less Than With Dot */
#define AGSI_GT_W_DOT                               "\xE2\x8B\x97" /* U+22D7 Greater Than With Dot */
#define AGSI_VERY_MUCH_LT                           "\xE2\x8B\x98" /* U+22D8 Very Much Less Than */
#define AGSI_VERY_MUCH_GT                           "\xE2\x8B\x99" /* U+22D9 Very Much Greater Than */
#define AGSI_LT_EQUAL_TO_OR_GT                      "\xE2\x8B\x9A" /* U+22DA Less Than Equal To Or Greater Than */
#define AGSI_GT_EQUAL_TO_OR_LT                      "\xE2\x8B\x9B" /* U+22DB Greater Than Equal To Or Less Than */
#define AGSI_EQUAL_TO_OR_LT                         "\xE2\x8B\x9C" /* U+22DC Equal To Or Less Than */
#define AGSI_EQUAL_TO_OR_GT                         "\xE2\x8B\x9D" /* U+22DD Equal To Or Greater Than */
#define AGSI_EQUAL_TO_OR_PRECEDES                   "\xE2\x8B\x9E" /* U+22DE */
#define AGSI_EQUAL_TO_OR_SUCCEEDS                   "\xE2\x8B\x9F" /* U+22DF */
#define AGSI_DOES_NOT_PRECEDE_OR_EQUAL              "\xE2\x8B\xA0" /* U+22E0 */
#define AGSI_DOES_NOT_SUCCEED_OR_EQUAL              "\xE2\x8B\xA1" /* U+22E1 */
#define AGSI_NOT_SQUARE_IMAGE_OF_OR_EQUAL_TO        "\xE2\x8B\xA2" /* U+22E2 */
#define AGSI_NOT_SQUARE_ORIGINAL_OF_OR_EQUAL_TO     "\xE2\x8B\xA3" /* U+22E3 */
#define AGSI_SQUARE_IMAGE_OF_OR_NOT_EQUAL_TO        "\xE2\x8B\xA4" /* U+22E4 */
#define AGSI_SQUARE_ORIGINAL_OF_OR_NOT_EQUAL_TO     "\xE2\x8B\xA5" /* U+22E5 */
#define AGSI_LT_BUT_NOT_EQUIVALENT_TO               "\xE2\x8B\xA6" /* U+22E6 Less Than But Not Equivalent To */
#define AGSI_GT_BUT_NOT_EQUIVALENT_TO               "\xE2\x8B\xA7" /* U+22E7 Greater Than But Not Equivalent To */
#define AGSI_PRECEDES_BUT_NOT_EQUIVALENT_TO         "\xE2\x8B\xA8" /* U+22E8 */
#define AGSI_SUCCEEDS_BUT_NOT_EQUIVALENT_TO         "\xE2\x8B\xA9" /* U+22E9 */
#define AGSI_NOT_NORMAL_SUBGROUP_OF                 "\xE2\x8B\xAA" /* U+22EA */
#define AGSI_DOES_NOT_CONTAIN_AS_NORMAL_SUBGROUP    "\xE2\x8B\xAB" /* U+22EB */
#define AGSI_NOT_NORMAL_SUBGROUP_OF_OR_EQUAL_TO     "\xE2\x8B\xAC" /* U+22EC */
#define AGSI_DOES_NOT_CONTAIN_AS_NORMAL_SUBGROUP_OR_EQUAL "\xE2\x8B\xAD" /* U+22ED */
#define AGSI_VERTICAL_ELLIPSIS                      "\xE2\x8B\xAE" /* U+22EE */
#define AGSI_MIDLINE_HORIZONTAL_ELLIPSIS            "\xE2\x8B\xAF" /* U+22EF */
#define AGSI_UP_R_DIAGONAL_ELLIPSIS                 "\xE2\x8B\xB0" /* U+22F0 Up Right Diagonal Ellipsis */
#define AGSI_DN_R_DIAGONAL_ELLIPSIS                 "\xE2\x8B\xB1" /* U+22F1 Down Right Diagonal Ellipsis */
#define AGSI_ELEMENT_OF_W_LONG_HSTROKE              "\xE2\x8B\xB2" /* U+22F2 Element Of With Long Horizontal Stroke */
#define AGSI_ELEMENT_OF_W_VBAR_AT_END_OF_HSTROKE    "\xE2\x8B\xB3" /* U+22F3 Element Of With Vertical Bar At End Of Horizontal Stroke */
#define AGSI_SM_ELEMENT_OF_W_VBAR_AT_END_OF_HSTROKE "\xE2\x8B\xB4" /* U+22F4 Small Element Of With Vertical Bar At End Of Horizontal Stroke */
#define AGSI_ELEMENT_OF_W_DOT_ABOVE                 "\xE2\x8B\xB5" /* U+22F5 Element Of With Dot Above */
#define AGSI_ELEMENT_OF_W_OVERBAR                   "\xE2\x8B\xB6" /* U+22F6 Element Of With Overbar */
#define AGSI_SM_ELEMENT_OF_W_OVERBAR                "\xE2\x8B\xB7" /* U+22F7 Small Element Of With Overbar */
#define AGSI_ELEMENT_OF_W_UNDERBAR                  "\xE2\x8B\xB8" /* U+22F8 Element Of With Underbar */
#define AGSI_ELEMENT_OF_W_TWO_HSTROKES              "\xE2\x8B\xB9" /* U+22F9 Element Of With Two Horizontal Strokes */
#define AGSI_CONTAINS_W_LONG_HSTROKE                "\xE2\x8B\xBA" /* U+22FA Contains With Long Horizontal Stroke */
#define AGSI_CONTAINS_W_VBAR_AT_END_OF_HSTROKE      "\xE2\x8B\xBB" /* U+22FB Contains With Vertical Bar At End Of Horizontal Stroke */
#define AGSI_SM_CONTAINS_W_VBAR_AT_END_OF_HSTROKE   "\xE2\x8B\xBC" /* U+22FC Small Contains With Vertical Bar At End Of Horizontal Stroke */
#define AGSI_CONTAINS_W_OVERBAR                     "\xE2\x8B\xBD" /* U+22FD Contains With Overbar */
#define AGSI_SM_CONTAINS_W_OVERBAR                  "\xE2\x8B\xBE" /* U+22FE Small Contains With Overbar */
#define AGSI_Z_NOTATION_BAG_MEMBERSHIP              "\xE2\x8B\xBF" /* U+22FF */
/*
 * Miscellaneous Technical ("PAREN" = "PARENTHESIS", "SQ" = "SQUARE").
 */
#define AGSI_DIAMETER_SIGN                  "\xE2\x8C\x80" /* U+2300 */
#define AGSI_ELECTRIC_ARROW                 "\xE2\x8C\x81" /* U+2301 */
#define AGSI_HOUSE                          "\xE2\x8C\x82" /* U+2302 */
#define AGSI_UP_ARROWHEAD                   "\xE2\x8C\x83" /* U+2303 */
#define AGSI_DN_ARROWHEAD                   "\xE2\x8C\x84" /* U+2304 Down Arrowhead */
#define AGSI_PROJECTIVE                     "\xE2\x8C\x85" /* U+2305 */
#define AGSI_PERSPECTIVE                    "\xE2\x8C\x86" /* U+2306 */
#define AGSI_WAVY_LINE                      "\xE2\x8C\x87" /* U+2307 */
#define AGSI_L_CEILING                      "\xE2\x8C\x88" /* U+2308 Left Ceiling */
#define AGSI_R_CEILING                      "\xE2\x8C\x89" /* U+2309 Right Ceiling */
#define AGSI_L_FLOOR                        "\xE2\x8C\x8A" /* U+230A Left Floor */
#define AGSI_R_FLOOR                        "\xE2\x8C\x8B" /* U+230B Right Floor */
#define AGSI_BOTTOM_R_CROP                  "\xE2\x8C\x8C" /* U+230C Bottom Right Crop */
#define AGSI_BOTTOM_L_CROP                  "\xE2\x8C\x8D" /* U+230D Bottom Left Crop */
#define AGSI_TOP_R_CROP                     "\xE2\x8C\x8E" /* U+230E Top Right Crop */
#define AGSI_TOP_L_CROP                     "\xE2\x8C\x8F" /* U+230F Top Left Crop */
#define AGSI_REVERSED_NOT_SIGN              "\xE2\x8C\x90" /* U+2310 */
#define AGSI_SQ_LOZENGE                     "\xE2\x8C\x91" /* U+2311 Square Lozenge */
#define AGSI_PLACE_OF_INTEREST_SIGN         "\xE2\x8C\x98" /* U+2318 */
#define AGSI_TURNED_NOT_SIGN                "\xE2\x8C\x99" /* U+2319 */
#define AGSI_TOP_L_CORNER                   "\xE2\x8C\x9C" /* U+231C Top Left Corner */
#define AGSI_TOP_R_CORNER                   "\xE2\x8C\x9D" /* U+231D Top Right Corner */
#define AGSI_BOTTOM_L_CORNER                "\xE2\x8C\x9E" /* U+231E Bottom Left Corner */
#define AGSI_BOTTOM_R_CORNER                "\xE2\x8C\x9F" /* U+231F Bottom Right Corner */
#define AGSI_TOP_HALF_INTEGRAL              "\xE2\x8C\xA0" /* U+2320 */
#define AGSI_BOTTOM_HALF_INTEGRAL           "\xE2\x8C\xA1" /* U+2321 */
#define AGSI_UP_ARROWHEAD_BETWEEN_TWO_HBARS "\xE2\x8C\xA4" /* U+2324 Up Arrowhead Between Two Horizontal Bars */
#define AGSI_OPTION_KEY                     "\xE2\x8C\xA5" /* U+2325 */
#define AGSI_ERASE_TO_THE_R                 "\xE2\x8C\xA6" /* U+2326 Erase To The Right */
#define AGSI_X_IN_A_RECTANGLE_BOX           "\xE2\x8C\xA7" /* U+2327 */
#define AGSI_KEYBOARD                       "\xE2\x8C\xA8" /* U+2328 */
#define AGSI_ERASE_TO_THE_L                 "\xE2\x8C\xAB" /* U+232B Erase To The Left */
#define AGSI_BENZENE_RING                   "\xE2\x8C\xAC" /* U+232C */
#define AGSI_APL_FUNCTIONAL_SYMBOL_IOTA     "\xE2\x8D\xB3" /* U+2373 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_RHO      "\xE2\x8D\xB4" /* U+2374 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_OMEGA    "\xE2\x8D\xB5" /* U+2375 */
#define AGSI_APL_FUNCTIONAL_SYMBOL_ALPHA    "\xE2\x8D\xBA" /* U+237A */
#define AGSI_SHOULDERED_OPEN_BOX            "\xE2\x8D\xBD" /* U+237D */
#define AGSI_ALTERNATIVE_KEY_SYMBOL         "\xE2\x8E\x87" /* U+2387 */
#define AGSI_SOFTWARE_FUNCTION_SYMBOL       "\xE2\x8E\x94" /* U+2394 */
#define AGSI_L_PAREN_UPPER_HOOK             "\xE2\x8E\x9B" /* U+239B Left Parenthesis Upper Hook */
#define AGSI_L_PAREN_EXTENSION              "\xE2\x8E\x9C" /* U+239C Left Parenthesis Extension */
#define AGSI_L_PAREN_LOWER_HOOK             "\xE2\x8E\x9D" /* U+239D Left Parenthesis Lower Hook */
#define AGSI_R_PAREN_UPPER_HOOK             "\xE2\x8E\x9E" /* U+239E Right Parenthesis Upper Hook */
#define AGSI_R_PAREN_EXTENSION              "\xE2\x8E\x9F" /* U+239F Right Parenthesis Extension */
#define AGSI_R_PAREN_LOWER_HOOK             "\xE2\x8E\xA0" /* U+23A0 Right Parenthesis Lower Hook */
#define AGSI_L_SQ_BRACKET_UPPER_CORNER      "\xE2\x8E\xA1" /* U+23A1 Left Square Bracket Upper Corner */
#define AGSI_L_SQ_BRACKET_EXTENSION         "\xE2\x8E\xA2" /* U+23A2 Left Square Bracket Extension */
#define AGSI_L_SQ_BRACKET_LOWER_CORNER      "\xE2\x8E\xA3" /* U+23A3 Left Square Bracket Lower Corner */
#define AGSI_R_SQ_BRACKET_UPPER_CORNER      "\xE2\x8E\xA4" /* U+23A4 Right Square Bracket Upper Corner */
#define AGSI_R_SQ_BRACKET_EXTENSION         "\xE2\x8E\xA5" /* U+23A5 Right Square Bracket Extension */
#define AGSI_R_SQ_BRACKET_LOWER_CORNER      "\xE2\x8E\xA6" /* U+23A6 Right Square Bracket Lower Corner */
#define AGSI_L_CURLY_BRACKET_UPPER_HOOK     "\xE2\x8E\xA7" /* U+23A7 Left Curly Bracket Upper Hook */
#define AGSI_L_CURLY_BRACKET_MIDDLE_PIECE   "\xE2\x8E\xA8" /* U+23A8 Left Curly Bracket Middle Piece */
#define AGSI_L_CURLY_BRACKET_LOWER_HOOK     "\xE2\x8E\xA9" /* U+23A9 Left Curly Bracket Lower Hook */
#define AGSI_CURLY_BRACKET_EXTENSION        "\xE2\x8E\xAA" /* U+23AA */
#define AGSI_R_CURLY_BRACKET_UPPER_HOOK     "\xE2\x8E\xAB" /* U+23AB Right Curly Bracket Upper Hook */
#define AGSI_R_CURLY_BRACKET_MIDDLE_PIECE   "\xE2\x8E\xAC" /* U+23AC Right Curly Bracket Middle Piece */
#define AGSI_R_CURLY_BRACKET_LOWER_HOOK     "\xE2\x8E\xAD" /* U+23AD Right Curly Bracket Lower Hook */
#define AGSI_INTEGRAL_EXTENSION             "\xE2\x8E\xAE" /* U+23AE */
#define AGSI_RETURN_SYMBOL                  "\xE2\x8F\x8E" /* U+23CE */
#define AGSI_EJECT_SYMBOL                   "\xE2\x8F\x8F" /* U+23CF */
#define AGSI_BENZENE_RING_W_CIRCLE          "\xE2\x8F\xA3" /* U+23E3 Benzene Ring With Circle */
#define AGSI_FLATNESS                       "\xE2\x8F\xA5" /* U+23E5 */
#define AGSI_DECIMAL_EXPONENT_SYMBOL        "\xE2\x8F\xA8" /* U+23E8 */
#define AGSI_STOPWATCH                      "\xE2\x8F\xB1" /* U+23F1 */
/*
 * Control Pictures.
 */
#define AGSI_BLANK_SYMBOL     "\xE2\x90\xA2"       /* U+2422 */
#define AGSI_OPEN_BOX         "\xE2\x90\xA3"       /* U+2423 */
/*
 * Enclosed Alphanumerics.
 */
#define AGSI_CIRCLED_DIGIT_1  "\xE2\x91\xA0"       /* U+2460 */
#define AGSI_CIRCLED_DIGIT_2  "\xE2\x91\xA1"       /* U+2461 */
#define AGSI_CIRCLED_DIGIT_3  "\xE2\x91\xA2"       /* U+2462 */
#define AGSI_CIRCLED_DIGIT_4  "\xE2\x91\xA3"       /* U+2463 */
#define AGSI_CIRCLED_DIGIT_5  "\xE2\x91\xA4"       /* U+2464 */
#define AGSI_CIRCLED_DIGIT_6  "\xE2\x91\xA5"       /* U+2465 */
#define AGSI_CIRCLED_DIGIT_7  "\xE2\x91\xA6"       /* U+2466 */
#define AGSI_CIRCLED_DIGIT_8  "\xE2\x91\xA7"       /* U+2467 */
#define AGSI_CIRCLED_DIGIT_9  "\xE2\x91\xA8"       /* U+2468 */
#define AGSI_CIRCLED_DIGIT_10 "\xE2\x91\xA9"       /* U+2469 */
/*
 * Geometric Shapes.
 */
#define AGSI_DOTTED_CIRCLE    "\xE2\x97\x8C"       /* U+25CC */
/*
 * Miscellaneous Symbols.
 */
#define AGSI_WHEEL_OF_DHARMA   "\xE2\x98\xB8"       /* U+2638 */
#define AGSI_GEAR              "\xE2\x9A\x99"       /* U+2699 */
#define AGSI_WHEELCHAIR_SYMBOL "\xE2\x99\xBF"       /* U+267F */
/*
 * Dingbats.
 */
#define AGSI_VICTORY_HAND            "\xE2\x9C\x8C" /* U+270C */
#define AGSI_WRITING_HAND            "\xE2\x9C\x8D" /* U+270D */
#define AGSI_LOWER_R_PENCIL          "\xE2\x9C\x8E" /* U+270E Lower Right Pencil */
#define AGSI_PENCIL                  "\xE2\x9C\x8F" /* U+270F */
#define AGSI_UPPER_R_PENCIL          "\xE2\x9C\x90" /* U+2710 Upper Right Pencil */
#define AGSI_WHITE_NIB               "\xE2\x9C\x91" /* U+2711 */
#define AGSI_BLACK_NIB               "\xE2\x9C\x92" /* U+2712 */
#define AGSI_CHECK_MARK              "\xE2\x9C\x93" /* U+2713 */
#define AGSI_HEAVY_CHECK_MARK        "\xE2\x9C\x94" /* U+2714 */
#define AGSI_MULTIPLICATION_X        "\xE2\x9C\x95" /* U+2715 */
#define AGSI_HEAVY_MULTIPLICATION_X  "\xE2\x9C\x96" /* U+2716 */
#define AGSI_BALLOT_X                "\xE2\x9C\x97" /* U+2717 */
#define AGSI_HEAVY_BALLOT_X          "\xE2\x9C\x98" /* U+2718 */
#define AGSI_BLK_4_POINTED_STAR      "\xE2\x9C\xA6" /* U+2726 Black Four Pointed Star */
#define AGSI_WHT_4_POINTED_STAR      "\xE2\x9C\xA7" /* U+2727 White Four Pointed Star */
#define AGSI_CCW_CLOSED_CIRCLE_ARROW "\xE2\xA5\x80" /* U+2940 Anticlockwise Closed Circle Arrow */
#define AGSI_CW_CLOSED_CIRCLE_ARROW  "\xE2\xA5\x81" /* U+2941 Clockwise Closed Circle Arrow */
/*
 * Miscellaneous Symbols and Pictographs.
 */
#define AGSI_NEW_MOON             "\xF0\x9F\x8C\x91" /* U+1F311 */
#define AGSI_WAXING_CRESCENT_MOON "\xF0\x9F\x8C\x92" /* U+1F312 */
#define AGSI_FIRST_QUARTER_MOON   "\xF0\x9F\x8C\x93" /* U+1F313 */
#define AGSI_WAXING_GIBBOUS_MOON  "\xF0\x9F\x8C\x94" /* U+1F314 */
#define AGSI_FULL_MOON            "\xF0\x9F\x8C\x95" /* U+1F315 */
#define AGSI_WANING_GIBBOUS_MOON  "\xF0\x9F\x8C\x96" /* U+1F316 */
#define AGSI_LAST_QUARTER_MOON    "\xF0\x9F\x8C\x97" /* U+1F317 */
#define AGSI_WANING_CRESCENT_MOON "\xF0\x9F\x8C\x98" /* U+1F318 */
#define AGSI_ARTISTS_PALETTE      "\xF0\x9F\x8E\xA8" /* U+1F3A8 */
#define AGSI_ANT                  "\xF0\x9F\x90\x9C" /* U+1F41C */
#define AGSI_MOUSE_FACE           "\xF0\x9F\x90\xAD" /* U+1F42D */
#define AGSI_COW_FACE             "\xF0\x9F\x90\xAE" /* U+1F42E */
#define AGSI_CAT_FACE             "\xF0\x9F\x90\xB1" /* U+1F431 */
#define AGSI_MONKEY_FACE          "\xF0\x9F\x90\xB5" /* U+1F435 */
#define AGSI_TEE_SHIRT            "\xF0\x9F\x91\x95" /* U+1F455 */
#define AGSI_JEANS                "\xF0\x9F\x91\x96" /* U+1F456 */
#define AGSI_PILE_OF_POO          "\xF0\x9F\x92\xA9" /* U+1F4A9 */
#define AGSI_FLOPPY_DISK          "\xF0\x9F\x92\xBE" /* U+1F4BE */
#define AGSI_DVD                  "\xF0\x9F\x93\x80" /* U+1F4C0 */
#define AGSI_SPKR_W_3_SOUND_WAVES "\xF0\x9F\x94\x8A" /* U+1F50A Speaker With Three Sound Waves */
#define AGSI_JOYSTICK             "\xF0\x9F\x95\xB9" /* U+1F579 */
#define AGSI_LOWER_L_PENCIL       "\xF0\x9F\x96\x89" /* U+1F589 Lower Left Pencil */
#define AGSI_TWO_BUTTON_MOUSE     "\xF0\x9F\x96\xB0" /* U+1F5B0 */

/*
 * Emoticons ("SML" = "SMILING", "OM" = "OPEN_MOUTH").
 */
#define AGSI_GRINNING_FACE                    "\xF0\x9F\x98\x80" /* U+1F600 */
#define AGSI_GRINNING_FACE_W_SML_EYES         "\xF0\x9F\x98\x81" /* U+1F601 Grinning Face With Smiling Eyes */
#define AGSI_FACE_W_TEARS_OF_JOY              "\xF0\x9F\x98\x82" /* U+1F602 Face With Tears Of Joy */
#define AGSI_SML_FACE_W_OM                    "\xF0\x9F\x98\x83" /* U+1F603 Smiling Face With Open Mouth */
#define AGSI_SML_FACE_W_OM_AND_SML_EYES       "\xF0\x9F\x98\x84" /* U+1F604 Smiling Face With Open Mouth And Smiling Eyes */
#define AGSI_SML_FACE_W_OM_AND_COLD_SWEAT     "\xF0\x9F\x98\x85" /* U+1F605 Smiling Face With Open Mouth And Cold Sweat */
#define AGSI_SML_FACE_W_OM_AND_CLOSED_EYES    "\xF0\x9F\x98\x86" /* U+1F606 Smiling Face With Open Mouth And Tightly Closed Eyes */
#define AGSI_SML_FACE_W_HALO                  "\xF0\x9F\x98\x87" /* U+1F607 Smiling Face With Halo */
#define AGSI_SML_FACE_W_HORNS                 "\xF0\x9F\x98\x88" /* U+1F608 Smiling Face With Horns */
#define AGSI_WINKING_FACE                     "\xF0\x9F\x98\x89" /* U+1F609 */
#define AGSI_SML_FACE_W_SML_EYES              "\xF0\x9F\x98\x8A" /* U+1F60A Smiling Face With Smiling Eyes */
#define AGSI_FACE_SAVORING_DELICIOUS_FOOD     "\xF0\x9F\x98\x8B" /* U+1F60B */
#define AGSI_RELIEVED_FACE                    "\xF0\x9F\x98\x8C" /* U+1F60C */
#define AGSI_SML_FACE_W_HEART_SHAPED_EYES     "\xF0\x9F\x98\x8D" /* U+1F60D Smiling Face With Heart-Shaped Eyes */
#define AGSI_SML_FACE_W_SUNGLASSES            "\xF0\x9F\x98\x8E" /* U+1F60E Smiling Face With Sunglasses */
#define AGSI_SMIRKING_FACE                    "\xF0\x9F\x98\x8F" /* U+1F60F */
#define AGSI_NEUTRAL_FACE                     "\xF0\x9F\x98\x90" /* U+1F610 */
#define AGSI_EXPRESSIONLESS_FACE              "\xF0\x9F\x98\x91" /* U+1F611 */
#define AGSI_UNAMUSED_FACE                    "\xF0\x9F\x98\x92" /* U+1F612 */
#define AGSI_FACE_W_COLD_SWEAT                "\xF0\x9F\x98\x93" /* U+1F613 Face With Cold Sweat */
#define AGSI_PENSIVE_FACE                     "\xF0\x9F\x98\x94" /* U+1F614 */
#define AGSI_CONFUSED_FACE                    "\xF0\x9F\x98\x95" /* U+1F615 */
#define AGSI_CONFOUNDED_FACE                  "\xF0\x9F\x98\x96" /* U+1F616 */
#define AGSI_KISSING_FACE                     "\xF0\x9F\x98\x97" /* U+1F617 */
#define AGSI_FACE_THROWING_A_KISS             "\xF0\x9F\x98\x98" /* U+1F618 */
#define AGSI_KISSING_FACE_W_SML_EYES          "\xF0\x9F\x98\x99" /* U+1F619 Kissing Face With Smiling Eyes */
#define AGSI_KISSING_FACE_W_CLOSED_EYES       "\xF0\x9F\x98\x9A" /* U+1F61A Kissing Face With Closed Eyes */
#define AGSI_FACE_W_TONGUE                    "\xF0\x9F\x98\x9B" /* U+1F61B Face With Stuck Out Tongue */
#define AGSI_FACE_W_TONGUE_AND_WINKING_EYE    "\xF0\x9F\x98\x9C" /* U+1F61C Face With Stuck Out Tongue And Winking Eye */
#define AGSI_FACE_W_TONGUE_AND_CLOSED_EYES    "\xF0\x9F\x98\x9D" /* U+1F61D Face With Stuck Out Tongue And Tightly Closed Eyes */
#define AGSI_DISAPPOINTED_FACE                "\xF0\x9F\x98\x9E" /* U+1F61E */
#define AGSI_WORRIED_FACE                     "\xF0\x9F\x98\x9F" /* U+1F61F */
#define AGSI_ANGRY_FACE                       "\xF0\x9F\x98\xA0" /* U+1F620 */
#define AGSI_POUTING_FACE                     "\xF0\x9F\x98\xA1" /* U+1F621 */
#define AGSI_CRYING_FACE                      "\xF0\x9F\x98\xA2" /* U+1F622 */
#define AGSI_PERSEVERING_FACE                 "\xF0\x9F\x98\xA3" /* U+1F623 */
#define AGSI_FACE_W_LOOK_OF_TRIUMPH           "\xF0\x9F\x98\xA4" /* U+1F624 */
#define AGSI_DISAPPOINTED_BUT_RELIEVED_FACE   "\xF0\x9F\x98\xA5" /* U+1F625 */
#define AGSI_FROWNING_FACE_W_OM               "\xF0\x9F\x98\xA6" /* U+1F626 Frowning Face With Open Mouth */
#define AGSI_ANGUISHED_FACE                   "\xF0\x9F\x98\xA7" /* U+1F627 */
#define AGSI_FEARFUL_FACE                     "\xF0\x9F\x98\xA8" /* U+1F628 */
#define AGSI_WEARY_FACE                       "\xF0\x9F\x98\xA9" /* U+1F629 */
#define AGSI_SLEEPY_FACE                      "\xF0\x9F\x98\xAA" /* U+1F62A */
#define AGSI_TIRED_FACE                       "\xF0\x9F\x98\xAB" /* U+1F62B */
#define AGSI_GRIMACING_FACE                   "\xF0\x9F\x98\xAC" /* U+1F62C */
#define AGSI_LOUDLY_CRYING_FACE               "\xF0\x9F\x98\xAD" /* U+1F62D */
#define AGSI_FACE_W_OM                        "\xF0\x9F\x98\xAE" /* U+1F62E Face With Open Mouth */
#define AGSI_HUSHED_FACE                      "\xF0\x9F\x98\xAF" /* U+1F62F */
#define AGSI_FACE_W_OM_AND_COLD_SWEAT         "\xF0\x9F\x98\xB0" /* U+1F630 Face With Open Mouth And Cold Sweat */
#define AGSI_FACE_SCREAMING_IN_FEAR           "\xF0\x9F\x98\xB1" /* U+1F631 */
#define AGSI_ASTONISHED_FACE                  "\xF0\x9F\x98\xB2" /* U+1F632 */
#define AGSI_FLUSHED_FACE                     "\xF0\x9F\x98\xB3" /* U+1F633 */
#define AGSI_SLEEPING_FACE                    "\xF0\x9F\x98\xB4" /* U+1F634 */
#define AGSI_DIZZY_FACE                       "\xF0\x9F\x98\xB5" /* U+1F635 */
#define AGSI_FACE_WO_MOUTH                    "\xF0\x9F\x98\xB6" /* U+1F636 */
#define AGSI_FACE_MEDICAL_MASK                "\xF0\x9F\x98\xB7" /* U+1F637 */
#define AGSI_GRINNING_CAT_FACE_W_SML_EYES     "\xF0\x9F\x98\xB8" /* U+1F638 */
#define AGSI_CAT_FACE_W_TEARS_OF_JOY          "\xF0\x9F\x98\xB9" /* U+1F639 */
#define AGSI_SML_CAT_FACE_W_OPEN_MOUTH        "\xF0\x9F\x98\xBA" /* U+1F63A */
#define AGSI_SML_CAT_FACE_W_HEART_SHAPED_EYES "\xF0\x9F\x98\xBB" /* U+1F63B */
#define AGSI_CAT_FACE_W_WRY_SMILE             "\xF0\x9F\x98\xBC" /* U+1F63C */
#define AGSI_KISSING_CAT_FACE_W_CLOSED_EYES   "\xF0\x9F\x98\xBD" /* U+1F63D */
#define AGSI_POUTING_CAT_FACE                 "\xF0\x9F\x98\xBE" /* U+1F63E */
#define AGSI_CRYING_CAT_FACE                  "\xF0\x9F\x98\xBF" /* U+1F63F */
#define AGSI_WEARY_CAT_FACE                   "\xF0\x9F\x99\x80" /* U+1F640 */
#define AGSI_UPSIDE_DOWN_FACE                 "\xF0\x9F\x99\x83" /* U+1F643 */
#define AGSI_CONSTRUCTION_SIGN                "\xF0\x9F\x9A\xA7" /* U+1F6A7 */
/*
 * Ideograms (Agar Ideograms; Algue Private Use Area).
 */
#define AGSI_BLACK_AGAR           "\xEE\x80\x80"  /* U+E000 Logo Filled */
#define AGSI_WHITE_AGAR           "\xEE\x80\x81"  /* U+E001 Logo Outline */
#define AGSI_MENUBOOL_TRUE        "\xEE\x80\x82"  /* U+E002 Menu Boolean True */
#define AGSI_MENUBOOL_FALSE       "\xEE\x80\x83"  /* U+E003 Menu Boolean False */
#define AGSI_KEYMOD_HYPHEN        "\xEE\x80\x84"  /* U+E004 Keyboard-Modifier Hyphen */
#define AGSI_MENU_EXPANDER        "\xEE\x80\x85"  /* U+E005 Menu Expansion Arrow */
#define AGSI_BOX_VERT             "\xEE\x80\x90"  /* U+E010 Vertically Packed Box */
#define AGSI_BOX_HORIZ            "\xEE\x80\x91"  /* U+E011 Horizontally Packed Box */
#define AGSI_BUTTON               "\xEE\x80\x92"  /* U+E012 GUI Button */
#define AGSI_BEZIER               "\xEE\x80\x93"  /* U+E013 Bezier Curve */
#define AGSI_CHARSETS             "\xEE\x80\x94"  /* U+E014 Character Sets */
#define AGSI_CHECKBOX             "\xEE\x80\x95"  /* U+E015 Checkbox */
#define AGSI_WINDOW_GRADIENT      "\xEE\x80\x96"  /* U+E016 Window Gradient */
#define AGSI_CONSOLE              "\xEE\x80\x97"  /* U+E017 Console */
#define AGSI_CUSTOM_WIDGET        "\xEE\x80\x98"  /* U+E018 Custom Widget */
#define AGSI_FIXED_LAYOUT         "\xEE\x80\x99"  /* U+E019 Fixed Layout */
#define AGSI_WIDGET_FOCUS         "\xEE\x80\x9A"  /* U+E01A Widget Focus */
#define AGSI_TYPOGRAPHY           "\xEE\x80\x9B"  /* U+E01B Typography */
#define AGSI_FILESYSTEM           "\xEE\x80\x9C"  /* U+E01C Filesystem */
#define AGSI_WIREFRAME_CUBE       "\xEE\x80\x9D"  /* U+E01D Wireframe Cube */
#define AGSI_LOAD_IMAGE           "\xEE\x80\x9E"  /* U+E01E Load Image */
#define AGSI_SAVE_IMAGE           "\xEE\x80\x9F"  /* U+E01F Save Image */
#define AGSI_KEYBOARD_KEY         "\xEE\x80\xA0"  /* U+E020 Key From a Keyboard */
#define AGSI_MATH_X_EQUALS        "\xEE\x80\xA1"  /* U+E021 Mathematical "X =" */
#define AGSI_V_MAXIMIZE           "\xEE\x80\xA2"  /* U+E022 Vertical Maximize */
#define AGSI_H_MAXIMIZE           "\xEE\x80\xA3"  /* U+E023 Horizontal Maximize */
#define AGSI_MEDIUM_WINDOW        "\xEE\x80\xA4"  /* U+E024 Medium Window */
#define AGSI_SMALL_WINDOW         "\xEE\x80\xA5"  /* U+E025 Small Window */
#define AGSI_SMALL_SPHERE         "\xEE\x80\xA6"  /* U+E026 Small Sphere */
#define AGSI_LARGE_SPHERE         "\xEE\x80\xA7"  /* U+E027 Large Sphere */
#define AGSI_WINDOW_PANE          "\xEE\x80\xA8"  /* U+E028 Window Pane */
#define AGSI_RADIO_BUTTON         "\xEE\x80\xA9"  /* U+E029 Radio Button */
#define AGSI_RENDER_TO_SURFACE    "\xEE\x80\xAA"  /* U+E02A Render To Surface */
#define AGSI_HORIZ_SCROLLBAR      "\xEE\x80\xAB"  /* U+E02B Horizontal Scrollbar */
#define AGSI_VERT_SCROLLBAR       "\xEE\x80\xAC"  /* U+E02C Vertical Scrollbar */
#define AGSI_SCROLLVIEW           "\xEE\x80\xAD"  /* U+E02D Scrollview */
#define AGSI_SWORD                "\xEE\x80\xAE"  /* U+E02E Sword */
#define AGSI_NUL_TERMINATION      "\xEE\x80\xAF"  /* U+E02F NUL Termination */
#define AGSI_TABLE                "\xEE\x80\xB0"  /* U+E030 Table */
#define AGSI_TEXTBOX              "\xEE\x80\xB1"  /* U+E031 Textbox */
#define AGSI_PROGRESS_BAR         "\xEE\x80\xB2"  /* U+E032 ProgressBar */
#define AGSI_CANNED_DIALOG        "\xEE\x80\xB3"  /* U+E033 Canned Dialog */
#define AGSI_THREADS              "\xEE\x80\xB4"  /* U+E034 Threads */
#define AGSI_EMPTY_HOURGLASS      "\xEE\x80\xB5"  /* U+E035 Empty Hourglass */
#define AGSI_UNIT_CONVERSION      "\xEE\x80\xB6"  /* U+E036 Unit Conversion */
#define AGSI_USER_ACCESS          "\xEE\x80\xB7"  /* U+E037 User Access */
#define AGSI_POPULATED_WINDOW     "\xEE\x80\xB8"  /* U+E038 Populated Window */
#define AGSI_TWO_WINDOWS          "\xEE\x80\xB9"  /* U+E039 Two Windows */
#define AGSI_ALICE                "\xEE\x80\xBA"  /* U+E03A Alice */
#define AGSI_BOB                  "\xEE\x80\xBB"  /* U+E03B Bob */
#define AGSI_USER_W_3_SOUND_WAVES "\xEE\x80\xBC"  /* U+E03C User With 3 Sound Waves */
#define AGSI_FOLDED_DIAPER        "\xEE\x80\xBD"  /* U+E03D Folded Diaper */
#define AGSI_UNFOLDED_DIAPER      "\xEE\x80\xBE"  /* U+E03E Unfolded Diaper */
#define AGSI_PAPER_ROLL           "\xEE\x80\xBF"  /* U+E03F Paper Roll */
#define AGSI_CONTAINER            "\xEE\x81\x80"  /* U+E040 Container */
#define AGSI_PARCEL               "\xEE\x81\x81"  /* U+E041 Parcel */
#define AGSI_SIZE_XS              "\xEE\x81\x82"  /* U+E042 Size X-Small */
#define AGSI_SIZE_SM              "\xEE\x81\x83"  /* U+E043 Size Small */
#define AGSI_SIZE_MD              "\xEE\x81\x84"  /* U+E044 Size Medium */
#define AGSI_SIZE_LG              "\xEE\x81\x85"  /* U+E045 Size Large */
#define AGSI_SIZE_XL              "\xEE\x81\x86"  /* U+E046 Size X-Large */
#define AGSI_SIZE_2XL             "\xEE\x81\x87"  /* U+E047 Size 2XL */
#define AGSI_SIZE_3XL             "\xEE\x81\x88"  /* U+E048 Size 3XL */
#define AGSI_SIZE_4XL             "\xEE\x81\x89"  /* U+E049 Size 4XL */
#define AGSI_CLOSE_X              "\xEE\x81\x8A"  /* U+E04A Close "X" */
#define AGSI_EXPORT_DOCUMENT      "\xEE\x81\x8B"  /* U+E04B Export Document */
#define AGSI_PAD                  "\xEE\x81\x8C"  /* U+E04C Pad */
#define AGSI_DEBUGGER             "\xEE\x81\x8D"  /* U+E04D Debugger */
#define AGSI_L_MENU_EXPANDER      "\xEE\x81\x8E"  /* U+E04E Leftwise Menu Expansion Arrow */
#define AGSI_USB_STICK            "\xEE\x81\x8F"  /* U+E04F USB Stick */
#define AGSI_VERTICAL_SPOOL       "\xEE\x81\x90"  /* U+E050 Vertical Spool */
#define AGSI_HORIZONTAL_SPOOL     "\xEE\x81\x91"  /* U+E051 Horizontal Spool */
#define AGSI_DIP_CHIP             "\xEE\x81\x92"  /* U+E052 Dual Inline Package Chip */
#define AGSI_SURFACE_MOUNT_CHIP   "\xEE\x81\x93"  /* U+E053 Surface Mount Chip */
#define AGSI_VACUUM_TUBE          "\xEE\x81\x94"  /* U+E054 Vacuum Tube */
#define AGSI_ZOOM_IN              "\xEE\x81\x95"  /* U+E055 Zoom In */
#define AGSI_ZOOM_OUT             "\xEE\x81\x96"  /* U+E056 Zoom Out */
#define AGSI_ZOOM_RESET           "\xEE\x81\x97"  /* U+E057 Zoom Reset */
#define AGSI_AGAR_AG              "\xEE\x81\x98"  /* U+E058 Agar "AG" */
#define AGSI_AGAR_AR              "\xEE\x81\x99"  /* U+E059 Agar "AR" */
#define AGSI_CUT                  "\xEE\x81\x9A"  /* U+E05A Cut */
#define AGSI_COPY                 "\xEE\x81\x9B"  /* U+E05B Copy */
#define AGSI_LH_COPY              "\xEE\x81\x9C"  /* U+E05C Left-Handed Copy */
#define AGSI_CLIPBOARD            "\xEE\x81\x9D"  /* U+E05D Clipboard */
#define AGSI_PASTE                "\xEE\x81\x9E"  /* U+E05E Paste */
#define AGSI_LH_PASTE             "\xEE\x81\x9F"  /* U+E05F Left-Handed Paste */
#define AGSI_SELECT_ALL           "\xEE\x81\xA0"  /* U+E060 Select All */
#define AGSI_CLEAR_ALL            "\xEE\x81\xA1"  /* U+E061 Clear All */
#define AGSI_GAME_CONTROLLER      "\xEE\x81\xA2"  /* U+E062 Game Controller */
#define AGSI_TOUCHSCREEN          "\xEE\x81\xA3"  /* U+E063 Touchscreen */
#define AGSI_TRI_CONSTRUCTION_SIGN "\xEE\x81\xA4" /* U+E064 Triangular Construction Sign */
#define AGSI_EDGAR_ALLAN_POE       "\xEE\x81\xA5" /* U+E065 Edgar Allan Poe */
#define AGSI_AGARIAN               "\xEE\x81\xA6" /* U+E066 Agarian */
#define AGSI_PAPIGROW              AGSI_AGARIAN
#define AGSI_AGARIAN_WARRIOR       "\xEE\x81\xA7" /* U+E067 Agarian Warrior */
#define AGSI_UNDO                  "\xEE\x81\xA8" /* U+E068 Undo */
#define AGSI_REDO                  "\xEE\x81\xA9" /* U+E069 Redo */
#define AGSI_ALPHA_ARCH            "\xEE\x81\xAA" /* U+E06A Alpha Architecture */
#define AGSI_AMIGA_BALL            "\xEE\x81\xAB" /* U+E06B Amiga Ball */
#define AGSI_COMMODORE_LOGO        "\xEE\x81\xAC" /* U+E06C Commodore Logo */
#define AGSI_AMD_LOGO              "\xEE\x81\xAD" /* U+E06D AMD Logo */
#define AGSI_6502_ARCH             "\xEE\x81\xAE" /* U+E06E 6502 Architecture */
#define AGSI_AMIGA_LOGO            "\xEE\x81\xAF" /* U+E06F Amiga Logo */
#define AGSI_MOTOROLA_LOGO         "\xEE\x81\xB0" /* U+E070 Motorola Logo */
#define AGSI_MAMISMOKE             "\xEE\x81\xB1" /* U+E071 Old Lady Smoking a Joint */
#define AGSI_TGT_FG_COLOR          "\xEE\x81\xB2" /* U+E072 Target Foreground Color */
#define AGSI_TGT_BG_COLOR          "\xEE\x81\xB3" /* U+E073 Target Background Color */
#define AGSI_ARM_ARCH              "\xEE\x81\xB4" /* U+E074 ARM Architecture */
#define AGSI_DREAMCAST             "\xEE\x81\xB5" /* U+E075 Dreamcast Logo */
#define AGSI_GAMECUBE              "\xEE\x81\xB6" /* U+E076 Gamecube Console Front */
#define AGSI_SEGA                  "\xEE\x81\xB7" /* U+E077 SEGA Logo */
#define AGSI_PA_RISC_ARCH          "\xEE\x81\xB8" /* U+E078 PA-RISC Architecture */
#define AGSI_X86_ARCH              "\xEE\x81\xB9" /* U+E079 x86 Architecture */
#define AGSI_X64_ARCH              "\xEE\x81\xBA" /* U+E07A x64 Architecture */
#define AGSI_I386_ARCH             "\xEE\x81\xBB" /* U+E07B i386 Architecture */
#define AGSI_JSON                  "\xEE\x81\xBC" /* U+E07C JSON Format */
#define AGSI_NES_CONTROLLER        "\xEE\x81\xBD" /* U+E07D NES Controller */
#define AGSI_MIPS32_ARCH           "\xEE\x81\xBE" /* U+E07E MIPS32 Architecture */
#define AGSI_MIPS64_ARCH           "\xEE\x81\xBF" /* U+E07F MIPS64 Architecture */
#define AGSI_N64_LOGO              "\xEE\x82\x80" /* U+E080 N64 Logo */
#define AGSI_IA64_ARCH             "\xEE\x82\x81" /* U+E081 IA64 Architecture */
#define AGSI_PPC32_ARCH            "\xEE\x82\x82" /* U+E082 PPC32 Architecture */
#define AGSI_PPC64_ARCH            "\xEE\x82\x83" /* U+E083 PPC64 Architecture */
#define AGSI_SNES_LOGO             "\xEE\x82\x84" /* U+E084 SNES Logo */
#define AGSI_RISCV_ARCH            "\xEE\x82\x85" /* U+E085 RISCV Architecture */

