/*	$Csoft: mediasel.c,v 1.2 2004/03/12 03:03:58 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include <config/edition.h>

#ifdef EDITION

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/loader/den.h>

#include <sys/stat.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "mediasel.h"

static void
refresh_pushed(int argc, union evarg *argv)
{
	struct mediasel *msel = argv[1].p;

	mediasel_refresh(msel);
}

/* Select a new graphics package to associate with the object. */
static void
media_selected(int argc, union evarg *argv)
{
	struct mediasel *msel = argv[1].p;
	struct tlist_item *it = argv[2].p;
	int was_used = 0;

	switch (msel->type) {
	case MEDIASEL_GFX:
		if (msel->obj->gfx != NULL) {
			gfx_unused(msel->obj->gfx);
			msel->obj->gfx = NULL;
			was_used++;
		}
		if (it->text[0] == '\0') {
			return;
		}
		if ((msel->obj->gfx = gfx_fetch(it->text)) == NULL) {
			text_msg(MSG_ERROR, "%s: %s", msel->obj->gfx_name,
			    error_get());
			return;
		}
		if (was_used) {
			gfx_unused(msel->obj->gfx);
			msel->obj->gfx = NULL;
		}
		msel->obj->gfx_name = Strdup(it->text);
		break;
	case MEDIASEL_AUDIO:
		if (msel->obj->audio_name != NULL) {
			free(msel->obj->audio_name);
			msel->obj->audio_name = NULL;
			was_used++;
		}
		if (it->text[0] == '\0') {
			return;
		}
		if ((msel->obj->audio = audio_fetch(it->text)) == NULL) {
			text_msg(MSG_ERROR, "%s: %s", msel->obj->audio_name,
			    error_get());
			return;
		}
		if (was_used) {
			audio_unused(msel->obj->audio);
			msel->obj->audio = NULL;
		}
		msel->obj->audio_name = Strdup(it->text);
		break;
	}
}

struct mediasel *
mediasel_new(void *parent, enum mediasel_type type, struct object *obj)
{
	struct box *bo;
	struct mediasel *msel;
	const char *label = NULL;

	switch (type) {
	case MEDIASEL_GFX:
		label = _("Graphics: ");
		break;
	case MEDIASEL_AUDIO:
		label = _("Audio: ");
		break;
	}

	bo = box_new(parent, BOX_HORIZ, BOX_WFILL);
	box_set_spacing(bo, 0);
	box_set_padding(bo, 0);

	msel = Malloc(sizeof(struct mediasel));
	msel->type = type;
	msel->obj = obj;

	msel->com = combo_new(bo, 0, label);
	textbox_prescale(msel->com->tbox, "XXXXXXXXXXXXXXXXX");
	event_new(msel->com, "combo-selected", media_selected, "%p", msel);

	msel->rfbu = button_new(bo, _("Refresh"));
	event_new(msel->rfbu, "button-pushed", refresh_pushed, "%p", msel);
	
	mediasel_refresh(msel);
	return (msel);
}

void
mediasel_refresh(struct mediasel *msel)
{
	char den_path[MAXPATHLEN];
	char *dir, *last;
	const char *hint = NULL;
	
	switch (msel->type) {
	case MEDIASEL_GFX:
		if (msel->obj->gfx_name != NULL) {
			textbox_printf(msel->com->tbox, "%s",
			    msel->obj->gfx_name);
		}
		hint = "gfx";
		break;
	case MEDIASEL_AUDIO:
		if (msel->obj->audio_name != NULL) {
			textbox_printf(msel->com->tbox, "%s",
			    msel->obj->audio_name);
		}
		hint = "audio";
		break;
	}

	tlist_clear_items(msel->com->list);

	prop_copy_string(config, "den-path", den_path, sizeof(den_path));

	for (dir = strtok_r(den_path, ":", &last);
	     dir != NULL;
	     dir = strtok_r(NULL, ":", &last)) {
		char cwd[MAXPATHLEN];

		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			text_msg(MSG_ERROR, "getcwd: %s", strerror(errno));
			continue;
		}

		if (chdir(dir) == 0) {
			mediasel_scan_dens(msel, hint, "");
			if (chdir(cwd) == -1) {
				text_msg(MSG_ERROR, "chdir %s: %s", cwd,
				    strerror(errno));
				continue;
			}
		}
	}
}

/* Search subdirectories for .den files containing a given hint. */
void
mediasel_scan_dens(struct mediasel *msel, const char *hint, const char *spath)
{
	DIR *di;
	struct dirent *dent;

	if ((di = opendir(".")) == NULL) {
		text_msg(MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	while ((dent = readdir(di)) != NULL) {
		char path[MAXPATHLEN];
		struct stat sta;
		char *ext;

		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		if (stat(dent->d_name, &sta) == -1) {
			dprintf("%s: %s\n", dent->d_name, strerror(errno));
			continue;
		}

		strlcpy(path, spath, sizeof(path));
		strlcat(path, "/", sizeof(path));
		strlcat(path, dent->d_name, sizeof(path));

		if ((sta.st_mode & S_IFREG) &&
		    (ext = strrchr(dent->d_name, '.')) != NULL &&
		    strcasecmp(ext, ".den") == 0) {
			struct den *den;

			if ((den = den_open(dent->d_name, DEN_READ)) != NULL) {
				if (strcmp(den->hint, hint) == 0 &&
				    (ext = strrchr(path, '.')) != NULL) {
					*ext = '\0';
					tlist_insert_item(msel->com->list, NULL,
					    path, NULL);
					/* TODO icon */
				}
				den_close(den);
			}
		} else if (sta.st_mode & S_IFDIR) {
			if (chdir(dent->d_name) == 0) {
				mediasel_scan_dens(msel, hint, path);
				if (chdir("..") == -1) {
					text_msg(MSG_ERROR, "..: %s",
					    strerror(errno));
					closedir(di);
					return;
				}
			}
		}
	}
	closedir(di);
}

#endif	/* EDITION */
