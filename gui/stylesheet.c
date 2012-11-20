/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <ctype.h>

#include "widget.h"

AG_StyleSheet agDefaultCSS;

void
AG_InitStyleSheet(AG_StyleSheet *css)
{
	TAILQ_INIT(&css->blks);
}

void
AG_DestroyStyleSheet(AG_StyleSheet *css)
{
	AG_StyleBlock *blk, *blkNext;

	for (blk = TAILQ_FIRST(&css->blks);
	     blk != TAILQ_END(&css->blks);
	     blk = blkNext) {
		blkNext = TAILQ_NEXT(blk, blks);
		free(blk);
	}
}

/*
 * Load a style sheet from file and apply to the specified widget/window
 * object. If the object argument is NULL, make the style sheet the global
 * default.
 */
AG_StyleSheet *
AG_LoadStyleSheet(void *obj, const char *path)
{
	AG_Widget *tgt = obj;
	AG_StyleSheet *css;
	AG_DataSource *ds;
	size_t fileSize;
	char *buf, *s, *line;
	AG_StyleBlock *cssBlk = NULL;

	if (tgt != NULL) {
		if (tgt->css != NULL) {
			AG_DestroyStyleSheet(tgt->css);
		}
		if ((tgt->css = css = TryMalloc(sizeof(AG_StyleSheet))) == NULL)
			return (NULL);
	} else {
		AG_DestroyStyleSheet(&agDefaultCSS);
		css = &agDefaultCSS;
	}
	AG_InitStyleSheet(css);

	if ((ds = AG_OpenFile(path, "rb")) == NULL) { goto fail; }
	if (AG_Seek(ds, 0, AG_SEEK_END) == -1) { goto fail_close; }
	fileSize = AG_Tell(ds);
	if ((buf = TryMalloc(fileSize)) == NULL) { goto fail_close; }
	if (AG_Read(ds, buf, fileSize) != 0) { goto fail_close; }
	AG_CloseFile(ds);

	s = buf;
	while ((line = Strsep(&s, "\n")) != NULL) {
		char *c = &line[0], *cKey, *cVal, *t;
		size_t len;
		AG_StyleEntry *cssEnt;
		
		while (isspace(*c)) { c++; }
		if (*c == '\0' || *c == '#') { continue;  }

		if ((t = strchr(c, '{')) != NULL) {
			if (cssBlk != NULL) {
				AG_SetError("Syntax error (nested block)");
				goto fail_parse;
			}
			*t = '\0';
			if ((cssBlk = TryMalloc(sizeof(AG_StyleBlock))) == NULL) {
				goto fail_parse;
			}
			Strlcpy(cssBlk->match, c, sizeof(cssBlk->match));
			TAILQ_INIT(&cssBlk->ents);
			continue;
		} else if (strchr(c, '}') != NULL) {
			if (cssBlk == NULL) {
				AG_SetError("Syntax error (unmatched `}')");
				goto fail_parse;
			}
			TAILQ_INSERT_TAIL(&css->blks, cssBlk, blks);
			continue;
		}
		cKey = AG_Strsep(&c, ":=");
		cVal = AG_Strsep(&c, ":=");
		if (cKey == NULL || cVal == NULL) {
			continue;
		}
		while (isspace(*cKey)) { cKey++; }
		while (isspace(*cVal)) { cVal++; }
		len = strlen(cKey)-1;
		for (;;) {
			if (!isspace(cKey[len])) { break; }
			cKey[len] = '\0';
			len--;
		}
		len = strlen(cVal)-1;
		for (;;) {
			if (!isspace(cVal[len])) { break; }
			cVal[len] = '\0';
			len--;
		}
		if (*cKey == '\0' || *cVal == '\0') {
			continue;
		}
		if ((cssEnt = TryMalloc(sizeof(AG_StyleEntry))) == NULL) {
			goto fail_parse;
		}
		Strlcpy(cssEnt->key, cKey, sizeof(cssEnt->key));
		Strlcpy(cssEnt->value, cVal, sizeof(cssEnt->value));
		TAILQ_INSERT_TAIL(&cssBlk->ents, cssEnt, ents);
	}

	free(buf);
	return (css);
fail_close:
	AG_CloseFile(ds);
fail:
	free(css);
	return (NULL);
fail_parse:
	free(buf);
	AG_DestroyStyleSheet(css);
	free(css);
	return (NULL);
}

/* Lookup a style sheet entry. */
int
AG_LookupStyleSheet(AG_StyleSheet *css, void *obj, const char *key, char **rv)
{
	AG_StyleBlock *blk;
	AG_StyleEntry *ent;
	char *id;

	TAILQ_FOREACH(blk, &css->blks, blks) {
		if ((id = strchr(blk->match, '.')) && id[1] != '\0') {
			if (AG_Defined(obj, id))
				break;
		}
		if (AG_OfClass(obj, blk->match))
			break;
	}
	if (blk == NULL) {
		return (0);
	}
	TAILQ_FOREACH(ent, &blk->ents, ents) {
		if (strcmp(ent->key, key) == 0) {
			*rv = ent->value;
			return (1);
		}
	}
	return (0);
}
