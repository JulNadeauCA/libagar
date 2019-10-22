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
# define AGSI_CROSSDOUT  "\x1b[9m"   /* crossed-out */
# define AGSI_FONT1	 "\x1b[10m"  /* a modification of Bitstream Vera */
# define AGSI_FONT2	 "\x1b[11m"  /* a modification of Bitstream Vera Mono */
# define AGSI_FONT3	 "\x1b[12m"  /* a modification of Computer Modern Serif */
# define AGSI_FONT4	 "\x1b[13m"  /* a modification of Computer Modern Sans */
# define AGSI_FONT5	 "\x1b[14m"  /* a modification of Computer Modern Typewriter */
# define AGSI_FONT6	 "\x1b[15m"  /* a modification of Computer Modern Serif Upright Italic */
# define AGSI_FONT7	 "\x1b[16m"  /* a modification of Bitstream Charter */
# define AGSI_FONT8	 "\x1b[17m"  /* a modification of Bedstead (Mode 7) */
# define AGSI_FONT9	 "\x1b[18m"  /* a modification of Courier Prime */
# define AGSI_FONT10	 "\x1b[19m"  /* a modification of DejaVu Sans */
# define AGSI_FONT11	 "\x1b[20m"  /* a modification of Unifraktur Maguntia */
# define AGSI_FRAK	 AGSI_FONT11 /* fraktur style */
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
# define AGSI_BR_RED_BG	 "\x1b[101m" /* bright red background */
# define AGSI_BR_GRN_BG	 "\x1b[102m" /* bright green background */
# define AGSI_BR_YEL_BG	 "\x1b[103m" /* bright yellow background */
# define AGSI_BR_BLU_BG	 "\x1b[104m" /* bright blue background */
# define AGSI_BR_MAG_BG	 "\x1b[105m" /* bright magenta background */
# define AGSI_BR_CYAN_BG "\x1b[106m" /* bright cyan background */
# define AGSI_BR_WHT_BG	 "\x1b[107m" /* bright white background */

#else /* !AG_ANSI_COLOR */

# define AGSI_RST 	""
# define AGSI_BOLD     	""
# define AGSI_FAINT    	""
# define AGSI_ITALIC   	""
# define AGSI_UNDERLINE	""
# define AGSI_REVERSE  	""
# define AGSI_CROSSDOUT ""
# define AGSI_FONT1	""
# define AGSI_FONT2	""
# define AGSI_FONT3	""
# define AGSI_FONT4	""
# define AGSI_FONT5	""
# define AGSI_FONT6	""
# define AGSI_FONT7	""
# define AGSI_FONT8	""
# define AGSI_FONT9	""
# define AGSI_FONT10	""
# define AGSI_FONT11	""
# define AGSI_FRAK	""
# define AGSI_FRAMED	""
# define AGSI_ENCIRCLED ""
# define AGSI_OVERLINED ""
# define AGSI_BLK	""
# define AGSI_RED	""
# define AGSI_GRN	""
# define AGSI_YEL	""
# define AGSI_BLU	""
# define AGSI_MAG	""
# define AGSI_CYA	""
# define AGSI_WHT	""
# define AGSI_BR_BLK	""
# define AGSI_BR_RED	""
# define AGSI_BR_GRN	""
# define AGSI_BR_YEL	""
# define AGSI_BR_BLU	""
# define AGSI_BR_MAG	""
# define AGSI_BR_CYA	""
# define AGSI_BR_WHT	""
# define AGSI_BLK_BG	""
# define AGSI_RED_BG	""
# define AGSI_GRN_BG	""
# define AGSI_YEL_BG	""
# define AGSI_BLU_BG	""
# define AGSI_MAG_BG	""
# define AGSI_CYA_BG	""
# define AGSI_WHT_BG	""
# define AGSI_BR_BLK_BG	""
# define AGSI_BR_RED_BG	""
# define AGSI_BR_GRN_BG	""
# define AGSI_BR_YEL_BG	""
# define AGSI_BR_BLU_BG	""
# define AGSI_BR_MAG_BG	""
# define AGSI_BR_CYA_BG	""
# define AGSI_BR_WHT_BG	""

#endif /* AG_ANSI_COLOR */

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
