/*
 * Copyright (c) 2012-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Dynamically-allocated, multilanguage text element.
 */

#include <agar/config/ag_unicode.h>
#ifdef AG_UNICODE

#include <agar/core/core.h>

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
	N_("Afar"),		N_("Abkhazian"),	N_("Afrikaans"),
	N_("Amharic"),		N_("Arabic"),		N_("Assamese"),
	N_("Aymara"),		N_("Azerbaijani"),	N_("Bashkir"),
	N_("Byelorussian"),	N_("Bulgarian"),	N_("Bihari"),
	N_("Bislama"),		N_("Bengali; Bangla"),	N_("Tibetan"),
	N_("Breton"),		N_("Catalan"),		N_("Corsican"),
	N_("Czech"),		N_("Welsh"),		N_("Danish"),
	N_("German"),		N_("Bhutani"),		N_("Greek"),
	N_("English"),		N_("Esperanto"),	N_("Spanish"),
	N_("Estonian"),		N_("Basque"),		N_("Persian"),
	N_("Finnish"),		N_("Fiji"),		N_("Faroese"),
	N_("French"),		N_("Frisian"),		N_("Irish"),
	N_("Scots Gaelic"),	N_("Galician"),		N_("Guarani"),
	N_("Gujarati"),		N_("Hausa"),		N_("Hebrew"),
	N_("Hindi"),		N_("Croatian"),		N_("Hungarian"),
	N_("Armenian"),		N_("Interlingua"),	N_("Indonesian"),
	N_("Interlingue"),	N_("Inupiak"),		N_("Icelandic"),
	N_("Italian"),		N_("Inuktitut"),	N_("Japanese"),
	N_("Javanese"),		N_("Georgian"),		N_("Kazakh"),
	N_("Greenlandic"),	N_("Cambodian"),	N_("Kannada"),
	N_("Korean"),		N_("Kashmiri"),		N_("Kurdish"),
	N_("Kirghiz"),		N_("Latin"),		N_("Lingala"),
	N_("Laothian"),		N_("Lithuanian"),	N_("Latvian, Lettish"),
	N_("Malagasy"),		N_("Maori"),		N_("Macedonian"),
	N_("Malayalam"),	N_("Mongolian"),	N_("Moldavian"),
	N_("Marathi"),		N_("Malay"),		N_("Maltese"),
	N_("Burmese"),		N_("Nauru"),		N_("Nepali"),
	N_("Dutch"),		N_("Norwegian"),	N_("Occitan"),
	N_("(Afan) Oromo"),	N_("Oriya"),		N_("Punjabi"),
	N_("Polish"),		N_("Pashto, Pushto"),	N_("Portuguese"),
	N_("Quechua"),		N_("Rhaeto-Romance"),	N_("Kirundi"),
	N_("Romanian"),		N_("Russian"),		N_("Kinyarwanda"),
	N_("Sanskrit"),		N_("Sindhi"),		N_("Sangho"),
	N_("Serbo-Croatian"),	N_("Sinhalese"),	N_("Slovak"),
	N_("Slovenian"),	N_("Samoan"),		N_("Shona"),
	N_("Somali"),		N_("Albanian"),		N_("Serbian"),
	N_("Siswati"),		N_("Sesotho"),		N_("Sundanese"),
	N_("Swedish"),		N_("Swahili"),		N_("Tamil"),
	N_("Telugu"),		N_("Tajik"),		N_("Thai"),
	N_("Tigrinya"),		N_("Turkmen"),		N_("Tagalog"),
	N_("Setswana"),		N_("Tonga"),		N_("Turkish"),
	N_("Tsonga"),		N_("Tatar"),		N_("Twi"),
	N_("Uighur"),		N_("Ukrainian"),	N_("Urdu"),
	N_("Uzbek"),		N_("Vietnamese"),	N_("Volapuk"),
	N_("Wolof"),		N_("Xhosa"),		N_("Yiddish"),
	N_("Yoruba"),		N_("Zhuang"),		N_("Chinese"),
	N_("Zulu")
};

/* Initialize a static AG_Text element. */
void
AG_TextInit(AG_Text *txt, AG_Size maxLen)
{
	Uint i;
	
	txt->lang = 0;
	txt->flags = 0;
	txt->maxLen = (maxLen != 0) ? maxLen : AG_INT_MAX-1;
	AG_MutexInitRecursive(&txt->lock);

	for (i = 0; i < AG_LANG_LAST; i++) {
		AG_TextEnt *te = &txt->ent[i];
		te->buf = NULL;
		te->maxLen = 0;
		te->len = 0;
	}
}

/* Release an AG_Text element. */
void
AG_TextDestroy(AG_Text *txt)
{
	AG_TextClear(txt);
	AG_MutexDestroy(&txt->lock);
}

/* Create an autoallocated AG_Text element. */
AG_Text *
AG_TextNew(AG_Size maxLen)
{
	AG_Text *txt = Malloc(sizeof(AG_Text));
	AG_TextInit(txt, maxLen);
	return (txt);
}

/* Delete all string data in an AG_Text element. */
void
AG_TextClear(AG_Text *txt)
{
	int i;

	AG_MutexLock(&txt->lock);
	for (i = 0; i < AG_LANG_LAST; i++) {
		AG_TextEnt *te = &txt->ent[i];

		Free(te->buf);
		te->buf = NULL;
		te->maxLen = 0;
		te->len = 0;
	}
	AG_MutexUnlock(&txt->lock);
}

/* Set text of active AG_Text entry (format string). */
int
AG_TextSet(AG_Text *txt, const char *fmt, ...)
{
	AG_TextEnt *te;
	va_list ap;
	char *sNew;

	if (fmt != NULL) {
		va_start(ap, fmt);
		if (TryVasprintf(&sNew, fmt, ap) != -1) {
			return (-1);
		}
		va_end(ap);
	} else {
		sNew = NULL;
	}

	AG_MutexLock(&txt->lock);
	te = &txt->ent[txt->lang];
	Free(te->buf);
	te->buf = sNew;
	if (sNew != NULL) {
		te->len = strlen(sNew);
		te->maxLen = te->len+1;
	} else {
		te->len = 0;
		te->maxLen = 0;
	}
	AG_MutexUnlock(&txt->lock);
	return (0);
}

/* Set text of specified AG_Text entry (format string). */
int
AG_TextSetEnt(AG_Text *txt, enum ag_language lang, const char *fmt, ...)
{
	AG_TextEnt *te;
	va_list ap;
	char *sNew;
	
	va_start(ap, fmt);
	if (TryVasprintf(&sNew, fmt, ap) == -1) {
		return (-1);
	}
	va_end(ap);
	
	AG_MutexLock(&txt->lock);
	te = &txt->ent[lang];
	Free(te->buf);
	te->buf = sNew;
	if (sNew != NULL) {
		te->len = strlen(te->buf);
		te->maxLen = te->len+1;
	} else {
		te->len = 0;
		te->maxLen = 0;
	}
	AG_MutexUnlock(&txt->lock);
	return (0);
}

/* Set text of specified AG_Text entry (C string). */
int
AG_TextSetEntS(AG_Text *txt, enum ag_language lang, const char *s)
{
	AG_TextEnt *te;
	char *sNew;

	if (s != NULL) {
		if ((sNew = TryStrdup(s)) == NULL)
			return (-1);
	} else {
		sNew = NULL;
	}

	AG_MutexLock(&txt->lock);
	te = &txt->ent[lang];
	Free(te->buf);
	te->buf = sNew;
	if (sNew != NULL) {
		te->len = strlen(sNew);
		te->maxLen = te->len+1;
	} else {
		te->len = 0;
		te->maxLen = 0;
	}
	AG_MutexUnlock(&txt->lock);
	return (0);
}

/* Return ISO-639 code for current string language. */
const char *
AG_TextGetLangISO(AG_Text *txt)
{
	const char *s;

	AG_MutexLock(&txt->lock);
	s = agLanguageCodes[txt->lang];
	AG_MutexUnlock(&txt->lock);
	return (s);
}

/* Set current string language (per ISO-639 code) */
int
AG_TextSetLangISO(AG_Text *txt, const char *iso)
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
	AG_MutexLock(&txt->lock);
	txt->lang = (enum ag_language)i;
	AG_MutexUnlock(&txt->lock);
	return (0);
}

/* Duplicate a string. */
AG_Text *
AG_TextDup(AG_Text *txtSrc)
{
	AG_Text *txtDst;
	int i;

	if ((txtDst = AG_TextNew(txtSrc->maxLen)) == NULL) {
		return (NULL);
	}
	AG_MutexLock(&txtDst->lock);
	AG_MutexLock(&txtSrc->lock);
	txtDst->lang = txtSrc->lang;
	txtDst->flags &= ~(AG_TEXT_SAVED_FLAGS);
	txtDst->flags |= (txtSrc->flags & AG_TEXT_SAVED_FLAGS);
	for (i = 0; i < AG_LANG_LAST; i++) {
		AG_TextEnt *te = &txtSrc->ent[i];

		if (te->buf != NULL) {
			AG_TextEnt *de = &txtDst->ent[i];

			if ((de->buf = TryStrdup(te->buf)) == NULL) {
				goto fail;
			}
			de->len = strlen(de->buf);
			de->maxLen = de->len+1;
		}
	}
	AG_MutexUnlock(&txtDst->lock);
	AG_MutexUnlock(&txtSrc->lock);
	return (txtDst);
fail:
	AG_MutexUnlock(&txtDst->lock);
	AG_MutexUnlock(&txtSrc->lock);
	AG_TextFree(txtDst);
	return (NULL);
}

#ifdef AG_SERIALIZATION
/* Load text from a data source. */
int
AG_TextLoad(AG_Text *txt, AG_DataSource *ds)
{
	Uint i, count;
	enum ag_language lang;
	AG_TextEnt *te;
	
	count = (Uint)AG_ReadUint8(ds);
	if (count >= AG_LANG_LAST) {
		AG_SetError("Bad language count");
		return (-1);
	}
	AG_MutexLock(&txt->lock);
	AG_TextClear(txt);
	for (i = 0; i < count; i++) {
		lang = (enum ag_language)AG_ReadUint32(ds);
		if (lang >= AG_LANG_LAST) {
			AG_SetError("Bad language code (%u)", lang);
			goto fail;
		}
		te = &txt->ent[lang];
		if ((te->buf = AG_ReadStringLen(ds, txt->maxLen)) == NULL) {
			goto fail;
		}
		te->len = strlen(te->buf);
		te->maxLen = te->len+1;
	}
	txt->flags &= ~(AG_TEXT_SAVED_FLAGS);
	txt->flags |= (Uint)(AG_ReadUint32(ds) & AG_TEXT_SAVED_FLAGS);
	AG_MutexUnlock(&txt->lock);
	return (0);
fail:
	AG_MutexUnlock(&txt->lock);
	return (-1);
}

void
AG_TextSave(AG_DataSource *ds, AG_Text *txt)
{
	Uint count, i;
	AG_TextEnt *te;

	AG_MutexLock(&txt->lock);
	for (i = 0, count = 0; i < AG_LANG_LAST; i++) {
		te = &txt->ent[i];
		if (te->buf != NULL)
			count++;
	}
	AG_WriteUint8(ds, (Uint8)count);
	for (i = 0; i < AG_LANG_LAST; i++) {
		te = &txt->ent[i];
		if (te->buf != NULL) {
			AG_WriteUint32(ds, (Uint32)i);
			AG_WriteString(ds, te->buf);
		}
	}
	AG_WriteUint32(ds, (Uint32)txt->flags & AG_TEXT_SAVED_FLAGS);
	AG_MutexUnlock(&txt->lock);
}
#endif /* AG_SERIALIZATION */

/* Set the text of an element (in its active language) */
int
AG_TextSetS(AG_Text *txt, const char *s)
{
	int rv;

	AG_MutexLock(&txt->lock);
	rv = AG_TextSetEntS(txt, txt->lang, s);
	AG_MutexUnlock(&txt->lock);
	return (rv);
}

/* Set a dynamically-enforced size limit on a text element */
void
AG_TextSetLimit(AG_Text *txt, AG_Size maxLen)
{
	AG_MutexLock(&txt->lock);
	txt->maxLen = maxLen;
	AG_MutexUnlock(&txt->lock);
}

/* Select the active language of a text element. */
void
AG_TextSetLang(AG_Text *_Nonnull txt, AG_Language lang)
{
	AG_MutexLock(&txt->lock);
	txt->lang = lang;
	AG_MutexUnlock(&txt->lock);
}

/* Get the active language of a text element. */
AG_Language
AG_TextGetLang(AG_Text *txt)
{
	enum ag_language lang;

	AG_MutexLock(&txt->lock);
	lang = txt->lang;
	AG_MutexUnlock(&txt->lock);
	return (lang);
}

/* Resize the buffer associated with a text element. */
int
AG_TextRealloc(AG_TextEnt *_Nonnull te, AG_Size maxLenNew)
{
	if (maxLenNew >= te->maxLen) {
		char *bufNew;

		bufNew = (char *)AG_TryRealloc(te->buf, maxLenNew);
		if (bufNew == NULL) {
			return (-1);
		}
		te->buf = bufNew;
		te->maxLen = maxLenNew;
	}
	return (0);
}

/* Concatenate the contents of the active text element against s. */
int
AG_TextCatS(AG_Text *_Nonnull txt, const char *_Nonnull s)
{
	AG_TextEnt *te;
	AG_Size len;
	
	len = strlen(s);

	AG_MutexLock(&txt->lock);
	te = &txt->ent[txt->lang];
	if (AG_TextRealloc(te, te->len + len + 1) == -1) {
		AG_MutexUnlock(&txt->lock);
		return (-1);
	}
	memcpy(&te->buf[te->len], s, len+1);
	te->len += len;
	AG_MutexUnlock(&txt->lock);
	return (0);
}

/* Release an autoallocated AG_Text element. */
void
AG_TextFree(AG_Text *_Nullable txt)
{
	if (txt == NULL) {
		return;
	}
	AG_TextDestroy(txt);
	AG_Free(txt);
}

#endif /* AG_UNICODE */
