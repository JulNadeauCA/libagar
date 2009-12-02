/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Screenshot upload tool. This runs in a separate thread, connects to the
 * given agar screenshot server and periodically uploads the contents of
 * the display in JPEG format.
 * XXX 1.4 single-display
 */

#include <config/ag_network.h>
#include <config/ag_threads.h>
#include <config/have_jpeg.h>
#include <config/have_opengl.h>

#if defined(AG_NETWORK) && defined(AG_THREADS) && defined(HAVE_JPEG)

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <core/core.h>
#include <core/config.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/hbox.h>
#include <gui/textbox.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/numerical.h>

#include "dev.h"

#undef HAVE_STDLIB_H		/* Work around SDL.h retardation */
#include <jpeglib.h>

static AG_Thread thread;
static AG_Mutex lock = AG_MUTEX_INITIALIZER;
static AG_Mutex xmit_lock = AG_MUTEX_INITIALIZER;
static int default_port = 1173;
static int sock = -1;
static int aflag = 0;
static int xmit_delay = 1000;

static AG_Textbox *hosttb, *porttb;
static char status[128];

static struct jpeg_error_mgr		 jerrmgr;
static struct jpeg_compress_struct	 jcomp;

static void
HandleJpegError(j_common_ptr jcomp)
{
	AG_TextMsg(AG_MSG_ERROR, _("A JPEG error has occured."));
}

static void
OutputJpegMessage(j_common_ptr jcomp)
{
}

static void
XmitLoop(int fd)
{
	int nframe = 0;
	FILE *fp;
	AG_Surface *su;
	Uint8 *jcopybuf;

	if ((fp = fdopen(fd, "w")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "fdopen failed");
		return;
	}

	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = HandleJpegError;
	jerrmgr.output_message = OutputJpegMessage;
	
	jpeg_create_compress(&jcomp);

	jcomp.image_width = agDriverSw->w;
	jcomp.image_height = agDriverSw->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, agScreenshotQuality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(agDriverSw->w*3);

	for (;;) {
		JSAMPROW row[1];
		int x;

		AG_MutexLock(&xmit_lock);
		if (aflag) {
			aflag = 0;
			AG_MutexUnlock(&xmit_lock);
			break;
		}

		Snprintf(status, sizeof(status), _("Transmitting frame %d"),
		    nframe);
	
		if (agDriverSw == NULL) {
			Verbose("Not using a single-display driver\n");
			AG_MutexUnlock(&xmit_lock);
			return;
		}
		if (AGDRIVER_SW_CLASS(agDriverSw)->videoCapture(agDriverSw, &su)
		    == -1) {
			Verbose("Capture failed: %s\n", AG_GetError());
			AG_MutexUnlock(&xmit_lock);
			return;
		}

		jpeg_start_compress(&jcomp, TRUE);

		while (jcomp.next_scanline < jcomp.image_height) {
			Uint8 *pSrc = (Uint8 *)su->pixels +
			    jcomp.next_scanline * su->pitch;
			Uint8 *pDst = jcopybuf;
			Uint8 r, g, b;

			for (x = agDriverSw->w; x > 0; x--) {
				AG_GetPixelRGB(AG_GET_PIXEL(su,pSrc),
				    su->format, &r,&g,&b);
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
		if (AGDRIVER_CLASS(agDriverSw)->flags & AG_DRIVER_OPENGL)
			AG_SurfaceFree(su);
#endif

		AG_Delay(xmit_delay);
		nframe++;
		AG_MutexUnlock(&xmit_lock);
	}
	Free(jcopybuf);
	jpeg_destroy_compress(&jcomp);
	fclose(fp);
}

static void *
XmitThread(void *p)
{
	char host[256];
	char port[32];
	struct addrinfo hints, *res, *res0;
	const char *cause = "";
	int rv;

	AG_MutexLock(&lock);

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

	AG_MutexLock(&xmit_lock);
	Snprintf(status, sizeof(status), _("Connecting to %s:%s.."),
	    host, port);
	AG_MutexUnlock(&xmit_lock);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(host, port, &hints, &res0)) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", host,
		    gai_strerror(rv));
		AG_MutexLock(&xmit_lock);
		Snprintf(status, sizeof(status), "%s", gai_strerror(rv));
		AG_MutexUnlock(&xmit_lock);
		goto out1;
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

		AG_MutexLock(&xmit_lock);
		Snprintf(status, sizeof(status), "%s: %s", host, cause);
		AG_MutexUnlock(&xmit_lock);
		goto out2;
	}

	AG_MutexLock(&xmit_lock);
	Snprintf(status, sizeof(status), _("Connected to %s"), host);
	AG_MutexUnlock(&xmit_lock);

	XmitLoop(sock);
	close(sock);
	sock = -1;
out2:
	freeaddrinfo(res0);
out1:
	AG_MutexUnlock(&lock);
	AG_ThreadExit(NULL);
}

static void
Connect(AG_Event *event)
{
	if (agDriverSw == NULL) {
		AG_TextMsg(AG_MSG_ERROR,
		    "This feature requires a single-display graphics driver");
		return;
	}
	if (AG_ThreadCreate(&thread, XmitThread, NULL) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "Failed to create thread!");
	}
}

static void
Disconnect(AG_Event *event)
{
	AG_MutexLock(&xmit_lock);
	aflag++;
	Snprintf(status, sizeof(status), _("Disconnected"));
	AG_MutexUnlock(&xmit_lock);
}

AG_Window *
DEV_ScreenshotUploader(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_HBox *hb;
	AG_Label *lbl;
	AG_Numerical *num;
	
	if ((win = AG_WindowNewNamedS(AG_WINDOW_NOVRESIZE,
	    "DEV_ScreenshotUploader")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Screenshot"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		Strlcpy(status, _("Idle"), sizeof(status));
#ifdef AG_THREADS
		lbl = AG_LabelNewPolledMT(vb, AG_LABEL_HFILL, &xmit_lock,
		    _("Status: %s."), &status);
#else
		lbl = AG_LabelNewPolled(vb, AG_LABEL_HFILL,
		    _("Status: %s."), &status);
#endif
		AG_LabelSizeHint(lbl, 1,
		    _("Status: Transmitting frame XXXXXXXXXX"));

		hosttb = AG_TextboxNew(vb, AG_TEXTBOX_HFILL, _("Host: "));
		porttb = AG_TextboxNew(vb, AG_TEXTBOX_HFILL, _("Port: "));
		AG_WidgetFocus(hosttb);

		num = AG_NumericalNewS(vb, AG_NUMERICAL_HFILL,
		    "ms", _("Refresh rate: "));
		AG_BindIntMp(num, "value", &xmit_delay, &xmit_lock);
		AG_NumericalSetRange(num, 1, 10000);

		AG_TextboxPrintf(porttb, "%i", default_port);
	}

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL|AG_HBOX_VFILL);
	{
		AG_Event *ev;
		AG_Button *bu;

		bu = AG_ButtonNewS(hb, 0, _("Connect"));
		AG_SetEvent(bu, "button-pushed", Connect, NULL);
		
		bu = AG_ButtonNewS(hb, 0, _("Disconnect"));
		ev = AG_SetEvent(bu, "button-pushed", Disconnect, NULL);
		//ev->flags |= AG_EVENT_ASYNC;
	}
	return (win);
}

#endif /* AG_NETWORK and AG_THREADS and HAVE_JPEG */
