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

#include <config/debug.h>
#include <config/network.h>
#include <config/threads.h>

#if defined(DEBUG) && defined(NETWORK) && defined(THREADS)

#include <config/version.h>
#include <config/release.h>
#include <config/have_jpeg.h>

#include <core/core.h>
#include <core/view.h>
#include <core/config.h>

#include <net/net.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/tlist.h>
#include <gui/textbox.h>
#include <gui/fspinbutton.h>
#include <gui/mfspinbutton.h>
#include <gui/statusbar.h>
#include <gui/menu.h>
#include <gui/text.h>

#include "monitor.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_JPEG
#include <jpeglib.h>
#endif

struct client {
	const char *name;
	const char *hostname;
	const char *version;
	AG_Window *win;
	int sock;
	TAILQ_ENTRY(client) clients;
};

static AG_Mutex lock = AG_MUTEX_INITIALIZER;
static TAILQ_HEAD(,client) clients = TAILQ_HEAD_INITIALIZER(clients);
static NS_Server server;
static int server_inited = 0;
static AG_Thread listenTh;
static int servRunning = 0;
static int jpegQuality = 75;

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
	NS_Log(NS_ERR, "%s: function failed (%s)", srv->name, AG_GetError());
	return (-1);
}

static int
cmd_version(NS_Server *ns, NS_Command *cmd, void *p)
{
	char hostname[128];

	hostname[0] = '\0';
	gethostname(hostname, sizeof(hostname));
	printf("0 agar:%s:%s:%s\n", VERSION, RELEASE, hostname);
	return (0);
}

#ifdef HAVE_JPEG
static void
jpegError(j_common_ptr jcomp)
{
	printf("1 jpeg error\n");
}

static void
jpegOutputMsg(j_common_ptr jcomp)
{
	printf("1 jpeg error\n");
}
#endif /* HAVE_JPEG */

/* Send the JPEG-compressed contents of a surface to the client. */
static int
cmd_surface(NS_Server *ns, NS_Command *cmd, void *pSu)
{
#ifdef HAVE_JPEG
	char sendbuf[BUFSIZ];
	static struct jpeg_error_mgr jerrmgr;
	static struct jpeg_compress_struct jcomp;
	char tmp[sizeof("/tmp/")+FILENAME_MAX];
	SDL_Surface *su = pSu;
	Uint8 *jcopybuf;
	int i, nshots = 1;
	size_t l, len;
	FILE *ftmp;
	int fd;
	size_t rv;

#ifdef HAVE_OPENGL
	if (agView->opengl && pSu == agView->v)
		su = AG_CaptureGLView();
#endif

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
	strlcpy(tmp, "/tmp/agarXXXXXXXX", sizeof(tmp));
	if ((fd = mkstemp(tmp)) == -1) {
		AG_SetError("mkstemp %s: %s", tmp, strerror(errno));
		return (-1);
	}
#else
	strlcpy(tmp, ".agarsurface.tmp", sizeof(tmp));
	if ((fd = open(tmp, O_RDWR|O_CREAT|O_EXCL)) == -1) {
		AG_SetError("mkstemp %s: %s", tmp, strerror(errno));
		return (-1);
	}
#endif
	if ((ftmp = fdopen(fd, "r+")) == NULL) {
		AG_SetError("fdopen %s: %s", tmp, strerror(errno));
		return (-1);
	}
	jpeg_stdio_dest(&jcomp, ftmp);
	jcopybuf = Malloc(su->w * 3, M_VIEW);
	if (SDL_MUSTLOCK(su)) {
		SDL_LockSurface(su);
	}
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
	}
	if (SDL_MUSTLOCK(su)) {
		SDL_UnlockSurface(su);
	}
#ifdef HAVE_OPENGL
	if (agView->opengl && su != pSu)
		SDL_FreeSurface(su);
#endif

	/* Send the JPEG data to the client. */
	len = (size_t)ftell(ftmp);
	rewind(ftmp);
	NS_BinaryMode(ns, len);
	while ((rv = fread(sendbuf, 1, sizeof(sendbuf), ftmp)) > 0) {
		fwrite(sendbuf, 1, rv, stdout);
	}
	fflush(stdout);
	NS_CommandMode(ns);
	fclose(ftmp);
	unlink(tmp);
	Free(jcopybuf, M_VIEW);
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
	AG_MutexLock(&agView->lock);
	printf("0 %s:%ux%ux%u:%08x,%08x,%08x,%08x:%d:%d\n",
	    agView->opengl ? "gl" : "",
	    agView->w, agView->h, agVideoInfo->vfmt->BitsPerPixel,
	    agVideoInfo->vfmt->Rmask,
	    agVideoInfo->vfmt->Gmask,
	    agVideoInfo->vfmt->Bmask,
	    agVideoInfo->vfmt->Amask,
	    agVideoInfo->vfmt->colorkey,
	    agVideoInfo->vfmt->alpha);
	AG_MutexUnlock(&agView->lock);
	return (0);
}

/* Return the current refresh rate. */
static int
cmd_refresh(NS_Server *ns, NS_Command *cmd, void *p)
{
	AG_MutexLock(&agView->lock);
	printf("0 %d:%d\n", agView->rCur, agView->rNom);
	AG_MutexUnlock(&agView->lock);
	return (0);
}

static void
cmd_world_find(NS_Server *ns, NS_Command *cmd, AG_Object *pob, int depth)
{
	AG_Object *cob;

	printf("%d/%s/%s/0x%08x/%u:", depth, pob->name, pob->ops->type,
	    pob->flags, pob->data_used);

	TAILQ_FOREACH(cob, &pob->children, cobjs) {
		cmd_world_find(ns, cmd, cob, depth+1);
	}
}

/* Return information about the current virtual filesystem. */
static int
cmd_world(NS_Server *ns, NS_Command *cmd, void *p)
{
	AG_LockLinkage();
	fputs("0 ", stdout);
	cmd_world_find(ns, cmd, agWorld, 0);
	fputc('\n', stdout);
	AG_UnlockLinkage();
	return (0);
}

static void *
ServerLoop(void *p)
{
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Debug server started"));

	if (NS_Listen(&server) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
	AG_ThreadExit(NULL);
	return (NULL);
}

int
AG_DebugServerStart(void)
{
	int rv;

	if (!server_inited) {
		NS_ServerInit(&server, "agar-debug", VERSION);
		NS_RegAuthMode(&server, "password", auth_password, NULL);
		NS_RegErrorFn(&server, srv_error);

		NS_RegCmd(&server, "version", cmd_version, NULL);
		NS_RegCmd(&server, "screen", cmd_surface, agView->v);
		NS_RegCmd(&server, "view-fmt", cmd_view_fmt, NULL);
		NS_RegCmd(&server, "refresh", cmd_refresh, NULL);
		NS_RegCmd(&server, "world", cmd_world, NULL);
		server_inited = 1;
	}
	if ((rv = AG_ThreadCreate(&listenTh, NULL, ServerLoop, NULL)) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "AG_ThreadCreate: %s", strerror(rv));
		return (-1);
	}
	return (0);
}

static void
StartServer(AG_Event *event)
{
	if (AG_DebugServerStart() == 0)
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
AG_DebugServerWindow(void)
{
	AG_Window *win;
	AG_Menu *me;
	AG_MenuItem *mi;
	AG_Tlist *tl;
	AG_Box *bo;
	
	win = AG_WindowNew(AG_WINDOW_NOCLOSE);
	AG_WindowSetCaption(win, _("Agar clients"));
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_RIGHT, 0);

	tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_TlistPrescale(tl, "CLIENT (255.255.255.255): 0.0-beta", 8);
	AG_SetEvent(tl, "tlist-poll", PollClients, NULL);
		
	AG_ButtonAct(win, AG_BUTTON_HFILL, _("Start server"),
	    StartServer, "%p", tl);
	AG_ButtonAct(win, AG_BUTTON_HFILL, _("Stop server"),
	    StopServer, "%p", tl);

	AG_LabelNewStatic(win, 0, _("Connected clients:"));
	AG_ObjectAttach(win, tl);
	return (win);
}
#endif	/* DEBUG and NETWORK and THREADS */
