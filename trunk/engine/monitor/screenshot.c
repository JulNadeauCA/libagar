/*	$Csoft: screenshot.c,v 1.4 2003/05/18 00:17:04 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/engine.h>

#include <config/have_jpeg.h>

#if defined(DEBUG) && defined(THREADS) && defined(HAVE_JPEG)

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <jpeglib.h>

#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>

#include "monitor.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int default_port = 1173;
static int sock = -1;

static struct button *connectbu, *disconnectbu;
static struct textbox *hosttb, *porttb;
static struct label *statusl;

static struct jpeg_error_mgr		 jerrmgr;
static struct jpeg_compress_struct	 jcomp;

static void
screenshot_error_exit(j_common_ptr jcomp)
{
	text_msg("JPEG error", "JPEG error occured");
}

static void
screenshot_output_message(j_common_ptr jcomp)
{
}

static void
screenshot_xmit(int fd)
{
	int nframe = 0;
	FILE *fp;
	SDL_Surface *srcsu = view->v;
	Uint8 *jcopybuf;

	if ((fp = fdopen(fd, "w")) == NULL) {
		text_msg("Error sending screenshot",
		    "fdopen: %s", strerror(errno));
		return;
	}

	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = screenshot_error_exit;
	jerrmgr.output_message = screenshot_output_message;
	
	jpeg_create_compress(&jcomp);

	jcomp.image_width = srcsu->w;
	jcomp.image_height = srcsu->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, 75, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(srcsu->w * 3);

	for (;;) {
		JSAMPROW row[1];
		int x;
	
		label_printf(statusl, "Status: xmit frame %d", nframe);

		jpeg_start_compress(&jcomp, TRUE);

		while (jcomp.next_scanline < jcomp.image_height) {
			Uint8 *src = (Uint8 *)srcsu->pixels +
			    jcomp.next_scanline * srcsu->pitch;
			Uint8 *dst = jcopybuf;
			Uint8 r, g, b;

			for (x = view->w; x > 0; x--) {
				switch (srcsu->format->BytesPerPixel) {
				case 4:
					SDL_GetRGB(*(Uint32 *)src,
					    srcsu->format, &r, &g, &b);
					break;
				case 3:
				case 2:
					SDL_GetRGB(*(Uint16 *)src,
					    srcsu->format, &r, &g, &b);
					break;
				case 1:
					SDL_GetRGB(*src,
					    srcsu->format, &r, &g, &b);
					break;
				}
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				src += srcsu->format->BytesPerPixel;
			}
			row[0] = jcopybuf;
			jpeg_write_scanlines(&jcomp, row, 1);
		}

		jpeg_finish_compress(&jcomp);
		SDL_Delay(1000);
		nframe++;
	}
	free(jcopybuf);
	jpeg_destroy_compress(&jcomp);
	fclose(fp);
}

static void
screenshot_connect(int argc, union evarg *argv)
{
	char *host, *port;
	struct addrinfo hints, *res, *res0;
	const char *cause = "";
	int rv;
	
	pthread_mutex_lock(&lock);
	button_disable(connectbu);
	button_disable(disconnectbu);

	if (sock != -1) {
		text_msg("Error", "Already connected");
		goto out1;
	}

	host = textbox_string(hosttb);
	port = textbox_string(porttb);

	if (strcmp(host, "") == 0 ||
	    strcmp(port, "") == 0) {
		text_msg("Error", "Missing server host/port");
		goto out2;
	}

	label_printf(statusl, "Status: connecting to %s:%s...", host, port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		text_msg("Error resolving hostname", "%s: %s", host,
		    gai_strerror(rv));
		label_printf(statusl, "Status: %s", gai_strerror(rv));
		goto out2;
	}

	for (sock = -1, res = res0; res != NULL; res = res->ai_next) {
		sock = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (sock < 0) {
			cause = "failed to create socket";
			continue;
		}
		if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
			cause = "connection refused";
			close(sock);
			sock = -1;
			continue;
		}
		break;
	}
	if (sock == -1) {
		text_msg("Error connecting", "%s: %s", host, cause);
		label_printf(statusl, "Status: %s: %s", host, cause);
		goto out3;
	}

	label_printf(statusl, "Status: connected to %s", host);
	screenshot_xmit(sock);
out3:
	freeaddrinfo(res0);
out2:
	free(host);
	free(port);
out1:
	button_enable(connectbu);
	button_enable(disconnectbu);
	pthread_mutex_unlock(&lock);
}

static void
screenshot_disconnect(int argc, union evarg *argv)
{
	pthread_mutex_lock(&lock);
	button_disable(connectbu);
	button_disable(disconnectbu);
	if (sock != -1) {
		close(sock);
		label_printf(statusl, "Status: disconnected");
	} else {
		text_msg("Error", "Not connected");
	}
	button_enable(connectbu);
	button_enable(disconnectbu);
	pthread_mutex_unlock(&lock);
}

struct window *
screenshot_window(void)
{
	struct window *win;
	struct region *reg;

	if ((win = window_generic_new(283, 121, "monitor-screenshot")) == NULL)
		return (NULL);	/* Exists */
	window_set_caption(win, "Screenshot");

	reg = region_new(win, REGION_VALIGN|REGION_CLIPPING, 0, 0, 100, -1);
	{
		statusl = label_new(reg, 100, -1, "Status: Not connected");
		hosttb = textbox_new(reg, "Host: ");
		porttb = textbox_new(reg, "Port: ");
		textbox_printf(porttb, "%i", default_port);
	}
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
	{
		struct event *ev;

		connectbu = button_new(reg, "Connect", NULL, 0, 50, -1);
		ev = event_new(connectbu, "button-pushed",
		    screenshot_connect, NULL);
		ev->flags |= EVENT_ASYNC;
		
		disconnectbu = button_new(reg, "Disconnect", NULL, 0, 50, -1);
		ev = event_new(disconnectbu, "button-pushed",
		    screenshot_disconnect, NULL);
		ev->flags |= EVENT_ASYNC;
	}
	return (win);
}

#endif	/* DEBUG and THREADS */
