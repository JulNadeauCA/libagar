/*	$Csoft$	 */

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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <SDL.h>
#include <smpeg.h>

#include "debug.h"
#include "video.h"

static SDL_mutex *mpeg_mutex;
static int stream = 0;

static void     *video_tick(void *);

static void *
video_tick(void *arg)
{
	SMPEG *mpeg = (SMPEG *)arg;
	SMPEG_Info info;
	SDL_Event event;

	SDL_mutexP(mpeg_mutex);
	SMPEG_getinfo(mpeg, &info);

	while (SMPEG_status(mpeg) == SMPEG_PLAYING) {
		SDL_mutexV(mpeg_mutex);
		while (SDL_WaitEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					/* Stop */
					break;
				case SDLK_f:
					/* Fullscreen */
					break;
				case SDLK_SPACE:
					/* Double size */
					break;
				default:
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
				case 2:
					/* Double size */
					break;
				case 3:
					/* Fullscreen */
					break;
				}
				break;
			default:
				break;
			}
		}
	}
	SDL_mutexV(mpeg_mutex);
	return (NULL);
}

/*
 * Play an audiovisual mpeg stream. The surface should be 
 */
int
video_play(char *path, SDL_Surface *v)
{
	SMPEG_Info info;
	int bitrate = 0, srate = 0, nch = 1;
	int fd;
	char *errmsg;
	struct stat buf;
	SMPEG *mpeg;
	pthread_t video_th;

	if ((stat(path, &buf) < 0) || !S_ISFIFO(buf.st_mode)) {
		stream = 0;
		mpeg = SMPEG_new(path, &info, 1);
	} else {
		fd = open(path, O_RDONLY, 0);
		stream = 1;
		mpeg = SMPEG_new_descr(fd, &info, 1);
	}

	errmsg = SMPEG_error(mpeg);
	if (errmsg != NULL) {
		fatal("%s: %s\n", path, errmsg);
		SMPEG_delete(mpeg);
		return (NULL);
	}

	mpeg_mutex = SDL_CreateMutex();

	SMPEG_scaleXY(mpeg, v->w, v->h);

	SDL_mutexP(mpeg_mutex);

	SMPEG_setdisplay(mpeg, v, NULL, NULL);
	SMPEG_enablevideo(mpeg, 1);
	SMPEG_enableaudio(mpeg, 1);
	SMPEG_loop(mpeg, 0);
	SMPEG_play(mpeg);
#if 0
		SMPEG_Filter *filter;
		
		filter = SMPEGfilter_bilinear();
		filter = SMPEG_filter(mpeg, filter);
		filter->destroy(filter);
#endif
	SDL_mutexV(mpeg_mutex);

	if (info.has_audio) {
		char *tmp = strstr(info.audio_string, "kbit/s");
		if (tmp) {
			while (isdigit(tmp[-1])) {
				tmp--;
			}
			sscanf(tmp, "%dkbit/s", &bitrate);
		}
		tmp = strstr(info.audio_string, "Hz");
		if (tmp) {
			while (isdigit(tmp[-1])) {
				tmp--;
			}
			sscanf(tmp, "%dHz", &srate);
		}
		if (strstr(info.audio_string, "stereo")) {
			nch = 2;
		} else if (strstr(info.audio_string, "mono")) {
			nch = 1;
		}
	}

	putenv("SDL_VIDEO_CENTERED=0");
	if (pthread_create(&video_th, NULL, video_tick, mpeg) != 0) {
		perror("playback");
		return (-1);
	}
	pthread_join(video_th, NULL);
	return (0);
}

#if 0
void 
stop()
{
	SDL_mutexP(mpeg_mutex);
	SDL_KillThread(thread);
	SMPEG_stop(mpeg);
	SMPEG_delete(mpeg);
	if (stream)
		close(fd);

	SDL_mutexV(mpeg_mutex);
	SDL_DestroyMutex(mpeg_mutex);
	SDL_FreeSurface(screen);
	stream = 0;
}
#endif
