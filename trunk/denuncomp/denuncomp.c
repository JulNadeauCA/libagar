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

#include <agar/core.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


int verbose = 1;
int iflag = 0;
char *outdir = ".";

static void
printusage(void)
{
	fprintf(stderr, "Usage: denuncomp [-qi] [-o outdir] archive.den\n");
}

int
main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	AG_Den *den;
	int c, i;

	AG_InitError();

	while ((c = getopt(argc, argv, "?qio:")) != -1) {
		switch (c) {
		case 'q':
			verbose = 0;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'o':
			outdir = optarg;
			break;
		case '?':
		default:
			printusage();
			goto fail;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		fprintf(stderr, "No den(5) file was given.\n");
		printusage();
		goto fail;
	}

	den = AG_DenOpen(argv[0], AG_DEN_READ);
	if (den == NULL) {
		fprintf(stderr, "opening %s: %s\n", argv[0], AG_GetError());
		goto fail;
	}

	if (verbose) {
		printf("Extracting from `%s':\n", den->name);
		printf("\tHint:      `%s'\n", den->hint);
		printf("\tAuthor:    `%s'\n", den->author);
		printf("\tCopyright: `%s'\n", den->copyright);

		if (den->descr[0] != '\0')
			printf("\tDescr:     `%s'\n", den->descr);
		if (den->keywords[0] != '\0')
			printf("\tKeywords:  `%s'\n", den->keywords);
	}
	if (iflag) {
		goto out;
	}

	if (chdir(outdir) == -1) {
		fprintf(stderr, "%s: %s\n", outdir, strerror(errno));
		goto fail;
	}

	for (i = 0; i < den->nmembers; i++) {
		char *buf;
		AG_DenMember *memb = &den->members[i];
		FILE *f;
		size_t i, len, rv;

		for (i = 0, len = strlen(memb->name);
		     i < len;
		     i++) {
			if (memb->name[i] == '/')
				memb->name[i] = '_';
		}

		printf("- %24s [%4s] (0x%04x+0x%04x)\n", memb->name,
		    memb->lang, (unsigned)memb->offs, (unsigned)memb->size);

		if ((f = fopen(memb->name, "wb")) == NULL) {
			fprintf(stderr, "%s: %s\n", memb->name,
			    strerror(errno));
			goto fail;
		}
		
		buf = AG_Malloc(memb->size);
		AG_Seek(den->buf, memb->offs, AG_SEEK_SET);
		AG_Read(den->buf, buf, 1, memb->size);
		if (den->buf->rdLast > 0) {
			if (fwrite(buf, rv, 1, f) < 1) {
				fprintf(stderr, "%s: write error\n",
				    memb->name);
				goto fail;
			}
		}
		fclose(f);
		AG_Free(buf);
	}
out:
	AG_DenClose(den);
	AG_DestroyError();
	return (0);
fail:
	AG_DestroyError();
	return (1);
}

