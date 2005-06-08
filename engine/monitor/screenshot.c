/*	$Csoft: screenshot.c,v 1.18 2005/05/18 03:16:53 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/debug.h>
#include <config/threads.h>
#include <config/have_jpeg.h>

#if defined(DEBUG) && defined(THREADS) && defined(HAVE_JPEG)

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <jpeglib.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/spinbutton.h>

#include "monitor.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xmit_lock = PTHREAD_MUTEX_INITIALIZER;
static int default_port = 1173;
static int sock = -1;
static int aflag = 0;
static int xmit_delay = 1000;

static AG_Textbox *hosttb, *porttb;
static char status[128];

static struct jpeg_error_mgr		 jerrmgr;
static struct jpeg_compress_struct	 jcomp;

static void
screenshot_error_exit(j_common_ptr jcomp)
{
	AG_TextMsg(AG_MSG_ERROR, _("A JPEG error has occured."));
}

static void
screenshot_output_message(j_common_ptr jcomp)
{
}

static void
screenshot_xmit(int fd)
{
	extern int agScreenshotQuality;
	int nframe = 0;
	FILE *fp;
	SDL_Surface *su;
	Uint8 *jcopybuf;

	if ((fp = fdopen(fd, "w")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "fdopen: %s", strerror(errno));
		return;
	}

	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = screenshot_error_exit;
	jerrmgr.output_message = screenshot_output_message;
	
	jpeg_create_compress(&jcomp);

	jcomp.image_width = agView->w;
	jcomp.image_height = agView->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, agScreenshotQuality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(agView->w * 3, M_VIEW);

	for (;;) {
		JSAMPROW row[1];
		int x;

		pthread_mutex_lock(&xmit_lock);
		if (aflag) {
			aflag = 0;
			pthread_mutex_unlock(&xmit_lock);
			break;
		}

		snprintf(status, sizeof(status), _("Transmitting frame %d"),
		    nframe);

		if (!agView->opengl) {
			su = agView->v;
		} else {
#ifdef HAVE_OPENGL
			su = AG_CaptureGLView();
#endif
		}

		jpeg_start_compress(&jcomp, TRUE);

		while (jcomp.next_scanline < jcomp.image_height) {
			Uint8 *pSrc = (Uint8 *)su->pixels +
			    jcomp.next_scanline * su->pitch;
			Uint8 *pDst = jcopybuf;
			Uint8 r, g, b;

			for (x = agView->w; x > 0; x--) {
				SDL_GetRGB(AG_GET_PIXEL(su, pSrc), su->format,
				    &r, &g, &b);
				*pDst++ = r;
				*pDst++ = g;
				*pDst++ = b;
				pSrc += su->format->BytesPerPixel;
			}
			row[0] = jcopybuf;
			jpeg_write_scanlines(&jcomp, row, 1);
		}

		jpeg_finish_compress(&jcomp);

#ifdef HAVE_OPENGL
		if (agView->opengl)
			SDL_FreeSurface(su);
#endif

		SDL_Delay(xmit_delay);
		nframe++;
		pthread_mutex_unlock(&xmit_lock);
	}
	Free(jcopybuf, M_VIEW);
	jpeg_destroy_compress(&jcomp);
	fclose(fp);
}

static void
screenshot_connect(int argc, union evarg *argv)
{
	char host[256];
	char port[32];
	struct addrinfo hints, *res, *res0;
	const char *cause = "";
	int rv;
	
	pthread_mutex_lock(&lock);

	if (sock != -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Already connected to a server."));
		goto out1;
	}

	AG_TextboxCopyString(hosttb, host, sizeof(host));
	AG_TextboxCopyString(porttb, port, sizeof(port));

	if (host[0] == '\0' || port[0] == '\0') {
		AG_TextMsg(AG_MSG_ERROR, _("Missing server hostname/port."));
		goto out1;
	}

	snprintf(status, sizeof(status),
	    _("Connecting to %s:%s.."), host, port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", host,
		    gai_strerror(rv));
		snprintf(status, sizeof(status), "%s", gai_strerror(rv));
		goto out2;
	}

	for (sock = -1, res = res0; res != NULL; res = res->ai_next) {
		sock = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (sock < 0) {
			cause = _("failed to create socket");
			continue;
		}
		if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
			cause = _("connection refused");
			close(sock);
			sock = -1;
			continue;
		}
		break;
	}
	if (sock == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", host, cause);
		snprintf(status, sizeof(status), "%s: %s", host, cause);
		goto out2;
	}

	snprintf(status, sizeof(status), _("Connected to %s"), host);
	screenshot_xmit(sock);
	close(sock);
	sock = -1;
out2:
	freeaddrinfo(res0);
out1:
	pthread_mutex_unlock(&lock);
}

static void
screenshot_disconnect(int argc, union evarg *argv)
{
	pthread_mutex_lock(&xmit_lock);
	aflag++;
	pthread_mutex_unlock(&xmit_lock);

	snprintf(status, sizeof(status), _("Disconnected"));
}

AG_Window *
AG_DebugScreenshot(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_HBox *hb;
	AG_Label *lab;
	AG_Spinbutton *sbu;
	
	if ((win = AG_WindowNew(AG_WINDOW_DETACH|AG_WINDOW_NO_VRESIZE,
	    "monitor-screenshot")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Screenshot"));

	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	{
		strlcpy(status, _("Idle"), sizeof(status));
		lab = AG_LabelNew(vb, AG_LABEL_POLLED, _("Status: %s."), &status);
		AG_LabelPrescale(lab, _("Transmitting frame XXXXXXXXXXX"));
		AGWIDGET(lab)->flags |= AG_WIDGET_CLIPPING;

		hosttb = AG_TextboxNew(vb, _("Host: "));
		porttb = AG_TextboxNew(vb, _("Port: "));

		sbu = AG_SpinbuttonNew(vb, _("Refresh rate (ms): "));
		AG_SpinbuttonSetMin(sbu, 1);
		AG_WidgetBindMp(sbu, "value", &xmit_lock, AG_WIDGET_INT,
		    &xmit_delay);
		AG_SpinbuttonSetMax(sbu, 10000);

		AG_TextboxPrintf(porttb, "%i", default_port);
	}

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_WFILL|AG_HBOX_HFILL);
	{
		AG_Event *ev;
		AG_Button *bu;

		bu = AG_ButtonNew(hb, _("Connect"));
		ev = AG_SetEvent(bu, "button-pushed", screenshot_connect, NULL);
		ev->flags |= AG_EVENT_ASYNC;
		
		bu = AG_ButtonNew(hb, _("Disconnect"));
		ev = AG_SetEvent(bu, "button-pushed", screenshot_disconnect,
		    NULL);
		ev->flags |= AG_EVENT_ASYNC;
	}
	return (win);
}

#endif	/* DEBUG and THREADS */
