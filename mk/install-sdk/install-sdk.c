/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <windows.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	char *dir = "C:\\Program Files\\Agar";
	char subdir[1024];
	HANDLE h;
	WIN32_FIND_DATA fdata;
	DWORD rv;

	if (argc > 1) {
		dir = argv[1];
	}
	printf("Installing Agar SDK into %s\n", dir);

	CreateDirectory(dir, NULL);
	sprintf(subdir, "%s/include", dir);
	CreateDirectory(subdir, NULL);
	sprintf(subdir, "%s/lib", dir);
	CreateDirectory(subdir, NULL);
	sprintf(subdir, "%s/bin", dir);
	CreateDirectory(subdir, NULL);

	if ((h = FindFirstFile(".\\*", &fdata)) == INVALID_HANDLE_VALUE) {
		printf("Invalid file handle (%d)\n", GetLastError());
		exit(1);
	}
	while (FindNextFile(h, &fdata) != 0) {
		char *dstFile, *c;
		char dest[1024];

		if ((dstFile = strdup(fdata.cFileName)) == NULL) {
			printf("Out of memory\n");
			exit(1);
		}
		if ((c = strrchr(dstFile, '_')) != NULL &&
		    strcmp(c, "_static.lib") == 0) {
			c[0] = '.'; c[1] = 'l'; c[2] = 'i'; c[3] = 'b';
			c[4] = '\0';
		} else if ((c = strrchr(dstFile, '.')) != NULL &&
		    strcmp(c, ".dll") == 0) {
		} else {
			free(dstFile);
			continue;
		}
		sprintf(dest, "%s/lib/%s", dir, dstFile);
		printf("%s -> %s\n", fdata.cFileName, dest);
		if (CopyFile(fdata.cFileName, dest, 0) == 0) {
			printf("%s: copy failed\n", dest);
			exit(1);
		}
		free(dstFile);
	}
	rv = GetLastError();
	FindClose(h);
	if (rv != ERROR_NO_MORE_FILES) {
		printf("FindNextFile Error (%lu)\n", rv);
		exit(1);
	}
	exit(0);
}

