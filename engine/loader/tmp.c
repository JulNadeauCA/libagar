/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <compat/dir.h>
#include <compat/arc4random.h>

#include <engine/engine.h>
#include <engine/config.h>

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "tmp.h"

struct netbuf *
tmp_open(const char *name)
{
	char savedir[MAXPATHLEN];
	char path[MAXPATHLEN];
	struct stat sb;
	struct netbuf *buf;

	/* XXX import mkstemp() eventually */
	prop_copy_string(config, "save-path", savedir, sizeof(savedir));
	strlcat(savedir, "/.tmp", sizeof(savedir));
	if (stat(savedir, &sb) == -1 && Mkdir(savedir) == -1) {
		error_set("%s: %s", savedir, strerror(errno));
		return (NULL);
	}
	snprintf(path, sizeof(path), "%s/%s%u", savedir, name, arc4random());

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	return (netbuf_open(path, "w+", NETBUF_BIG_ENDIAN));
#else
	return (netbuf_open(path, "w+", NETBUF_LITTLE_ENDIAN));
#endif
}

void
tmp_close(struct netbuf *buf)
{
	unlink(buf->path);
	netbuf_close(buf);
}
