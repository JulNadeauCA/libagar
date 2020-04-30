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

/*
 * Install a newly compiled Agar library on the Windows platform.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_DIR "C:\\Program Files\\Agar"

char *dstArch = "x86";

const char *miscFiles[] = {
	"CHANGELOG.txt",
	"INSTALL.txt",
	"LICENSE.txt",
	"README.txt",
	"Logo.png",
	"VisualC.html"
};
const int nMiscFiles = sizeof(miscFiles)/sizeof(miscFiles[0]);

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
		sprintf_s(dest, sizeof(dest), "%s\\lib\\%s\\%s", dir,
		    dstArch, dstFile);
		printf("%s -> %s\n", fdata.cFileName, dest);
		if (CopyFile(fdata.cFileName, dest, 0) == 0) {
			printf("%s: CopyFile() failed\n", dest);
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

int
InstallIncludes(const char *srcDir, const char *dstDir)
{
	char path[1024], dest[1024], filename[1024];
	char *c;
	WIN32_FIND_DATA fdata;
	DWORD attrs, rv;
	HANDLE h;
	
	CreateDirectory(dstDir, NULL);
	printf("> %s\n", srcDir);
	sprintf_s(path, sizeof(path), "%s\\*", srcDir);
	if ((h = FindFirstFile(path, &fdata)) == INVALID_HANDLE_VALUE) {
		printf("Invalid file handle (%d)\n", GetLastError());
		return (-1);
	}
	while (FindNextFile(h, &fdata) != 0) {
		if (fdata.cFileName[0] == '.') {
			continue;
		}
		if (strcmp(srcDir, ".") == 0) {
			sprintf_s(path, sizeof(path), "%s", fdata.cFileName);
		} else {
			sprintf_s(path, sizeof(path), "%s\\%s", srcDir,
			    fdata.cFileName);
		}
		if ((attrs = GetFileAttributes(path)) ==
		    INVALID_FILE_ATTRIBUTES) {
			printf("GetFileAttributes(%s) failed\n", path);
			continue;
		}
		if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
			sprintf_s(dest, sizeof(dest), "%s\\%s", dstDir,
			    fdata.cFileName);
			if (InstallIncludes(path, dest) == -1) {
				return (-1);
			}
			continue;
		}
		if ((c = strrchr(path, '.')) == NULL ||
		    c[1] != 'h' || c[2] != '\0') {
			continue;
		}

		sprintf_s(filename, sizeof(filename), "%s", fdata.cFileName);
		if ((c = strrchr(filename, '_')) != NULL &&
		    strcmp(c, "_pub.h") == 0) {
			c[0] = '.'; c[1] = 'h'; c[2] = '\0';
			sprintf_s(dest, sizeof(dest), "%s\\..\\%s", dstDir,
			    filename);
		} else {
			sprintf_s(dest, sizeof(dest), "%s\\%s", dstDir,
			    filename);
		}

		printf("%s\n", dest);
		if (CopyFile(path, dest, 0) == 0) {
			printf("%s: CopyFile() failed\n", dest);
			return (-1);
		}
	}
	rv = GetLastError();
	FindClose(h);
	if (rv != ERROR_NO_MORE_FILES) {
		printf("FindNextFile Error (%lu)\n", rv);
		return (-1);
	}
	return (0);
}

int
RemoveEmptyDirs(const char *dir)
{
	char path[1024];
	WIN32_FIND_DATA fdata;
	DWORD attrs, rv;
	HANDLE h;
	
	sprintf_s(path, sizeof(path), "%s\\*", dir);
	if ((h = FindFirstFile(path, &fdata)) == INVALID_HANDLE_VALUE) {
		printf("Invalid file handle (%d)\n", GetLastError());
		return (-1);
	}
	while (FindNextFile(h, &fdata) != 0) {
		if (fdata.cFileName[0] == '.') {
			continue;
		}
		sprintf_s(path, sizeof(path), "%s\\%s", dir, fdata.cFileName);
		if ((attrs = GetFileAttributes(path)) ==
		    INVALID_FILE_ATTRIBUTES) {
			printf("GetFileAttributes(%s) failed\n", path);
			continue;
		}
		if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
			if (RemoveEmptyDirs(path) == -1)
				return (-1);
		}
	}
	rv = GetLastError();
	FindClose(h);
	if (rv != ERROR_NO_MORE_FILES) {
		printf("FindNextFile Error (%lu)\n", rv);
		return (-1);
	}
	RemoveDirectory(dir);
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

int
main(int argc, char *argv[])
{
	char *dir = DEFAULT_DIR, *over;
	char libdir[2048], incldir[2048], path[2048];
	int i;
	DWORD attrs;

	printf("Agar SDK Installation\n");
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

	printf("Installing Agar SDK into %s\n", dir);
	CreateDirectory(dir, NULL);
	sprintf_s(incldir, sizeof(incldir), "%s\\include", dir);
	CreateDirectory(incldir, NULL);

	sprintf_s(libdir, sizeof(libdir), "%s\\lib", dir);
	CreateDirectory(libdir, NULL);

	sprintf_s(libdir, sizeof(libdir), "%s\\lib\\%s", dir, dstArch);
	CreateDirectory(libdir, NULL);

	if (InstallLibs(dir) == -1) {
		printf("Failed to install libraries\n");
		exit(1);
	}
	if (InstallIncludes("include", incldir) == -1) {
		printf("Failed to install includes\n");
		exit(1);
	}
	RemoveEmptyDirs(incldir);

	for (i = 0; i < nMiscFiles; i++) {
		sprintf_s(path, sizeof(path), "%s\\%s", dir, miscFiles[i]);
		printf("%s\n", path);
		if (CopyFile(miscFiles[i], path, 0) == 0) {
			printf("%s: CopyFile() failed; ignoring\n", path);
			continue;
		}
	}

	printf("\nAgar SDK was successfully installed under %s\n", dir);
	AnyKey();

	exit(0);
}

