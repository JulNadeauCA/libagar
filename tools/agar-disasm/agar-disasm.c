/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/core/types.h>
#include <agar/config/have_64bit.h>
#include <agar/config/have_float.h>
#include <agar/config/have_long_double.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

const struct {
	enum ag_data_source_type type;
	const char *name;
} types[] = {
	{ AG_SOURCE_UINT8,		"Uint8" },
	{ AG_SOURCE_SINT8,		"Sint8" },
	{ AG_SOURCE_UINT16,		"Uint16" },
	{ AG_SOURCE_SINT16,		"Sint16" },
	{ AG_SOURCE_UINT32,		"Uint32" },
	{ AG_SOURCE_SINT32,		"Sint32" },
	{ AG_SOURCE_UINT64,		"Uint64" },
	{ AG_SOURCE_SINT64,		"Sint64" },
	{ AG_SOURCE_FLOAT,		"Float" },
	{ AG_SOURCE_DOUBLE,		"Double" },
	{ AG_SOURCE_STRING,		"String" },
	{ AG_SOURCE_COLOR_RGBA,		"ColorRGBA" },
	{ AG_SOURCE_STRING_PAD,		"StringPad" },
};
const int nTypes = sizeof(types) / sizeof(types[0]);

static void
printusage(void)
{
	fprintf(stderr, "Usage: agar-disasm [filename]\n");
}

#define FORWARD(n) \
	if (p+(n) > &buf[len]) { \
		goto out; \
	} \
	p += (n)

int
main(int argc, char *argv[])
{
	char *file, *s;
	extern char *optarg;
	extern int optind;
	int i, c;
	AG_DataSource *ds = NULL;
	AG_ObjectHeader oh;
	Uint32 pos, sLen;
	size_t len;
	Uint8 *buf, *p;
	int asis = 0;

	if (AG_InitCore("agar-disasm", 0) == -1) {
		return (0);
	}
	while ((c = getopt(argc, argv, "?")) != -1) {
		switch (c) {
		case '?':
		default:
			printusage();
			return (1);
		}
	}
	if (argc < 2) {
		printusage();
		return (1);
	}
	file = argv[1];

	if ((ds = AG_OpenFile(file, "r")) == NULL) {
		goto fail;
	}
	if (AG_ObjectReadHeader(ds, &oh) == -1) {
		goto fail;
	}
	printf("Class:\t\t%s\n", oh.cs.hier);
	if (oh.cs.libs[0] != '\0') {
		printf("Modules:\t%s\n", oh.cs.libs);
	}
	printf("Dataset at:\t%lx\n", (unsigned long)oh.dataOffs);
	printf("Version:\t%u.%u\n", oh.ver.major, oh.ver.minor);
	printf("Flags:\t\t0x%x\n", oh.flags);
	printf("\n");

	if (AG_Seek(ds, 0, AG_SEEK_END) == -1) {
		goto fail;
	}
	len = AG_Tell(ds) - oh.dataOffs;
	if (AG_Seek(ds, (off_t)oh.dataOffs, AG_SEEK_SET) == -1) {
		goto fail;
	}
	pos = 0;

	if ((buf = malloc(len)) == NULL) {
		AG_SetError("Out of memory for dataset (%lu)",
		    (unsigned long)len);
		goto fail;
	}
	if (AG_Read(ds, buf, len) == -1)
		goto fail;

	for (p = buf; ;) {
		for (i = 0; i < nTypes; i++) {
			if (AG_SwapBE32(*(Uint32 *)p) == types[i].type) {
				if (asis) {
					printf("\n");
				}
				asis = 0;
				break;
			}
		}
		if (i == nTypes) {
			if (!asis) {
				printf("[%8s] ", "???");
				asis = 1;
			}
			if (isprint(*(Uint8 *)p)) {
				fputc(*(Uint8 *)p, stdout);
			} else {
				printf("\\%x", *(Uint8 *)p);
			}
			FORWARD(1);
			continue;
		}
		FORWARD(4);

		printf("[%8s] ", types[i].name);
		switch (types[i].type) {
		case AG_SOURCE_UINT8:
			printf("%u\n", *(Uint8 *)p);
			FORWARD(1);
			break;
		case AG_SOURCE_SINT8:
			printf("%d\n", *(Sint8 *)p);
			FORWARD(1);
			break;
		case AG_SOURCE_UINT16:
			printf("%u\n", AG_SwapBE16(*(Uint16 *)p));
			FORWARD(2);
			break;
		case AG_SOURCE_SINT16:
			printf("%d\n", (Sint16)AG_SwapBE16(*(Uint16 *)p));
			FORWARD(2);
			break;
		case AG_SOURCE_UINT32:
			printf("%u\n", AG_SwapBE32(*(Uint32 *)p));
			FORWARD(2);
			break;
		case AG_SOURCE_SINT32:
			printf("%u\n", AG_SwapBE32(*(Sint32 *)p));
			FORWARD(2);
			break;
#ifdef HAVE_64BIT
		case AG_SOURCE_UINT64:
			printf("%llu\n",
			    (unsigned long long)AG_SwapBE64(*(Uint64 *)p));
			FORWARD(8);
			break;
		case AG_SOURCE_SINT64:
			printf("%lld\n", (long long)AG_SwapBE64(*(Uint64 *)p));
			FORWARD(8);
			break;
#endif
#ifdef HAVE_FLOAT
		case AG_SOURCE_FLOAT:
			printf("%f\n", *(float *)p);
			FORWARD(4);
			break;
		case AG_SOURCE_DOUBLE:
			printf("%f\n", *(double *)p);
			FORWARD(8);
			break;
#endif
		case AG_SOURCE_STRING:
			if (AG_SwapBE32(*(Uint32 *)p) != AG_SOURCE_UINT32) {
				printf("Bad string length!\n");
				break;
			}
			FORWARD(4);
			sLen = AG_SwapBE32(*(Uint32 *)p);
			FORWARD(4);
			if ((s = malloc(sLen+1)) == NULL) {
				printf("Out of memory for string!\n");
				break;
			}
			memcpy(s, p, sLen);
			s[sLen] = '\0';
			printf("\"%s\"\n", s);
			free(s);
			FORWARD(sLen);
			break;
		case AG_SOURCE_STRING_PAD:
			/* TODO */
			break;
		default:
			printf("(skipping)\n");
			break;
		}
	}
out:
	AG_CloseFile(ds);
	return (0);
fail:
	fprintf(stderr, "%s\n", AG_GetError());
	if (ds != NULL) { AG_CloseFile(ds); }
	AG_Destroy();
	return (1);
}

