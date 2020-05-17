/*	Public domain	*/

/*
 * C-string related routines.
 */

#ifdef AG_UNICODE
/*
 * Return the length of a UCS-4 string in characters (not including NUL).
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Size _Pure_Attribute
AG_LengthUCS4(const AG_Char *_Nonnull ucs)
#else
AG_Size
ag_length_ucs4(const AG_Char *ucs)
#endif
{
	AG_Size len;

	for (len = 0; *ucs != '\0'; ucs++) {
		len++;
	}
	return (len);
}

/*
 * Compute # bytes that would be needed to encode an UCS-4 char in UTF-8.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Const_Attribute
AG_CharLengthUTF8FromUCS4(AG_Char ch)
#else
int
ag_char_length_utf8_from_ucs4(AG_Char ch)
#endif
{
	if      (ch <  0x80)		{ return (1); }
	else if (ch <  0x800)		{ return (2); }
	else if (ch <  0x10000)		{ return (3); }
	else if (ch <  0x200000)	{ return (4); }
	else if (ch <  0x4000000)	{ return (5); }
	else if (ch <= 0x7fffffff)	{ return (6); }

	return (-1);
}

/*
 * Compute # bytes that would be needed to encode an UCS-4 string in UTF-8.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int
AG_LengthUTF8FromUCS4(const AG_Char *_Nonnull ucs4, AG_Size *_Nonnull rv)
#else
int
ag_length_utf8_from_ucs4(const AG_Char *_Nonnull ucs4, AG_Size *_Nonnull rv)
#endif
{
	const AG_Char *c;
	int cLen;

	*rv = 0;
	for (c = &ucs4[0]; *c != '\0'; c++) {
		if ((cLen = AG_CharLengthUTF8FromUCS4(*c)) == -1) {
			return (-1);
		}
		(*rv) += cLen;
	}
	return (0);
}

/*
 * Parse the first byte of a possible UTF-8 sequence and return the length
 * of the sequence in bytes. Return 1 if there is no sequence.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Const_Attribute
AG_CharLengthUTF8(unsigned char ch)
#else
int
ag_char_length_utf8(unsigned char ch)
#endif
{
	int rv;

	if ((ch >> 7) == 0) {
		rv = 1;
	} else if (((ch & 0xe0) >> 5) == 0x6) {
		rv = 2;
	} else if (((ch & 0xf0) >> 4) == 0xe) {
		rv = 3;
	} else if (((ch & 0xf8) >> 3) == 0x1e) {
		rv = 4;
	} else if (((ch & 0xfc) >> 2) == 0x3e) {
		rv = 5;
	} else if (((ch & 0xfe) >> 1) == 0x7e) {
		rv = 6;
	} else {
		return (1);
	}
	return (rv);
}

/*
 * Return the number of characters in the given UTF-8 string, not counting
 * the terminating NUL. If the string is invalid, fail and return -1.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Size _Pure_Attribute
AG_LengthUTF8(const char *_Nonnull s)
#else
AG_Size
ag_length_utf8(const char *s)
#endif
{
	const char *c = &s[0];
	int i, cLen;
	AG_Size rv = 0;

	if (s[0] == '\0') {
		return (0);
	}
	for (;;) {
		cLen = AG_CharLengthUTF8((unsigned char)*c);
		for (i = 0; i < cLen; i++) {
			if (c[i] == '\0')
				return (rv);
		}
		rv++;
		c += cLen;
	}
	return (rv);
}
#endif /* AG_UNICODE */

/*
 * Compare two strings ignoring case.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_Strcasecmp(const char *_Nonnull s1, const char *_Nonnull s2)
#else
int
ag_strcasecmp(const char *s1, const char *s2)
#endif
{
	const unsigned char *cm = agStrcasecmpMapASCII;
	const unsigned char *us1 = (const unsigned char *)s1;
	const unsigned char *us2 = (const unsigned char *)s2;

	while (cm[*us1] == cm[*us2++]) {
		if (*us1++ == '\0')
			return (0);
	}
	return (cm[*us1] - cm[*--us2]);
}

/*
 * Compare the first n characters of two strings ignoring case.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_Strncasecmp(const char *_Nonnull s1, const char *_Nonnull s2, AG_Size n)
#else
int
ag_strncasecmp(const char *s1, const char *s2, AG_Size n)
#endif
{
	const unsigned char *cm = agStrcasecmpMapASCII;
	const unsigned char *us1 = (const unsigned char *)s1;
	const unsigned char *us2 = (const unsigned char *)s2;
	AG_Size i;

	for (i = 0; i < n; i++) {
		if (cm[us1[i]] != cm[us2[i]])
			break;
	}
	return i == n ? 0 : cm[us1[i]] - cm[us2[i]];
}
