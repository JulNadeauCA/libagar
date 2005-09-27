/*	$Csoft: server.c,v 1.11 2005/09/27 00:25:19 vedge Exp $	*/

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

#include <engine/engine.h>
#include <config/have_jpeg.h>
#include <config/network.h>

#if defined(DEBUG) && defined(NETWORK) && defined(THREADS)

#include <engine/view.h>
#include <engine/config.h>
#include <config/version.h>

#include <qnet/qnet.h>
#include <qnet/server.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/textbox.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mfspinbutton.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/menu.h>
#include <engine/widget/text.h>

#include "monitor.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

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
static TAILQ_HEAD(,client) clients = TAILQ_HEAD_INITIALIZER(clients);

static char *serv_host = NULL;
static char *serv_port = "1173";
static pthread_t serv_th;
static int server_inited = 0;
static int jpeg_quality = 75;

void start_server(int, union evarg *);

static void
my_exit(void)
{
	AG_ObjectSave(agWorld);
}

static void
poll_clients(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_TlistItem *it;
	struct client *cl;

	AG_TlistClear(tl);
	TAILQ_FOREACH(cl, &clients, clients) {
		it = AG_TlistAdd(tl, NULL, "%s (%s): %s", cl->name,
		    cl->hostname, cl->version);
		it->p1 = cl;
		it->class = "client";
		
	}
	AG_TlistRestore(tl);
}

static void
disconnect_client(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	struct client *cl;

	if (it == NULL) {
		return;
	}
	cl = it->p1;
}

static int
auth_password(void *p)
{
	char buf[64];

	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (-1);

	if (strcmp(buf, "foo:bar\n") == 0) {
		return (1);
	}
	qerror_set("User/password mismatch");
	return (0);
}

static void
handle_error(void)
{
	printf("1 %s\n", AG_GetError());
}

static int
cmd_version(struct command *cmd, void *p)
{
	char hostname[128];

	gethostname(hostname, sizeof(hostname));

	printf("0 agar:%s:%s\n", VERSION, hostname);
	return (0);
}

static void
error_exit(j_common_ptr jcomp)
{
	printf("1 jpeg error\n");
}

static void
output_msg(j_common_ptr jcomp)
{
	printf("1 jpeg error\n");
}

static int
cmd_surface(struct command *cmd, void *pSu)
{
#ifdef HAVE_JPEG
	static struct jpeg_error_mgr jerrmgr;
	static struct jpeg_compress_struct jcomp;
	char tmp[sizeof("/tmp/")+FILENAME_MAX];
	SDL_Surface *su;
	Uint8 *jcopybuf;
	int i, nshots = 1;
	size_t l, len;
	FILE *ftmp;
	int fd;

	if (!agView->opengl || pSu != agView->v) {
		su = pSu;
	} else {
#ifdef HAVE_OPENGL
		su = AG_CaptureGLView();
#endif
	}

	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = error_exit;
	jerrmgr.output_message = output_msg;
	
	jpeg_create_compress(&jcomp);
	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, jpeg_quality, TRUE);

	strlcpy(tmp, "/tmp/agarXXXXXXXX", sizeof(tmp));
	if ((fd = mkstemp(tmp)) == -1) {
		AG_SetError("mkstemp %s: %s", tmp, strerror(errno));
		return (-1);
	}
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

	/* Send the compressed data now that we know its size. */
	len = (size_t)ftell(ftmp);
	rewind(ftmp);
	server_binary_mode(len);
	{
		char sendbuf[BUFSIZ];
		size_t rv;

		while ((rv = fread(sendbuf, 1, sizeof(sendbuf), ftmp)) > 0) {
			fwrite(sendbuf, 1, rv, stdout);
		}
	}
	fflush(stdout);
	server_command_mode();
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

static int
cmd_view_fmt(struct command *cmd, void *p)
{
	pthread_mutex_lock(&agView->lock);
	printf("0 %s:%ux%ux%u:%08x,%08x,%08x,%08x:%d:%d\n",
	    agView->opengl ? "gl" : "",
	    agView->w, agView->h, agVideoInfo->vfmt->BitsPerPixel,
	    agVideoInfo->vfmt->Rmask,
	    agVideoInfo->vfmt->Gmask,
	    agVideoInfo->vfmt->Bmask,
	    agVideoInfo->vfmt->Amask,
	    agVideoInfo->vfmt->colorkey,
	    agVideoInfo->vfmt->alpha);
	pthread_mutex_unlock(&agView->lock);
	return (0);
}

static int
cmd_refresh(struct command *cmd, void *p)
{
	pthread_mutex_lock(&agView->lock);
	printf("0 %d:%d\n", agView->rCur, agView->rNom);
	pthread_mutex_unlock(&agView->lock);
	return (0);
}

static void
find_objs(struct command *cmd, AG_Object *pob, int depth)
{
	AG_Object *cob;

	printf("%d/%s/%s/0x%08x/%u:", depth, pob->name, pob->type,
	    pob->flags, pob->data_used);

	TAILQ_FOREACH(cob, &pob->children, cobjs) {
		find_objs(cmd, cob, depth+1);
	}
}

static int
cmd_world(struct command *cmd, void *p)
{
	AG_LockLinkage();
	fputs("0 ", stdout);
	find_objs(cmd, agWorld, 0);
	fputc('\n', stdout);
	AG_UnlockLinkage();
	return (0);
}

static void *
loop_server(void *p)
{
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Now listening on %s:%s..."),
	    serv_host == NULL ? "*" : serv_host, serv_port);

	if (server_listen("agar", VERSION, serv_host, serv_port) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Server error (%s:%s): %s"), serv_host,
		    serv_port, strerror(errno));
	}
	return (NULL);
}

static void
init_server(void)
{
	server_regauth("password", auth_password, NULL);
	server_regerr(handle_error);

	server_regcmd("version", cmd_version, NULL);
	server_regcmd("screen", cmd_surface, agView->v);
	server_regcmd("view-fmt", cmd_view_fmt, NULL);
	server_regcmd("refresh", cmd_refresh, NULL);
	server_regcmd("world", cmd_world, NULL);

	server_inited++;
}

int
AG_DebugServerStart(void)
{
	int rv;

	if (!server_inited) {
		server_inited = 1;
		init_server();
	}
	if ((rv = pthread_create(&serv_th, NULL, loop_server, NULL)) != 0) {
		AG_TextMsg(AG_MSG_ERROR, "pthread_create: %s", strerror(rv));
		return (-1);
	}
	return (0);
}

void
start_server(int argc, union evarg *argv)
{
	AG_DebugServerStart();
}

static void
stop_server(int argc, union evarg *argv)
{
}

AG_Window *
AG_DebugServerWindow(void)
{
	AG_Window *win;
	AG_Menu *me;
	AG_MenuItem *mi;
	AG_Tlist *tl;
	AG_Box *bo;
	
	win = AG_WindowNew(AG_WINDOW_NO_CLOSE, NULL);
	AG_WindowSetCaption(win, _("Agar clients"));
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_RIGHT, 0);

	tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl, AG_TLIST_POLL);
	AG_SetEvent(tl, "tlist-poll", poll_clients, NULL);
	
	me = AG_MenuNew(win);
	mi = AG_MenuAddItem(me, _("Server"));
	AG_MenuAction(mi, _("Start server"), -1, start_server, "%p", tl);
	AG_MenuAction(mi, _("Stop server"), -1, stop_server, "%p", tl);

	AG_ObjectAttach(win, tl);
	return (win);
}
#endif	/* DEBUG and NETWORK and THREADS */
