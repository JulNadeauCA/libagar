/*	$Csoft: video.c,v 1.5 2002/01/27 10:59:35 vedge Exp $	 */

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include <smpeg.h>
#include <glib.h>

#include <engine/debug.h>
#include <engine/video.h>


static void     *video_tick(void *);

static void *
video_tick(void *arg)
{
	struct video *v = (struct video *)arg;
	SMPEG_Info info;
	SDL_Event ev;

	SDL_mutexP(v->lock);
	SMPEG_getinfo(v->mpeg, &info);

	while (SMPEG_status(v->mpeg) == SMPEG_PLAYING) {
		SDL_mutexV(v->lock);
		while (SDL_PollEvent(&ev) == 1) {
			switch (ev.type) {
			case SDL_KEYDOWN:
				switch (ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					goto videodone;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				goto videodone;
			default:
				break;
			}
		}
		SDL_Delay(1000);
	}

videodone:
	SDL_mutexV(v->lock);
	return (NULL);
}

/*
 * Play an audiovisual mpeg stream. The surface should be 
 */
struct video *
video_create(char *path, SDL_Surface *s)
{
	SDL_SysWMinfo wm;
	SMPEG_Info info;
	struct stat sta;
	char *errmsg, **exts;
	struct video *v = NULL;
	int i, xvideo = 0, nexts = 0;

	/*
	 * Work around smpeg not returning an error code when the
	 * XVideo extension is missing. We must obtain a pointer to
	 * the X11 Display structure through the window manager.
	 */
	SDL_VERSION(&wm.version);
	if (SDL_GetWMInfo(&wm) != 1) {
		return (NULL);
	}
	wm.info.x11.lock_func();
	exts = XListExtensions(wm.info.x11.display, &nexts);
	for (i = 0; i < nexts; i++) {
		if (strcmp("XVideo", exts[i]) == 0) {
			xvideo++;
		}
	}
	free(exts);
	wm.info.x11.unlock_func();
	if (!xvideo) {
		warning("XVideo extension missing, skipping video.\n");
		return (NULL);
	}

	v = malloc(sizeof(struct video));
	v->flags = 0;
	v->fd = 0;
	v->mpeg = NULL;
	v->lock = SDL_CreateMutex();

	if ((stat(path, &sta) < 0) || !S_ISFIFO(sta.st_mode)) {
		v->fd = 0;
		v->mpeg = SMPEG_new(path, &info, 1);
	} else {
		v->fd = open(path, O_RDONLY, 0);
		v->mpeg = SMPEG_new_descr(v->fd, &info, 1);
	}

	errmsg = SMPEG_error(v->mpeg);
	if (errmsg != NULL) {
		warning("%s: %s\n", path, errmsg);
		goto videoerr;
	}

	SMPEG_scaleXY(v->mpeg, s->w, s->h);
	SDL_mutexP(v->lock);

	SMPEG_setdisplay(v->mpeg, s, NULL, NULL);
	SMPEG_enablevideo(v->mpeg, 1);
	SMPEG_enableaudio(v->mpeg, 1);
	SMPEG_loop(v->mpeg, 0);
	SMPEG_play(v->mpeg);

	/* XXX preference */
#if 0
	SMPEG_Filter *filter;
		
	filter = SMPEGfilter_bilinear();
	filter = SMPEG_filter(v->mpeg, filter);
	filter->destroy(filter);
#endif
	SDL_mutexV(v->lock);

	putenv("SDL_VIDEO_CENTERED=0");
	if (pthread_create(&v->thread, NULL, video_tick, v) != 0) {
		perror("playback");
		goto videoerr;
	}
	return (v);

videoerr:
	if (v != NULL) {
		if (v->mpeg != NULL) {
			SMPEG_delete(v->mpeg);
		}
		free(v);
	}
	return (NULL);
}

void
video_destroy(struct video *v)
{
	SDL_mutexP(v->lock);
	pthread_kill(v->thread, SIGTERM);
	SMPEG_stop(v->mpeg);
	SMPEG_delete(v->mpeg);
	if (v->fd != 0) {
		close(v->fd);
	}
	SDL_mutexV(v->lock);
	SDL_DestroyMutex(v->lock);
	free(v);
}

