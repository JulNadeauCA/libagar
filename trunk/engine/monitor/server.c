/*	$Csoft: server.c,v 1.4 2005/01/28 12:50:59 vedge Exp $	*/

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
#include <config/have_libqnet.h>

#if defined(DEBUG) && defined(HAVE_LIBQNET)

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
	struct window *win;
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
	object_save(world);
}

static void
poll_clients(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it;
	struct client *cl;

	tlist_clear_items(tl);
	TAILQ_FOREACH(cl, &clients, clients) {
		it = tlist_insert(tl, NULL, "%s (%s): %s", cl->name,
		    cl->hostname, cl->version);
		it->p1 = cl;
		it->class = "client";
		
	}
	tlist_restore_selections(tl);
}

static void
disconnect_client(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it = tlist_item_selected(tl);
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
	printf("1 %s\n", error_get());
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
cmd_surface(struct command *cmd, void *p)
{
#ifdef HAVE_JPEG
	static struct jpeg_error_mgr jerrmgr;
	static struct jpeg_compress_struct jcomp;
	char tmp[sizeof("/tmp/")+FILENAME_MAX];
	SDL_Surface *srcsu = p;
	Uint8 *jcopybuf;
	int i, nshots = 1;
	size_t l, len;
	FILE *ftmp;
	int fd;

	jcomp.err = jpeg_std_error(&jerrmgr);
	jerrmgr.error_exit = error_exit;
	jerrmgr.output_message = output_msg;
	
	jpeg_create_compress(&jcomp);
	jcomp.image_width = srcsu->w;
	jcomp.image_height = srcsu->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, jpeg_quality, TRUE);

	strlcpy(tmp, "/tmp/agarXXXXXXXX", sizeof(tmp));
	if ((fd = mkstemp(tmp)) == -1) {
		error_set("mkstemp %s: %s", tmp, strerror(errno));
		return (-1);
	}
	if ((ftmp = fdopen(fd, "r+")) == NULL) {
		error_set("fdopen %s: %s", tmp, strerror(errno));
		return (-1);
	}
	jpeg_stdio_dest(&jcomp, ftmp);

	jcopybuf = Malloc(srcsu->w * 3, M_VIEW);
	if (SDL_MUSTLOCK(srcsu)) {
		SDL_LockSurface(srcsu);
	}
	for (i = 0; i < nshots; i++) {
		JSAMPROW row[1];
		int x;

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
	}
	if (SDL_MUSTLOCK(srcsu))
		SDL_UnlockSurface(srcsu);

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
	error_set("libjpeg is not available");
	return (-1);
#endif /* HAVE_JPEG */
}

static int
cmd_view_fmt(struct command *cmd, void *p)
{
	pthread_mutex_lock(&view->lock);
	printf("0 %d:%s:%ux%ux%u:%08x,%08x,%08x,%08x:%d:%d\n",
	    view->gfx_engine,
	    view->opengl ? "gl" : "",
	    view->w, view->h, vinfo->vfmt->BitsPerPixel,
	    vinfo->vfmt->Rmask,
	    vinfo->vfmt->Gmask,
	    vinfo->vfmt->Bmask,
	    vinfo->vfmt->Amask,
	    vinfo->vfmt->colorkey,
	    vinfo->vfmt->alpha);
	pthread_mutex_unlock(&view->lock);
	return (0);
}

static int
cmd_refresh(struct command *cmd, void *p)
{
	pthread_mutex_lock(&view->lock);
	printf("0 %d:%d\n", view->refresh.r, view->refresh.rnom);
	pthread_mutex_unlock(&view->lock);
	return (0);
}

static void
find_objs(struct command *cmd, struct object *pob, int depth)
{
	struct object *cob;

	printf("%d/%s/%s/0x%08x/%u:", depth, pob->name, pob->type,
	    pob->flags, pob->data_used);

	TAILQ_FOREACH(cob, &pob->children, cobjs) {
		find_objs(cmd, cob, depth+1);
	}
}

static int
cmd_world(struct command *cmd, void *p)
{
	lock_linkage();
	fputs("0 ", stdout);
	find_objs(cmd, world, 0);
	fputc('\n', stdout);
	unlock_linkage();
	return (0);
}

static void *
loop_server(void *p)
{
	text_tmsg(MSG_INFO, 1000, _("Now listening on %s:%s..."),
	    serv_host == NULL ? "*" : serv_host, serv_port);

	if (server_listen("agar", VERSION, serv_host, serv_port) == -1) {
		text_msg(MSG_ERROR, _("Server error (%s:%s): %s"), serv_host,
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
	server_regcmd("screen", cmd_surface, view->v);
	server_regcmd("view-fmt", cmd_view_fmt, NULL);
	server_regcmd("refresh", cmd_refresh, NULL);
	server_regcmd("world", cmd_world, NULL);

	server_inited++;
}

int
server_start(void)
{
	int rv;

	if (!server_inited) {
		server_inited = 1;
		init_server();
	}
	if ((rv = pthread_create(&serv_th, NULL, loop_server, NULL)) != 0) {
		text_msg(MSG_ERROR, "pthread_create: %s", strerror(rv));
		return (-1);
	}
	return (0);
}

void
start_server(int argc, union evarg *argv)
{
	server_start();
}

static void
stop_server(int argc, union evarg *argv)
{
}

struct window *
server_window(void)
{
	struct window *win;
	struct AGMenu *me;
	struct AGMenuItem *mi;
	struct tlist *tl;
	struct box *bo;
	
	win = window_new(WINDOW_NO_CLOSE, NULL);
	window_set_caption(win, _("Agar clients"));
	window_set_position(win, WINDOW_LOWER_RIGHT, 0);

	tl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl, TLIST_POLL);
	event_new(tl, "tlist-poll", poll_clients, NULL);
	
	me = menu_new(win);
	mi = menu_add_item(me, _("Server"));
	menu_action(mi, _("Start server"), -1, start_server, "%p", tl);
	menu_action(mi, _("Stop server"), -1, stop_server, "%p", tl);

	object_attach(win, tl);
	return (win);
}
#endif	/* DEBUG and LIBQNET */
