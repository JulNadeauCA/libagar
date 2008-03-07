/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_progname.h>

#include <core/core.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <core/load_den.h>

int verbose = 0;
char *outfile = NULL;
char *hint = "unk";
char *name = "Untitled";
char *lang = "en";
char *author = "";
char *copy = "";
char *descr = "";
char *keyw = "";

static void
printusage(void)
{
#ifdef HAVE_PROGNAME
	extern char *__progname;
	char *progname = __progname;
#else
	char *progname = "dencomp";
#endif
	fprintf(stderr, "Usage: %s [-q] [-o outfile] [-h hint]"
	    " [-n name] [-l lang] [-a author] [-c copyright] [-d descr] "
	    " [-k keywords] file1 file2:lang ...\n", progname);
}

int
main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	AG_Den *den;
	int c, i;

	AG_InitError();

	while ((c = getopt(argc, argv, "?vo:h:n:l:a:c:d:k:")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'h':
			hint = optarg;
			break;
		case 'n':
			name = optarg;
			break;
		case 'l':
			lang = optarg;
			break;
		case 'a':
			author = optarg;
			break;
		case 'c':
			copy = optarg;
			break;
		case 'd':
			descr = optarg;
			break;
		case 'k':
			keyw = optarg;
			break;
		case '?':
		default:
			printusage();
			goto fail1;
		}
	}
	argc -= optind;
	argv += optind;

	if (outfile == NULL) {
		fprintf(stderr, "No output file (-o) was specified\n");
		printusage();
		goto fail1;
	}

	den = AG_DenOpen(outfile, AG_DEN_WRITE);
	if (den == NULL) {
		fprintf(stderr, "opening %s: %s\n", outfile, AG_GetError());
		goto fail1;
	}

	if (verbose) {
		printf("Encoding `%s':\n", name);
		printf("\tHint:      %s\n", hint);
		printf("\tLanguage:  %s\n", lang);

		if (author[0] != '\0')	printf("\tAuthor:    %s\n", author);
		if (copy[0] != '\0')	printf("\tCopyright: %s\n", copy);
		if (descr[0] != '\0')	printf("\tDescr:     %s\n", descr);
		if (keyw[0] != '\0')	printf("\tKeywords:  %s\n", keyw);
		printf("\n");
	}

	Strlcpy(den->hint, hint, sizeof(den->hint));
	Strlcpy(den->name, name, sizeof(den->name));
	den->author = strdup(author);
	den->copyright = strdup(copy);
	den->descr = strdup(descr);
	den->keywords = strdup(keyw);

	AG_DenWriteHeader(den, argc);
	for (i = 0; i < argc; i++) {
		char *s, *fspec = argv[i];
		char *name = NULL, *langspec = NULL;

		if ((s = AG_Strsep(&fspec, ":")) != NULL && s[0] != '\0')
			name = s;
		if ((s = AG_Strsep(&fspec, ":")) != NULL && s[0] != '\0')
			langspec = s;
		if (langspec == NULL)
			langspec = lang;

		printf("+ %s (%s)\n", name, langspec);
		if (AG_DenImportFile(den, i, name, langspec, argv[i]) == -1) {
			fprintf(stderr, "Import of `%s' failed\n", argv[i]);
			goto fail2;
		}
	}
	AG_DenWriteMappings(den);
	AG_DenClose(den);
	AG_DestroyError();
	return (0);
fail2:
	AG_DenClose(den);
fail1:
	AG_DestroyError();
	return (1);
}

