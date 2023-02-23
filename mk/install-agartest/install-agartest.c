/*
 * Copyright (c) 2020-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Install a newly compiled Agar library on the Windows platform.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_DIR "C:\\Program Files\\Agar\\tests"
#define SRC_GUI   "..\\gui\\"
#define SRC_FONTS "..\\gui\\fonts\\"

char *dstArch = "x86";

const char *dataFiles[] = {
	"agartest.exe",
	"agar.png",
	"agar64.png",
	"agar-index.png",
	"agar.bmp",
	"agar-1.bmp",
	"agar-2.bmp",
	"agar-3.bmp",
	"agar-4.bmp",
	"axe.bmp",
	"axe.png",
	"glView.png",
	"helmet.png",
	"helmet.bmp",
	"helmet-socket.bmp",
	"loss.txt",
	"menubg.bmp",
	"pepe.jpg",
	"sword.bmp",
	"sword-socket.bmp",
	"sq-agar.bmp",
	"sq-agar.png",
	SRC_GUI "license.txt:license.txt",
	SRC_FONTS "agar-minimal-12.png:agar-minimal-12.png",
	SRC_FONTS "agar-minimal.agbf:agar-minimal.agbf",
	SRC_FONTS "agar-ideograms-12.png:agar-ideograms-12.png",
	SRC_FONTS "agar-ideograms.agbf:agar-ideograms.agbf",
	SRC_FONTS "algue.ttf:algue.ttf",
	SRC_FONTS "algue-bold-italic.ttf:algue-bold-italic.ttf",
	SRC_FONTS "algue-bold.ttf:algue-bold.ttf",
	SRC_FONTS "algue-italic.ttf:algue-italic.ttf",
	SRC_FONTS "charter-bold-italic.otf:charter-bold-italic.otf",
	SRC_FONTS "charter-bold.otf:charter-bold.otf",
	SRC_FONTS "charter-italic.otf:charter-italic.otf",
	SRC_FONTS "charter.otf:charter.otf",
	SRC_FONTS "fraktur.ttf:fraktur.ttf",
	SRC_FONTS "league-gothic-condensed-italic.otf:league-gothic-condensed-italic.otf",
	SRC_FONTS "league-gothic-condensed.otf:league-gothic-condensed.otf",
	SRC_FONTS "league-gothic.otf:league-gothic.otf",
	SRC_FONTS "league-spartan.otf:league-spartan.otf",
	SRC_FONTS "league-spartan-black.otf:league-spartan-black.otf",
	SRC_FONTS "league-spartan-bold.otf:league-spartan-bold.otf",
	SRC_FONTS "league-spartan-extrabold.otf:league-spartan-extrabold.otf",
	SRC_FONTS "league-spartan-extralight.otf:league-spartan-extralight.otf",
	SRC_FONTS "league-spartan-light.otf:league-spartan-light.otf",
	SRC_FONTS "league-spartan-semibold.otf:league-spartan-semibold.otf",
	SRC_FONTS "monoalgue.ttf:monoalgue.ttf",
	SRC_FONTS "monoalgue-bold-italic.ttf:monoalgue-bold-italic.ttf",
	SRC_FONTS "monoalgue-bold.ttf:monoalgue-bold.ttf",
	SRC_FONTS "monoalgue-italic.ttf:monoalgue-italic.ttf",
	SRC_FONTS "unialgue.ttf:unialgue.ttf",
	NULL
};

int
InstallLibs(const char *dir)
{
	char dest[1024];
	char *c, *dstFile;
	WIN32_FIND_DATA fdata;
	DWORD rv;
	HANDLE h;

	if ((h = FindFirstFile(".\\*", &fdata)) == INVALID_HANDLE_VALUE) {
		printf("Invalid file handle (%d)\n", GetLastError());
		return (-1);
	}
	while (FindNextFile(h, &fdata) != 0) {
		if ((dstFile = _strdup(fdata.cFileName)) == NULL) {
			printf("Out of memory\n");
			return (-1);
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
		sprintf_s(dest, sizeof(dest), "%s\\%s", dir, dstFile);
		printf("%s -> %s\n", fdata.cFileName, dest);
		if (CopyFile(fdata.cFileName, dest, 0) == 0) {
			printf("CopyFile(%s) failed\n", dest);
			return (-1);
		}
		free(dstFile);
	}
	rv = GetLastError();
	FindClose(h);
	if (rv != ERROR_NO_MORE_FILES) {
		printf("FindNextFile Error (%lu)\n", rv);
		return (-1);
	}
	return (0);
}

static char *
Prompt(const char *msg, const char *dflt)
{
	char buf[1024];

	printf("%s [%s]: ", msg, dflt);
	fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
	if (buf[0] == '\n') {
		return _strdup(dflt);
	} else {
		if (buf[strlen(buf)-1] == '\n') {
			buf[strlen(buf)-1] = '\0';
		}
		return _strdup(buf);
	}
}

static void
AnyKey(void)
{
	char buf[2];

	printf("Press any key to continue...");
	fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
}

static char *
Strsep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL) {
		return (NULL);
	}
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0) {
					s = NULL;
				} else {
					s[-1] = 0;
				}
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

int
main(int argc, char *argv[])
{
	char *dir = DEFAULT_DIR, *over;
	char bindir[2048], path[2048];
	const char **dataFile;
	DWORD attrs;

	printf("Agartest Installation\n");
	printf("=====================\n");

	if (argc > 2) {
		dstArch = argv[1];
		dir = argv[2];
	} else if (argc > 1) {
		dir = argv[1];
		dstArch = Prompt("Architecture (x86 or x64)", "x86");
	} else {
		dir = Prompt("Installation directory", DEFAULT_DIR);
		dstArch = Prompt("Architecture (x86 or x64)", "x86");
	}
	if (strcmp(dstArch, "x86") != 0 &&
	    strcmp(dstArch, "x64") != 0) {
		printf("No such architecture \"%s\" (use x86 or x64)\n",
		    dstArch);
		AnyKey();
		exit(1);
	}
	printf("Will install %s binaries into %s\n", dstArch, dir);

	if ((attrs = GetFileAttributes(dir)) != INVALID_FILE_ATTRIBUTES) {
		if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
			over = Prompt("Directory exists. Overwrite?", "no");
			if (over[0] != 'y' && over[0] != 'Y') {
				printf("Aborting installation.\n");
				AnyKey();
				exit(1);
			}
		} else {
			printf("Existing file: %s; remove first\n", dir);
			AnyKey();
			exit(1);
		}
	}

	printf("Installing Agartest into %s\n", dir);
	CreateDirectory(dir, NULL);

	sprintf_s(bindir, sizeof(bindir), "%s\\%s", dir, dstArch);
	CreateDirectory(bindir, NULL);

	if (InstallLibs(bindir) == -1) {
		printf("Failed to install libraries\n");
		exit(1);
	}

	for (dataFile = &dataFiles[0]; *dataFile != NULL; dataFile++) {
		if (strchr(*dataFile, ':') != NULL) {
			char *pfile = _strdup(*dataFile), *pfp = pfile;
			char *df_from = Strsep(&pfp, ":");
			char *df_to = Strsep(&pfp, ":");

			if (df_from != NULL && df_to != NULL) {
				sprintf_s(path, sizeof(path), "%s\\%s",
				    bindir, df_to);
				printf("%s\n", path);
				if (CopyFile(df_from, path, 0) == 0)
					printf("CopyFile(->%s) failed; ignoring\n", path);
			}

			free(pfile);
		} else {
			sprintf_s(path, sizeof(path), "%s\\%s", bindir, *dataFile);
			printf("%s\n", path);
			if (CopyFile(*dataFile, path, 0) == 0)
				printf("CopyFile(->%s) failed; ignoring\n", path);
		}
	}

	printf("\nAgartest was successfully installed under %s\n", bindir);
	AnyKey();

	exit(0);
}

