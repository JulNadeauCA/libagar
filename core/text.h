/*	Public domain	*/

#ifndef _AGAR_CORE_TEXT_H_
#define _AGAR_CORE_TEXT_H_
#include <agar/core/begin.h>

/* Language code (see iso639-gen.pl) */
typedef enum ag_language {
	AG_LANG_NONE,	/* Undefined */
	AG_LANG_AA,	/* Afar */
	AG_LANG_AB,	/* Abkhazian */
	AG_LANG_AF,	/* Afrikaans */
	AG_LANG_AM,	/* Amharic */
	AG_LANG_AR,	/* Arabic */
	AG_LANG_AS,	/* Assamese */
	AG_LANG_AY,	/* Aymara */
	AG_LANG_AZ,	/* Azerbaijani */
	AG_LANG_BA,	/* Bashkir */
	AG_LANG_BE,	/* Byelorussian */
	AG_LANG_BG,	/* Bulgarian */
	AG_LANG_BH,	/* Bihari */
	AG_LANG_BI,	/* Bislama */
	AG_LANG_BN,	/* Bengali; Bangla */
	AG_LANG_BO,	/* Tibetan */
	AG_LANG_BR,	/* Breton */
	AG_LANG_CA,	/* Catalan */
	AG_LANG_CO,	/* Corsican */
	AG_LANG_CS,	/* Czech */
	AG_LANG_CY,	/* Welsh */
	AG_LANG_DA,	/* Danish */
	AG_LANG_DE,	/* German */
	AG_LANG_DZ,	/* Bhutani */
	AG_LANG_EL,	/* Greek */
	AG_LANG_EN,	/* English */
	AG_LANG_EO,	/* Esperanto */
	AG_LANG_ES,	/* Spanish */
	AG_LANG_ET,	/* Estonian */
	AG_LANG_EU,	/* Basque */
	AG_LANG_FA,	/* Persian */
	AG_LANG_FI,	/* Finnish */
	AG_LANG_FJ,	/* Fiji */
	AG_LANG_FO,	/* Faroese */
	AG_LANG_FR,	/* French */
	AG_LANG_FY,	/* Frisian */
	AG_LANG_GA,	/* Irish */
	AG_LANG_GD,	/* Scots Gaelic */
	AG_LANG_GL,	/* Galician */
	AG_LANG_GN,	/* Guarani */
	AG_LANG_GU,	/* Gujarati */
	AG_LANG_HA,	/* Hausa */
	AG_LANG_HE,	/* Hebrew (formerly iw) */
	AG_LANG_HI,	/* Hindi */
	AG_LANG_HR,	/* Croatian */
	AG_LANG_HU,	/* Hungarian */
	AG_LANG_HY,	/* Armenian */
	AG_LANG_IA,	/* Interlingua */
	AG_LANG_ID,	/* Indonesian (formerly in) */
	AG_LANG_IE,	/* Interlingue */
	AG_LANG_IK,	/* Inupiak */
	AG_LANG_IS,	/* Icelandic */
	AG_LANG_IT,	/* Italian */
	AG_LANG_IU,	/* Inuktitut */
	AG_LANG_JA,	/* Japanese */
	AG_LANG_JW,	/* Javanese */
	AG_LANG_KA,	/* Georgian */
	AG_LANG_KK,	/* Kazakh */
	AG_LANG_KL,	/* Greenlandic */
	AG_LANG_KM,	/* Cambodian */
	AG_LANG_KN,	/* Kannada */
	AG_LANG_KO,	/* Korean */
	AG_LANG_KS,	/* Kashmiri */
	AG_LANG_KU,	/* Kurdish */
	AG_LANG_KY,	/* Kirghiz */
	AG_LANG_LA,	/* Latin */
	AG_LANG_LN,	/* Lingala */
	AG_LANG_LO,	/* Laothian */
	AG_LANG_LT,	/* Lithuanian */
	AG_LANG_LV,	/* Latvian, Lettish */
	AG_LANG_MG,	/* Malagasy */
	AG_LANG_MI,	/* Maori */
	AG_LANG_MK,	/* Macedonian */
	AG_LANG_ML,	/* Malayalam */
	AG_LANG_MN,	/* Mongolian */
	AG_LANG_MO,	/* Moldavian */
	AG_LANG_MR,	/* Marathi */
	AG_LANG_MS,	/* Malay */
	AG_LANG_MT,	/* Maltese */
	AG_LANG_MY,	/* Burmese */
	AG_LANG_NA,	/* Nauru */
	AG_LANG_NE,	/* Nepali */
	AG_LANG_NL,	/* Dutch */
	AG_LANG_NO,	/* Norwegian */
	AG_LANG_OC,	/* Occitan */
	AG_LANG_OM,	/* (Afan) Oromo */
	AG_LANG_OR,	/* Oriya */
	AG_LANG_PA,	/* Punjabi */
	AG_LANG_PL,	/* Polish */
	AG_LANG_PS,	/* Pashto, Pushto */
	AG_LANG_PT,	/* Portuguese */
	AG_LANG_QU,	/* Quechua */
	AG_LANG_RM,	/* Rhaeto-Romance */
	AG_LANG_RN,	/* Kirundi */
	AG_LANG_RO,	/* Romanian */
	AG_LANG_RU,	/* Russian */
	AG_LANG_RW,	/* Kinyarwanda */
	AG_LANG_SA,	/* Sanskrit */
	AG_LANG_SD,	/* Sindhi */
	AG_LANG_SG,	/* Sangho */
	AG_LANG_SH,	/* Serbo-Croatian */
	AG_LANG_SI,	/* Sinhalese */
	AG_LANG_SK,	/* Slovak */
	AG_LANG_SL,	/* Slovenian */
	AG_LANG_SM,	/* Samoan */
	AG_LANG_SN,	/* Shona */
	AG_LANG_SO,	/* Somali */
	AG_LANG_SQ,	/* Albanian */
	AG_LANG_SR,	/* Serbian */
	AG_LANG_SS,	/* Siswati */
	AG_LANG_ST,	/* Sesotho */
	AG_LANG_SU,	/* Sundanese */
	AG_LANG_SV,	/* Swedish */
	AG_LANG_SW,	/* Swahili */
	AG_LANG_TA,	/* Tamil */
	AG_LANG_TE,	/* Telugu */
	AG_LANG_TG,	/* Tajik */
	AG_LANG_TH,	/* Thai */
	AG_LANG_TI,	/* Tigrinya */
	AG_LANG_TK,	/* Turkmen */
	AG_LANG_TL,	/* Tagalog */
	AG_LANG_TN,	/* Setswana */
	AG_LANG_TO,	/* Tonga */
	AG_LANG_TR,	/* Turkish */
	AG_LANG_TS,	/* Tsonga */
	AG_LANG_TT,	/* Tatar */
	AG_LANG_TW,	/* Twi */
	AG_LANG_UG,	/* Uighur */
	AG_LANG_UK,	/* Ukrainian */
	AG_LANG_UR,	/* Urdu */
	AG_LANG_UZ,	/* Uzbek */
	AG_LANG_VI,	/* Vietnamese */
	AG_LANG_VO,	/* Volapuk */
	AG_LANG_WO,	/* Wolof */
	AG_LANG_XH,	/* Xhosa */
	AG_LANG_YI,	/* Yiddish (formerly ji) */
	AG_LANG_YO,	/* Yoruba */
	AG_LANG_ZA,	/* Zhuang */
	AG_LANG_ZH,	/* Chinese */
	AG_LANG_ZU,	/* Zulu */
	AG_LANG_LAST
} AG_Language;

/* Text entry */
typedef struct ag_text_ent {
	char  *_Nullable buf;		/* String buffer */
	AG_Size maxLen;			/* Length (allocated) */
	AG_Size len;			/* Length (chars) */
} AG_TextEnt;

/* Text object */
typedef struct ag_text {
	_Nonnull_Mutex AG_Mutex lock;
	AG_TextEnt ent[AG_LANG_LAST];	/* Language entries */
	AG_Language lang;		/* Selected language */
	Uint flags;
#define AG_TEXT_SAVED_FLAGS	0
	AG_Size maxLen;			/* Maximum string length (bytes) */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
} AG_Text, AG_TextElement;

#define AGTEXTELEMENT(p) ((AG_TextElement *)(p))

__BEGIN_DECLS
#include <agar/config/ag_unicode.h>
#ifdef AG_UNICODE

extern const char *_Nonnull agLanguageCodes[];
extern const char *_Nonnull agLanguageNames[];

AG_Text *_Nonnull  AG_TextNew(AG_Size);
AG_Text *_Nullable AG_TextDup(AG_Text *_Nonnull);
void               AG_TextInit(AG_Text *_Nonnull, AG_Size);
void               AG_TextDestroy(AG_Text *_Nonnull);
void               AG_TextFree(AG_Text *_Nullable);

int  AG_TextRealloc(AG_TextEnt *_Nonnull, AG_Size);
void AG_TextClear(AG_Text *_Nonnull);
void AG_TextSetLimit(AG_Text *_Nonnull, AG_Size);
int  AG_TextCatS(AG_Text *_Nonnull, const char *_Nonnull);
int  AG_TextSetS(AG_Text *_Nonnull, const char *_Nonnull);
int  AG_TextSet(AG_Text *_Nonnull, const char *_Nullable, ...)
               FORMAT_ATTRIBUTE(__printf__,2,3);
int  AG_TextSetEntS(AG_Text *_Nonnull, AG_Language, const char *_Nonnull);
int  AG_TextSetEnt(AG_Text *_Nonnull, AG_Language,
                   const char *_Nonnull, ...)
                  FORMAT_ATTRIBUTE(__printf__,3,4);

int                  AG_TextSetLangISO(AG_Text *_Nonnull, const char *_Nonnull);
const char *_Nonnull AG_TextGetLangISO(AG_Text *_Nonnull);

#ifdef AG_SERIALIZATION
int  AG_TextLoad(AG_Text *_Nonnull, AG_DataSource *_Nonnull);
void AG_TextSave(AG_DataSource *_Nonnull, AG_Text *_Nonnull);
#endif

void        AG_TextSetLang(AG_Text *_Nonnull, AG_Language);
AG_Language AG_TextGetLang(AG_Text *_Nonnull)
                          _Pure_Attribute_If_Unthreaded;

#endif /* AG_UNICODE */

__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_TEXT_H_ */
