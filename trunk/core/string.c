/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * General-purpose multilanguage text structure.
 */

#include <core/core.h>

/* Language codes (see string.h, iso639-gen.pl) */
const char *agLanguageCodes[] = {
        "??", "aa", "ab", "af", "am", "ar", "as", "ay", "az", "ba", "be", "bg", 
        "bh", "bi", "bn", "bo", "br", "ca", "co", "cs", "cy", "da", "de", "dz", 
        "el", "en", "eo", "es", "et", "eu", "fa", "fi", "fj", "fo", "fr", "fy", 
        "ga", "gd", "gl", "gn", "gu", "ha", "he", "hi", "hr", "hu", "hy", "ia", 
        "id", "ie", "ik", "is", "it", "iu", "ja", "jw", "ka", "kk", "kl", "km", 
        "kn", "ko", "ks", "ku", "ky", "la", "ln", "lo", "lt", "lv", "mg", "mi", 
        "mk", "ml", "mn", "mo", "mr", "ms", "mt", "my", "na", "ne", "nl", "no", 
        "oc", "om", "or", "pa", "pl", "ps", "pt", "qu", "rm", "rn", "ro", "ru", 
        "rw", "sa", "sd", "sg", "sh", "si", "sk", "sl", "sm", "sn", "so", "sq", 
        "sr", "ss", "st", "su", "sv", "sw", "ta", "te", "tg", "th", "ti", "tk", 
        "tl", "tn", "to", "tr", "ts", "tt", "tw", "ug", "uk", "ur", "uz", "vi", 
        "vo", "wo", "xh", "yi", "yo", "za", "zh", "zu"
};
const char *agLanguageNames[] = {
	N_("Undefined"),
	N_("Afar"),
	N_("Abkhazian"),
	N_("Afrikaans"),
	N_("Amharic"),
	N_("Arabic"),
	N_("Assamese"),
	N_("Aymara"),
	N_("Azerbaijani"),
	N_("Bashkir"),
	N_("Byelorussian"),
	N_("Bulgarian"),
	N_("Bihari"),
	N_("Bislama"),
	N_("Bengali; Bangla"),
	N_("Tibetan"),
	N_("Breton"),
	N_("Catalan"),
	N_("Corsican"),
	N_("Czech"),
	N_("Welsh"),
	N_("Danish"),
	N_("German"),
	N_("Bhutani"),
	N_("Greek"),
	N_("English"),
	N_("Esperanto"),
	N_("Spanish"),
	N_("Estonian"),
	N_("Basque"),
	N_("Persian"),
	N_("Finnish"),
	N_("Fiji"),
	N_("Faroese"),
	N_("French"),
	N_("Frisian"),
	N_("Irish"),
	N_("Scots Gaelic"),
	N_("Galician"),
	N_("Guarani"),
	N_("Gujarati"),
	N_("Hausa"),
	N_("Hebrew (formerly iw)"),
	N_("Hindi"),
	N_("Croatian"),
	N_("Hungarian"),
	N_("Armenian"),
	N_("Interlingua"),
	N_("Indonesian (formerly in)"),
	N_("Interlingue"),
	N_("Inupiak"),
	N_("Icelandic"),
	N_("Italian"),
	N_("Inuktitut"),
	N_("Japanese"),
	N_("Javanese"),
	N_("Georgian"),
	N_("Kazakh"),
	N_("Greenlandic"),
	N_("Cambodian"),
	N_("Kannada"),
	N_("Korean"),
	N_("Kashmiri"),
	N_("Kurdish"),
	N_("Kirghiz"),
	N_("Latin"),
	N_("Lingala"),
	N_("Laothian"),
	N_("Lithuanian"),
	N_("Latvian, Lettish"),
	N_("Malagasy"),
	N_("Maori"),
	N_("Macedonian"),
	N_("Malayalam"),
	N_("Mongolian"),
	N_("Moldavian"),
	N_("Marathi"),
	N_("Malay"),
	N_("Maltese"),
	N_("Burmese"),
	N_("Nauru"),
	N_("Nepali"),
	N_("Dutch"),
	N_("Norwegian"),
	N_("Occitan"),
	N_("(Afan) Oromo"),
	N_("Oriya"),
	N_("Punjabi"),
	N_("Polish"),
	N_("Pashto, Pushto"),
	N_("Portuguese"),
	N_("Quechua"),
	N_("Rhaeto-Romance"),
	N_("Kirundi"),
	N_("Romanian"),
	N_("Russian"),
	N_("Kinyarwanda"),
	N_("Sanskrit"),
	N_("Sindhi"),
	N_("Sangho"),
	N_("Serbo-Croatian"),
	N_("Sinhalese"),
	N_("Slovak"),
	N_("Slovenian"),
	N_("Samoan"),
	N_("Shona"),
	N_("Somali"),
	N_("Albanian"),
	N_("Serbian"),
	N_("Siswati"),
	N_("Sesotho"),
	N_("Sundanese"),
	N_("Swedish"),
	N_("Swahili"),
	N_("Tamil"),
	N_("Telugu"),
	N_("Tajik"),
	N_("Thai"),
	N_("Tigrinya"),
	N_("Turkmen"),
	N_("Tagalog"),
	N_("Setswana"),
	N_("Tonga"),
	N_("Turkish"),
	N_("Tsonga"),
	N_("Tatar"),
	N_("Twi"),
	N_("Uighur"),
	N_("Ukrainian"),
	N_("Urdu"),
	N_("Uzbek"),
	N_("Vietnamese"),
	N_("Volapuk"),
	N_("Wolof"),
	N_("Xhosa"),
	N_("Yiddish (formerly ji)"),
	N_("Yoruba"),
	N_("Zhuang"),
	N_("Chinese"),
	N_("Zulu")
};

/* Allocate a new string; if s argument is given, set as undefined language entry. */
AG_String *
AG_StringNewS(const char *s)
{
	AG_String *as;
	Uint i;

	if ((as = AG_TryMalloc(sizeof(AG_String))) == NULL) {
		return (NULL);
	}
	for (i = 0; i < AG_LANG_LAST; i++) {
		AG_StringEnt *se = &as->ent[i];
		se->buf = NULL;
		se->bufSize = 0;
		se->len = 0;
	}
	as->lang = AG_LANG_NONE;
	if (AG_MutexTryInit(&as->lock) == -1) {
		Free(as);
		return (NULL);
	}
	if (s != NULL &&
	    AG_StringSetS(as, s) == -1) {
		AG_StringFree(as);
		return (NULL);
	}
	return (as);
}

/* Allocate a new string; if fmt is non-NULL, set as undefined language entry. */
AG_String *
AG_StringNew(const char *fmt, ...)
{
	AG_String *as;

	if ((as = AG_StringNewS(NULL)) == NULL) {
		return (NULL);
	}
	if (fmt != NULL) {
		AG_StringEnt *se = &as->ent[AG_LANG_NONE];
		va_list ap;
	
		va_start(ap, fmt);
		if (vasprintf(&se->buf, fmt, ap) == -1) {
			AG_StringFree(as);
			return (NULL);
		}
		va_end(ap);
		se->len = strlen(se->buf);
		se->bufSize = se->len+1;
	}
	return (as);
}

/* Free an AG_String structure. */
void
AG_StringFree(AG_String *as)
{
	int i;

	for (i = 0; i < AG_LANG_LAST; i++) {
		Free(as->ent[i].buf);
	}
	AG_MutexDestroy(&as->lock);
	Free(as);
}

/* Set current language entry (format string). */
int
AG_StringSet(AG_String *as, const char *fmt, ...)
{
	AG_StringEnt *se = &as->ent[as->lang];
	va_list ap;
	
	Free(se->buf);
	va_start(ap, fmt);
	if (vasprintf(&se->buf, fmt, ap) == -1) {
		return (-1);
	}
	va_end(ap);
	se->len = strlen(se->buf);
	se->bufSize = se->len+1;
	return (0);
}

/* Set current language entry (C string). */
int
AG_StringSetS(AG_String *as, const char *s)
{
	AG_StringEnt *se;
	char *sNew;

	if ((sNew = TryStrdup(s)) == NULL) {
		return (-1);
	}
	AG_MutexLock(&as->lock);
	se = &as->ent[as->lang];
	Free(se->buf);
	se->buf = sNew;
	se->len = strlen(sNew);
	se->bufSize = se->len+1;
	AG_MutexUnlock(&as->lock);
	return (0);
}

/* Return ISO-639 code for current string language. */
const char *
AG_StringGetLangISO639(AG_String *as)
{
	const char *s;

	AG_MutexLock(&as->lock);
	s = agLanguageCodes[as->lang];
	AG_MutexUnlock(&as->lock);
	return (s);
}

/* Set current string language (per ISO-639 code) */
int
AG_StringSetLangISO639(AG_String *as, const char *iso)
{
	int i;

	for (i = 0; i < AG_LANG_LAST; i++) {
		if (Strcasecmp(agLanguageCodes[i], iso) == 0)
			break;
	}
	if (i == AG_LANG_LAST) {
		AG_SetError("No such language: %s", iso);
		return (-1);
	}
	as->lang = (enum ag_language)i;
	return (0);
}

/* Duplicate a string. */
AG_String *
AG_StringDup(AG_String *ss)
{
	AG_String *ds;
	int i;

	if ((ds = AG_StringNewS(NULL)) == NULL) {
		return (NULL);
	}
	AG_MutexLock(&ds->lock);
	AG_MutexLock(&ss->lock);
	ds->lang = ss->lang;
	for (i = 0; i < AG_LANG_LAST; i++) {
		AG_StringEnt *se = &ss->ent[i];

		if (se->buf != NULL) {
			AG_StringEnt *de = &ds->ent[i];

			if ((de->buf = TryStrdup(se->buf)) == NULL) {
				goto fail;
			}
			de->len = strlen(de->buf);
			de->bufSize = de->len+1;
		}
	}
	AG_MutexUnlock(&ds->lock);
	AG_MutexUnlock(&ss->lock);
	return (ds);
fail:
	AG_MutexUnlock(&ds->lock);
	AG_MutexUnlock(&ss->lock);
	AG_StringFree(ds);
	return (NULL);
}
