/*	$Csoft$	*/
/*	$NetBSD: gettext.c,v 1.9 2001/02/16 07:20:35 minoura Exp $	*/

/*-
 * Copyright (c) 2000, 2001 Citrus Project,
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config/localedir.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#if 0
#include <util.h>
#endif
#include "libintl.h"
#include <locale.h>
#include "libintl_local.h"

static const char *lookup_category(int);
static const char *split_locale(const char *);
static const char *lookup_mofile(char *, size_t, const char *,
	char *, const char *, const char *, struct domainbinding *);
static u_int32_t flip(u_int32_t, u_int32_t);
static int validate(void *, struct mohandle *);
static int mapit(const char *, struct domainbinding *);
static int unmapit(struct domainbinding *);
static const char *lookup_hash(const char *, struct domainbinding *);
static const char *lookup_bsearch(const char *, struct domainbinding *);
static const char *lookup(const char *, struct domainbinding *);

/*
 * shortcut functions.  the main implementation resides in dcngettext().
 */
char *
gettext(msgid)
	const char *msgid;
{

	return dcngettext(NULL, msgid, NULL, 1UL, LC_MESSAGES);
}

char *
dgettext(domainname, msgid)
	const char *domainname;
	const char *msgid;
{

	return dcngettext(domainname, msgid, NULL, 1UL, LC_MESSAGES);
}

char *
dcgettext(domainname, msgid, category)
	const char *domainname;
	const char *msgid;
	int category;
{

	return dcngettext(domainname, msgid, NULL, 1UL, category);
}

char *
ngettext(msgid1, msgid2, n)
	const char *msgid1;
	const char *msgid2;
	unsigned long int n;
{

	return dcngettext(NULL, msgid1, msgid2, n, LC_MESSAGES);
}

char *
dngettext(domainname, msgid1, msgid2, n)
	const char *domainname;
	const char *msgid1;
	const char *msgid2;
	unsigned long int n;
{

	return dcngettext(domainname, msgid1, msgid2, n, LC_MESSAGES);
}

/*
 * dcngettext() -
 * lookup internationalized message on database locale/category/domainname
 * (like ja_JP.eucJP/LC_MESSAGES/domainname).
 * if n equals to 1, internationalized message will be looked up for msgid1.
 * otherwise, message will be looked up for msgid2.
 * if the lookup fails, the function will return msgid1 or msgid2 as is.
 *
 * Even though the return type is "char *", caller should not rewrite the
 * region pointed to by the return value (should be "const char *", but can't
 * change it for compatibility with other implementations).
 *
 * by default (if domainname == NULL), domainname is taken from the value set
 * by textdomain().  usually name of the application (like "ls") is used as
 * domainname.  category is usually LC_MESSAGES.
 *
 * the code reads in *.mo files generated by GNU gettext.  *.mo is a host-
 * endian encoded file.  both endians are supported here, as the files are in
 * /usr/share/locale! (or we should move those files into /usr/libdata)
 */

static const char *
lookup_category(category)
	int category;
{

	switch (category) {
	case LC_COLLATE:	return "LC_COLLATE";
	case LC_CTYPE:		return "LC_CTYPE";
	case LC_MONETARY:	return "LC_MONETARY";
	case LC_NUMERIC:	return "LC_NUMERIC";
	case LC_TIME:		return "LC_TIME";
	case LC_MESSAGES:	return "LC_MESSAGES";
	}
	return NULL;
}

/*
 * XPG syntax: language[_territory[.codeset]][@modifier]
 * XXX boundary check on "result" is lacking
 */
static const char *
split_locale(lname)
	const char *lname;
{
	char buf[BUFSIZ], tmp[BUFSIZ];
	char *l, *t, *c, *m;
	static char result[BUFSIZ];

	memset(result, 0, sizeof(result));

	if (strlen(lname) + 1 > sizeof(buf)) {
fail:
		return lname;
	}

	strlcpy(buf, lname, sizeof(buf));
	m = strrchr(buf, '@');
	if (m)
		*m++ = '\0';
	c = strrchr(buf, '.');
	if (c)
		*c++ = '\0';
	t = strrchr(buf, '_');
	if (t)
		*t++ = '\0';
	l = buf;
	if (strlen(l) == 0)
		goto fail;
	if (c && !t)
		goto fail;

	if (m) {
		if (t) {
			if (c) {
				snprintf(tmp, sizeof(tmp), "%s_%s.%s@%s",
				    l, t, c, m); 
				strlcat(result, tmp, sizeof(result));
				strlcat(result, ":", sizeof(result));
			}
			snprintf(tmp, sizeof(tmp), "%s_%s@%s", l, t, m); 
			strlcat(result, tmp, sizeof(result));
			strlcat(result, ":", sizeof(result));
		}
		snprintf(tmp, sizeof(tmp), "%s@%s", l, m); 
		strlcat(result, tmp, sizeof(result));
		strlcat(result, ":", sizeof(result));
	}
	if (t) {
		if (c) {
			snprintf(tmp, sizeof(tmp), "%s_%s.%s", l, t, c); 
			strlcat(result, tmp, sizeof(result));
			strlcat(result, ":", sizeof(result));
		}
		strlcat(result, tmp, sizeof(result));
		strlcat(result, ":", sizeof(result));
	}
	strlcat(result, l, sizeof(result));

	return result;
}

static const char *
lookup_mofile(buf, len, dir, lpath, category, domainname, db)
	char *buf;
	size_t len;
	const char *dir;
	char *lpath;	/* list of locales to be tried */
	const char *category;
	const char *domainname;
	struct domainbinding *db;
{
	struct stat st;
	char *p, *q;

	q = lpath;
	/* CONSTCOND */
	while (1) {
		p = strsep(&q, ":");
		if (!p)
			break;
		if (!*p)
			continue;

		/* don't mess with default locales */
		if (strcmp(p, "C") == 0 || strcmp(p, "POSIX") == 0)
			return NULL;

		/* validate pathname */
		if (strchr(p, '/') || strchr(category, '/'))
			continue;
#if 1	/*?*/
		if (strchr(domainname, '/'))
			continue;
#endif

		snprintf(buf, len, "%s/%s/%s/%s.mo", dir, p,
		    category, domainname);
		if (stat(buf, &st) < 0)
			continue;
		if ((st.st_mode & S_IFMT) != S_IFREG)
			continue;

		if (mapit(buf, db) == 0)
			return buf;
	}

	return NULL;
}

static u_int32_t
flip(v, magic)
	u_int32_t v;
	u_int32_t magic;
{

	if (magic == MO_MAGIC)
		return v;
	else if (magic == MO_MAGIC_SWAPPED) {
		v = ((v >> 24) & 0xff) | ((v >> 8) & 0xff00) |
		    ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000);
		return v;
	} else {
		abort();
		/*NOTREACHED*/
	}
}

static int
validate(arg, mohandle)
	void *arg;
	struct mohandle *mohandle;
{
	char *p;

	p = (char *)arg;
	if (p < (char *)mohandle->addr ||
	    p > (char *)mohandle->addr + mohandle->len)
		return 0;
	else
		return 1;
}

int
mapit(path, db)
	const char *path;
	struct domainbinding *db;
{
	int fd;
	struct stat st;
	char *base;
	u_int32_t magic, revision;
	struct moentry *otable, *ttable;
	struct moentry_h *p;
	struct mo *mo;
	size_t l;
	int i;
	char *v;
	struct mohandle *mohandle = &db->mohandle;

	if (mohandle->addr && mohandle->addr != MAP_FAILED &&
	    mohandle->mo.mo_magic)
		return 0;	/*already opened*/

	unmapit(db);

#if 0
	if (secure_path(path) != 0)
		goto fail;
#endif
	if (stat(path, &st) < 0)
		goto fail;
	if ((st.st_mode & S_IFMT) != S_IFREG || st.st_size > GETTEXT_MMAP_MAX)
		goto fail;
	fd = open(path, O_RDONLY);
	if (fd < 0)
		goto fail;
	if (read(fd, &magic, sizeof(magic)) != sizeof(magic) ||
	    (magic != MO_MAGIC && magic != MO_MAGIC_SWAPPED)) {
		close(fd);
		goto fail;
	}
	if (read(fd, &revision, sizeof(revision)) != sizeof(revision) ||
	    flip(revision, magic) != MO_REVISION) {
		close(fd);
		goto fail;
	}
	mohandle->addr = mmap(NULL, (size_t)st.st_size, PROT_READ,
	    MAP_FILE | MAP_SHARED, fd, (off_t)0);
	if (!mohandle->addr || mohandle->addr == MAP_FAILED) {
		close(fd);
		goto fail;
	}
	close(fd);
	mohandle->len = (size_t)st.st_size;

	base = mohandle->addr;
	mo = (struct mo *)mohandle->addr;

	/* flip endian.  do not flip magic number! */
	mohandle->mo.mo_magic = mo->mo_magic;
	mohandle->mo.mo_revision = flip(mo->mo_revision, magic);
	mohandle->mo.mo_nstring = flip(mo->mo_nstring, magic);

	/* validate otable/ttable */
	otable = (struct moentry *)(base + flip(mo->mo_otable, magic));
	ttable = (struct moentry *)(base + flip(mo->mo_ttable, magic));
	if (!validate(otable, mohandle) ||
	    !validate(&otable[mohandle->mo.mo_nstring], mohandle)) {
		unmapit(db);
		goto fail;
	}
	if (!validate(ttable, mohandle) ||
	    !validate(&ttable[mohandle->mo.mo_nstring], mohandle)) {
		unmapit(db);
		goto fail;
	}

	/* allocate [ot]table, and convert to normal pointer representation. */
	l = sizeof(struct moentry_h) * mohandle->mo.mo_nstring;
	mohandle->mo.mo_otable = (struct moentry_h *)malloc(l);
	if (!mohandle->mo.mo_otable) {
		unmapit(db);
		goto fail;
	}
	mohandle->mo.mo_ttable = (struct moentry_h *)malloc(l);
	if (!mohandle->mo.mo_ttable) {
		unmapit(db);
		goto fail;
	}
	p = mohandle->mo.mo_otable;
	for (i = 0; i < mohandle->mo.mo_nstring; i++) {
		p[i].len = flip(otable[i].len, magic);
		p[i].off = base + flip(otable[i].off, magic);

		if (!validate(p[i].off, mohandle) ||
		    !validate(p[i].off + p[i].len + 1, mohandle)) {
			unmapit(db);
			goto fail;
		}
	}
	p = mohandle->mo.mo_ttable;
	for (i = 0; i < mohandle->mo.mo_nstring; i++) {
		p[i].len = flip(ttable[i].len, magic);
		p[i].off = base + flip(ttable[i].off, magic);

		if (!validate(p[i].off, mohandle) ||
		    !validate(p[i].off + p[i].len + 1, mohandle)) {
			unmapit(db);
			goto fail;
		}
	}

	/* grab MIME-header and charset field */
	mohandle->mo.mo_header = lookup("", db);
	if (mohandle->mo.mo_header)
		v = strstr(mohandle->mo.mo_header, "charset=");
	else
		v = NULL;
	if (v) {
		mohandle->mo.mo_charset = strdup(v + 8);
		if (!mohandle->mo.mo_charset)
			goto fail;
		v = strchr(mohandle->mo.mo_charset, '\n');
		if (v)
			*v = '\0';
	}

	/*
	 * XXX check charset, reject it if we are unable to support the charset
	 * with the current locale.
	 * for example, if we are using euc-jp locale and we are looking at
	 * *.mo file encoded by euc-kr (charset=euc-kr), we should reject
	 * the *.mo file as we cannot support it.
	 */

	return 0;

fail:
	return -1;
}

static int
unmapit(db)
	struct domainbinding *db;
{
	struct mohandle *mohandle = &db->mohandle;

	/* unmap if there's already mapped region */
	if (mohandle->addr && mohandle->addr != MAP_FAILED)
		munmap(mohandle->addr, mohandle->len);
	mohandle->addr = NULL;
	if (mohandle->mo.mo_otable)
		free(mohandle->mo.mo_otable);
	if (mohandle->mo.mo_ttable)
		free(mohandle->mo.mo_ttable);
	if (mohandle->mo.mo_charset)
		free(mohandle->mo.mo_charset);
	memset(&mohandle->mo, 0, sizeof(mohandle->mo));
	return 0;
}

/* ARGSUSED */
static const char *
lookup_hash(msgid, db)
	const char *msgid;
	struct domainbinding *db;
{

	/*
	 * XXX should try a hashed lookup here, but to do so, we need to
	 * look inside the GPL'ed *.c and re-implement...
	 */
	return NULL;
}

static const char *
lookup_bsearch(msgid, db)
	const char *msgid;
	struct domainbinding *db;
{
	int top, bottom, middle, omiddle;
	int n;
	struct mohandle *mohandle = &db->mohandle;

	top = 0;
	bottom = mohandle->mo.mo_nstring;
	omiddle = -1;
	/* CONSTCOND */
	while (1) {
		if (top > bottom)
			break;
		middle = (top + bottom) / 2;
		/* avoid possible infinite loop, when the data is not sorted */
		if (omiddle == middle)
			break;
		if (middle < 0 || middle >= mohandle->mo.mo_nstring)
			break;

		n = strcmp(msgid, mohandle->mo.mo_otable[middle].off);
		if (n == 0)
			return (const char *)mohandle->mo.mo_ttable[middle].off;
		else if (n < 0)
			bottom = middle;
		else
			top = middle;
		omiddle = middle;
	}

	return NULL;
}

static const char *
lookup(msgid, db)
	const char *msgid;
	struct domainbinding *db;
{
	const char *v;

	v = lookup_hash(msgid, db);
	if (v)
		return v;

	return lookup_bsearch(msgid, db);
}

char *
dcngettext(domainname, msgid1, msgid2, n, category)
	const char *domainname;
	const char *msgid1;
	const char *msgid2;
	unsigned long int n;
	int category;
{
	const char *msgid;
	char path[PATH_MAX];
	static char lpath[PATH_MAX];
	static char olpath[PATH_MAX];
	const char *locale;
	const char *language;
	const char *cname = NULL;
	const char *v;
	static char *ocname = NULL;
	static char *odomainname = NULL;
	struct domainbinding *db;

	msgid = (n == 1) ? msgid1 : msgid2;
	if (msgid == NULL)
		return NULL;

	if (!domainname)
		domainname = __current_domainname;
	cname = lookup_category(category);
	if (!domainname || !cname)
		goto fail;

	language = getenv("LANGUAGE");
	locale = setlocale(LC_MESSAGES, NULL);	/*XXX*/
	if (locale)
		locale = split_locale(locale);
	if (language && locale) {
		if (strlen(language) + strlen(locale) + 2 > sizeof(lpath))
			goto fail;
		snprintf(lpath, sizeof(lpath), "%s:%s", language, locale);
	} else if (language) {
		if (strlen(language) + 1 > sizeof(lpath))
			goto fail;
		strlcpy(lpath, language, sizeof(lpath));
	} else if (locale) {
		if (strlen(locale) + 1 > sizeof(lpath))
			goto fail;
		strlcpy(lpath, locale, sizeof(lpath));
	} else
		goto fail;

	for (db = __bindings; db; db = db->next)
		if (strcmp(db->domainname, domainname) == 0)
			break;
	if (!db) {
		if (!bindtextdomain(domainname, LOCALEDIR))
			goto fail;
		db = __bindings;
	}

	/* don't bother looking it up if the values are the same */
	if (odomainname && strcmp(domainname, odomainname) == 0 &&
	    ocname && strcmp(cname, ocname) == 0 && strcmp(lpath, olpath) == 0 &&
	    db->mohandle.mo.mo_magic)
		goto found;

	/* try to find appropriate file, from $LANGUAGE */
	if (lookup_mofile(path, sizeof(path), db->path, lpath, cname,
	    domainname, db) == NULL)
		goto fail;

	if (odomainname)
		free(odomainname);
	if (ocname)
		free(ocname);
	odomainname = strdup(domainname);
	ocname = strdup(cname);
	if (!odomainname || !ocname) {
		if (odomainname)
			free(odomainname);
		if (ocname)
			free(ocname);
		odomainname = ocname = NULL;
		goto fail;
	}

	strlcpy(olpath, lpath, sizeof(olpath));

found:
	v = lookup(msgid, db);
	if (v) {
		/*
		 * XXX call iconv() here, if translated text is encoded
		 * differently from currently-selected encoding (locale).
		 * look at Content-type header in *.mo file, in string obtained
		 * by gettext("").
		 */

		/*
		 * Given the amount of printf-format security issues, it may
		 * be a good idea to validate if the original msgid and the
		 * translated message format string carry the same printf-like
		 * format identifiers.
		 */

		msgid = v;
	}

fail:
	/* LINTED const cast */
	return (char *)msgid;
}
