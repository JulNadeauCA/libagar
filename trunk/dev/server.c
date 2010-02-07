/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Debug server. This runs in a separate thread and accepts authenticated
 * connections. Various commands useful for debugging are implemented, such
 * as query of display variables and surface contents.
 */

#include <config/ag_network.h>
#include <config/ag_threads.h>
#if defined(AG_NETWORK) && defined(AG_THREADS)

#include <config/version.h>
#include <config/release.h>
#include <config/have_jpeg.h>
#include <config/have_opengl.h>

#include <core/core.h>
#include <core/config.h>
#include <core/net.h>

#include <gui/window.h>
#include <gui/tlist.h>
#include <gui/text.h>
#include <gui/label.h>

#include "dev.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_JPEG
#undef HAVE_STDLIB_H		/* Work around SDL.h retardation */
#include <jpeglib.h>
#endif

#define PROTO_VERSION "1.0"
#define DEFAULT_PORT "1234"

struct client {
	const char *name;
	const char *hostname;
	const char *version;
	AG_Window *win;
	int sock;
	TAILQ_ENTRY(client) clients;
};

static AG_Mutex lock = AG_MUTEX_INITIALIZER;
static TAILQ_HEAD_(client) clients = TAILQ_HEAD_INITIALIZER(clients);
static NS_Server server;
static int server_inited = 0;
static AG_Thread listenTh;
static int servRunning = 0;

#if 0
#ifdef HAVE_JPEG
static int jpegQuality = 75;
#endif
#endif

static void
PollClients(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_TlistItem *it;
	struct client *cl;

	AG_MutexLock(&lock);
	AG_TlistClear(tl);
	TAILQ_FOREACH(cl, &clients, clients) {
		it = AG_TlistAdd(tl, NULL, "%s (%s): %s", cl->name,
		    cl->hostname, cl->version);
		it->p1 = cl;
		it->cat = "client";
	}
	AG_TlistRestore(tl);
	AG_MutexUnlock(&lock);
}

static int
auth_password(NS_Server *ns, void *p)
{
	char buf[64];

	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (-1);

	if (strcmp(buf, "foo:bar\n") == 0) {
		return (1);
	}
	AG_SetError("User/password mismatch");
	return (0);
}

static int
srv_error(NS_Server *srv)
{
	NS_Log(NS_ERR, "%s: function failed (%s)", OBJECT(srv)->name,
	    AG_GetError());
	return (-1);
}

static int
srv_login(NS_Server *srv, void *p)
{
	return (0);
}

static void
srv_logout(NS_Server *srv, void *p)
{
}

static int
cmd_version(NS_Server *ns, NS_Command *cmd, void *p)
{
	char hostname[128];

	hostname[0] = '\0';
	NS_BeginList(ns);
	NS_ListString(ns, "%s", VERSION);
	NS_ListString(ns, "%s", RELEASE);
	NS_ListString(ns, "%s", "localhost");
	NS_EndList(ns);
	return (0);
}

#if 0
/*
 * XXX TODO 1.4
 */
#ifdef HAVE_JPEG
static void
jpegError(j_common_ptr jcomp)
{
	fprintf(stderr, "jpeg error\n");
}

static void
jpegOutputMsg(j_common_ptr jcomp)
{
	fprintf(stderr, "jpeg message\n");
}
#endif /* HAVE_JPEG */

/* Send the JPEG-compressed contents of a surface to the client. */
static int
cmd_surface(NS_Server *ns, NS_Command *cmd, void *pSu)
{
#ifdef HAVE_JPEG
	AG_Driver *drv = agDriver;		/* XXX 1.4 single-display */
	char sendbuf[AG_BUFFER_MAX];
	static struct jpeg_error_mgr jerrmgr;
	static struct jpeg_compress_struct jcomp;
	char tmp[sizeof("/tmp/")+AG_FILENAME_MAX];
	AG_Surface *su = pSu;
	Uint8 *jcopybuf;
	int i, nshots = 1;
	size_t len;
	FILE *ftmp;
	int fd;
	size_t rv;

	if (!AGDRIVER_SINGLE(drv)) {
		AG_SetError("Not using a single-display driver");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(drv)->videoCapture(drv, &su) == -1)
		return (-1);

	/* Write the JPEG to a temporary file. */
	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = jpegError;
	jerrmgr.output_message = jpegOutputMsg;
	jpeg_create_compress(&jcomp);
	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;
	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, jpegQuality, TRUE);
#ifdef HAVE_MKSTEMP
	Strlcpy(tmp, "/tmp/agarXXXXXXXX", sizeof(tmp));
	if ((fd = mkstemp(tmp)) == -1) {
		AG_SetError("mkstemp failed (%s)", tmp);
		return (-1);
	}
#else
	Strlcpy(tmp, ".agarsurface.tmp", sizeof(tmp));
	if ((fd = open(tmp, O_RDWR|O_CREAT|O_EXCL)) == -1) {
		AG_SetError("mkstemp failed (%s)", tmp);
		return (-1);
	}
#endif
	if ((ftmp = fdopen(fd, "r+")) == NULL) {
		AG_SetError("fdopen failed");
		return (-1);
	}
	jpeg_stdio_dest(&jcomp, ftmp);
	jcopybuf = Malloc(su->w*3);
	for (i = 0; i < nshots; i++) {
		JSAMPROW row[1];
		int x;

		jpeg_start_compress(&jcomp, TRUE);
		while (jcomp.next_scanline < jcomp.image_height) {
			Uint8 *pSrc = (Uint8 *)su->pixels +
			    jcomp.next_scanline * su->pitch;
			Uint8 *pDst = jcopybuf;
			Uint8 r, g, b;

			for (x = agView->w; x > 0; x--) {
				AG_GetPixelRGB(AG_GET_PIXEL(su,pSrc), su->format,
				    &r,&g,&b);
				*pDst++ = r;
				*pDst++ = g;
				*pDst++ = b;
				pSrc += su->format->BytesPerPixel;
			}
			row[0] = jcopybuf;
			jpeg_write_scanlines(&jcomp, row, 1);
		}
		jpeg_finish_compress(&jcomp);
	}
#ifdef HAVE_OPENGL
	if ((AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL) &&
	    su != pSu)
		AG_SurfaceFree(su);
#endif

	/* Send the JPEG data to the client. */
	len = (size_t)ftell(ftmp);
	rewind(ftmp);
	NS_BeginData(ns, len);
	while ((rv = fread(sendbuf, 1, sizeof(sendbuf), ftmp)) > 0) {
		NS_Data(ns, sendbuf, rv);
	}
	NS_EndData(ns);
	fclose(ftmp);
	AG_FileDelete(tmp);
	Free(jcopybuf);
	jpeg_destroy_compress(&jcomp);
	return (0);
#else /* !HAVE_JPEG */
	AG_SetError("libjpeg is not available");
	return (-1);
#endif /* HAVE_JPEG */
}

/* Return information about the display format. */
static int
cmd_view_fmt(NS_Server *ns, NS_Command *cmd, void *p)
{
	NS_BeginList(ns);
	/* XXX TODO 1.4 */
	NS_EndList(ns);
	return (0);
}

static void
cmd_scan_vfs(NS_Server *ns, NS_Command *cmd, AG_Object *pob, int depth)
{
	AG_Object *cob;

	NS_ListString(ns, "%s", pob->name);
	NS_ListString(ns, "%s", pob->cls->hier);
	NS_ListString(ns, "0x%08x", pob->flags);

	TAILQ_FOREACH(cob, &pob->children, cobjs) {
		cmd_scan_vfs(ns, cmd, cob, depth+1);
	}
}

/* Return information about the current virtual filesystem. */
static int
cmd_scan_vfs(NS_Server *ns, NS_Command *cmd, void *p)
{
	NS_BeginList(ns);
	AG_LockLinkage();
	cmd_scan_vfs(ns, cmd, agWorld, 0);
	AG_UnlockLinkage();
	NS_EndList(ns);
	return (0);
}
#endif

static void *
ServerLoop(void *p)
{
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Debug server started"));

	if (NS_ServerLoop(&server) == -1) {
		AG_TextMsgFromError();
	}
	AG_ThreadExit(NULL);
	return (NULL);
}

int
DEV_DebugServerStart(void)
{
	int rv;

	if (!server_inited) {
		AG_ObjectInitStatic(&server, &nsServerClass);
		AG_ObjectSetName(&server, "_DebugServer");
		NS_ServerSetProtocol(&server, "agar-debug", PROTO_VERSION);
		NS_ServerBind(&server, NULL, DEFAULT_PORT);
	
		NS_RegAuthMode(&server, "password", auth_password, NULL);
		NS_RegErrorFn(&server, srv_error);
		NS_RegLoginFn(&server, srv_login);
		NS_RegLogoutFn(&server, srv_logout);

		NS_RegCmd(&server, "version", cmd_version, NULL);
#if 0
		NS_RegCmd(&server, "screen", cmd_surface, agView->v);
		NS_RegCmd(&server, "view-fmt", cmd_view_fmt, NULL);
		NS_RegCmd(&server, "scan-vfs", cmd_scan_vfs, NULL);
#endif
		server_inited = 1;
	}
	if ((rv = AG_ThreadCreate(&listenTh, ServerLoop, NULL)) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "AG_ThreadCreate: %s", strerror(rv));
		return (-1);
	}
	return (0);
}

static void
StartServer(AG_Event *event)
{
	NS_InitSubsystem(0);
	if (DEV_DebugServerStart() == 0)
		servRunning = 1;
}

static void
StopServer(AG_Event *event)
{
	if (servRunning) {
		AG_ThreadKill(listenTh, 15);
		servRunning = 0;
	}
}

AG_Window *
DEV_DebugServer(void)
{
	AG_Window *win;
	AG_Tlist *tl;
	
	win = AG_WindowNewNamedS(0, "DEV_DebugServer");
	AG_WindowSetCaptionS(win, _("Debug Server"));
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_RIGHT, 0);

	tl = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_TlistSizeHint(tl, "CLIENT (255.255.255.255): 0.0-beta", 8);
	AG_SetEvent(tl, "tlist-poll", PollClients, NULL);
		
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Start server"),
	    StartServer, "%p", tl);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Stop server"),
	    StopServer, "%p", tl);

	AG_LabelNewS(win, 0, _("Connected clients:"));
	AG_ObjectAttach(win, tl);
	return (win);
}
#endif /* AG_NETWORK and AG_THREADS */
