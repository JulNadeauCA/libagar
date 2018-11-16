/*
 * Copyright (c) 2010 Michael J. Wood
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

#ifdef _XBOX

#include <agar/core/core.h>
#include <agar/core/xbox.h>

#define INVALID_FILE_ATTRIBUTES -1

DWORD
AG_XBOX_GetXbeTitleId(const char *xbePath)
{
	HANDLE h;
	DWORD titleId = -1;
	DWORD loadAddress;
	DWORD certLocation;
	DWORD bytesRead;
	ANSI_STRING file;
	OBJECT_ATTRIBUTES attr;
	IO_STATUS_BLOCK status;

	if(!xbePath)
		return -1;

	RtlInitAnsiString(&file, xbePath);

	InitializeObjectAttributes(&attr, &file, OBJ_CASE_INSENSITIVE, NULL);

	if(SUCCEEDED(NtCreateFile(&h, FILE_GENERIC_READ, &attr, &status,
	                           NULL, 0, 0, FILE_OPEN,
	                           FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT))) {
		if(SetFilePointer(h,  0x104, NULL, FILE_BEGIN) == 0x104) {
			ReadFile(h, &loadAddress, sizeof(DWORD), &bytesRead, NULL);
			if(bytesRead == sizeof(DWORD)) {
				if(SetFilePointer(h, 0x118, NULL, FILE_BEGIN ) == 0x118) {
					ReadFile(h, &certLocation, sizeof(DWORD), &bytesRead, NULL);
					if(bytesRead == sizeof(DWORD)) {
						certLocation -= loadAddress;
						certLocation += 8;
						titleId = 0;
						if(SetFilePointer(h, certLocation, NULL, FILE_BEGIN) == certLocation) {
							ReadFile(h, &titleId, sizeof(DWORD), &bytesRead, NULL);
							if(bytesRead != sizeof(DWORD)) {
								titleId = 0;
							}
						}
					}
				}
			}
		}
		CloseHandle(h);
	}

	return titleId;
}

char*
AG_XBOX_GetDeviceFromLogicalDrive(const char *drive)
{
	HANDLE h;
	ANSI_STRING szDevice;
	ANSI_STRING szDrive;
	OBJECT_ATTRIBUTES attr;
	ULONG size;
	char devBuf[256];
	char driveL[3];
	char driveP[7];

	if(!drive || strlen(drive) < 2 ||
		!isalpha(drive[0]) || drive[1] != ':') {
		return NULL;
	}

	Strlcpy(driveL, drive, sizeof(driveL));
	Snprintf(driveP, sizeof(driveP), "\\??\\%s", driveL);
	RtlInitAnsiString(&szDrive, driveP);

	szDevice.Buffer = devBuf;
	szDevice.Length = 0xf;
	szDevice.MaximumLength = 256;

	InitializeObjectAttributes(&attr, &szDrive, OBJ_CASE_INSENSITIVE, NULL);

	if(SUCCEEDED(NtOpenSymbolicLinkObject(&h, &attr))) {
		if(SUCCEEDED(NtQuerySymbolicLinkObject(h, &szDevice, &size))) {
			Strlcpy(devBuf, szDevice.Buffer, size + 1);
			CloseHandle(h);
			return TryStrdup(devBuf);
		}
		CloseHandle(h);
	}

	return NULL;
}

BOOL
AG_XBOX_DriveIsMounted(const char driveLetter)
{
	char path[3];
	char *dev;

	if(!isalpha(driveLetter)) {
		return FALSE;
	}

	path[0] = driveLetter;
	path[1] = ':';
	path[2] = '\0';

	if((dev = AG_XBOX_GetDeviceFromLogicalDrive(path)) == NULL) {
		return FALSE;
	}

	Free(dev);

	return TRUE;
}

BOOL
AG_XBOX_PathIsValid(const char *path)
{
	DWORD attr;

	if(path && (attr = GetFileAttributes(path)) != INVALID_FILE_ATTRIBUTES) {
		if(attr & FILE_ATTRIBUTE_DIRECTORY) {
			return TRUE;
		}
	}

	return FALSE;
}

DWORD
AG_XBOX_GetLogicalDrives()
{
	DWORD d;
	int i;

	for (i = 0; i < 26; i++) {
		if(AG_XBOX_DriveIsMounted('A'+i)) {
			d |= (1 << i);
		} else {
			d &= ~(1 << i);
		}
	}

	return d;
}

#endif /* _XBOX */
