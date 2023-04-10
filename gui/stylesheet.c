/*
 * Copyright (c) 2012-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar stylesheet parser and interface to style queries.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/widget.h>
#include <agar/gui/style_data.h>

#include <ctype.h>

AG_StyleSheet agDefaultCSS;

AG_StaticCSS *agBuiltinStyles[] = {
	&agStyleDefault
};
const int agBuiltinStylesCount = sizeof(agBuiltinStyles)/sizeof(agBuiltinStyles[0]);

void
AG_InitStyleSheet(AG_StyleSheet *css)
{
	TAILQ_INIT(&css->blks);
}

void
AG_DestroyStyleSheet(AG_StyleSheet *css)
{
	AG_StyleBlock *blk, *blkNext;
	AG_StyleEntry *ent, *entNext;

	for (blk = TAILQ_FIRST(&css->blks);
	     blk != TAILQ_END(&css->blks);
	     blk = blkNext) {
		blkNext = TAILQ_NEXT(blk, blks);
		for (ent = TAILQ_FIRST(&blk->ents);
		     ent != TAILQ_END(&blk->ents);
		     ent = entNext) {
			entNext = TAILQ_NEXT(ent, ents);
			free(ent);
		}
		free(blk);
	}
	TAILQ_INIT(&css->blks);
}

/*
 * Load a style sheet and apply to the specified widget/window object. If
 * the object argument is NULL, make the style sheet the global default.
 *
 * If path begins with a "_", search for a statically-compiled stylesheet
 * of the given name (e.g., "_agStyleDefault").
 */
AG_StyleSheet *
AG_LoadStyleSheet(void *obj, const char *path)
{
	AG_Widget *tgt = obj;
	AG_StyleSheet *css;
	AG_DataSource *ds;
	AG_Size fileSize;
	char *buf, *s, *line;
	AG_StyleBlock *blk = NULL;
	int i, inComment=0;

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

	if (path[0] == '_') {                           /* Read from memory */
		AG_StaticCSS *builtin;

		for (i = 0; i < agBuiltinStylesCount; i++) {
			if (strcmp(agBuiltinStyles[i]->name, &path[1]) == 0)
				break;
		}
		if (i == agBuiltinStylesCount) {
			AG_SetErrorS(_("No such stylesheet"));
			goto fail;
		}
		builtin = agBuiltinStyles[i];
		if ((ds = AG_OpenConstCore(*builtin->data, builtin->size)) == NULL) {
			goto fail;
		}
		builtin->css = css;
		fileSize = builtin->size;
	} else {                                               /* Load file */
		if ((ds = AG_OpenFile(path, "rb")) == NULL) {
			goto fail;
		}
		if (AG_Seek(ds, 0, AG_SEEK_END) == -1) { goto fail_close; }
		fileSize = AG_Tell(ds);
		if (AG_Seek(ds, 0, AG_SEEK_SET) == -1) { goto fail_close; }
	}

	if ((buf = TryMalloc(fileSize + 1)) == NULL) {
		goto fail_close;
	}
	if (AG_Read(ds, buf, fileSize) != 0) {
		goto fail_close;
	}
	AG_CloseDataSource(ds);
	buf[fileSize] = '\0';

	s = buf;
	while ((line = Strsep(&s, "\n")) != NULL) {
		char *c = &line[0], *cKey, *cVal, *t, *cCo, *cEp;
		AG_StyleEntry *cssEnt;
		int len;
	
		while (isspace((int)*c)) {
			c++;
		}
		if (*c == '\0') {  /* || *c == '#') { */
			continue; 
		}
		if ((cCo = strstr(c, "/*")) != NULL) {   /* C-style comment */
			char *cCoEnd;

			if ((cCoEnd = strstr(&cCo[2], "*/"))) {
				memmove(cCo, &cCoEnd[2], &cCoEnd[2] - cCo);
			} else {
				inComment++;
				continue;
			}
		} else if ((cCo = strstr(c, "*/")) != NULL) {
			if (--inComment < 0) {
				AG_SetErrorS(_("Unmatched comment terminator"));
				goto fail_parse;
			}
			c = &cCo[2];
		} else if (inComment) {
			continue;
		}

		if ((t = strchr(c, '{')) != NULL) {     /* Stylesheet block */
			char *f;

			if (blk != NULL) {
				AG_SetErrorS(_("Blocks cannot be nested"));
				goto fail_parse;
			}
			while (isspace((int)t[-1])) {
				t--;
			}
			*t = '\0';

			if ((blk = TryMalloc(sizeof(AG_StyleBlock))) == NULL)
				goto fail_parse;

			Strlcpy(blk->e, c, sizeof(blk->e));

			f = strchr(blk->e, '>');                   /* E > F */
			if (f != NULL && f[1] != '\0') {
				char *fPrev = &f[-1];

				while (isspace(*fPrev)) {
					--fPrev;
				}
				fPrev++;
				*fPrev = '\0';
				f++;
				while (isspace(*f)) {
					f++;
				}
				if (*f == '"' && f[1] != '\0') {
					char *cEnd;

					f++;
					if ((cEnd = strchr(f, '"')) == NULL) {
						AG_SetErrorS(
						    _("Unterminated quote"));
						goto fail_parse;
					}
					*cEnd = '\0';

					blk->selector = AG_SELECTOR_CHILD_NAMED;
					Strlcpy(blk->f, f, sizeof(blk->f));
				} else {
					blk->selector = AG_SELECTOR_CHILD_OF_CLASS;
					Strlcpy(blk->f, f, sizeof(blk->f));
				}
			} else {
				if (strchr(blk->e, '*') != NULL) {
					blk->selector = AG_SELECTOR_CLASS_PATTERN;
				} else {
					blk->selector = AG_SELECTOR_CLASS_NAME;
				}
			}

			TAILQ_INIT(&blk->ents);
			continue;
		} else if (strchr(c, '}') != NULL) {
			if (blk == NULL) {
				AG_SetErrorS(_("Unmatched block terminator `}'"));
				goto fail_parse;
			}
			TAILQ_INSERT_TAIL(&css->blks, blk, blks);
			blk = NULL;
			continue;
		}

		cKey = Strsep(&c, ":=");
		cVal = Strsep(&c, ":=");

		if (cKey == NULL || cVal == NULL) {
			continue;
		}
		while (isspace((int)*cKey)) { cKey++; }
		while (isspace((int)*cVal)) { cVal++; }

		len = (int)strlen(cKey)-1;
		for (;;) {
			if (!isspace((int)cKey[len])) { break; }
			cKey[len] = '\0';
			len--;
		}
		len = strlen(cVal)-1;
		for (;;) {
			if (!isspace((int)cVal[len])) { break; }
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

		if ((cEp = strchr(cssEnt->value, ';')) != NULL) {
			*cEp = '\0';
		}
		if (blk == NULL) {
			AG_SetError(_("Statement outside of block: \"%s\""),
			    cssEnt->key);
			free(cssEnt);
			goto fail_parse;
		}
		TAILQ_INSERT_TAIL(&blk->ents, cssEnt, ents);
	}

	free(buf);
	return (css);
fail_close:
	AG_CloseFile(ds);
fail:
	if (css != &agDefaultCSS) { free(css); }
	return (NULL);
fail_parse:
	AG_SetError(_("Syntax error: %s"), AG_GetError());
	free(buf);
	AG_DestroyStyleSheet(css);
	if (css != &agDefaultCSS) { free(css); }
	return (NULL);
}

/*
 * Lookup a style sheet entry.
 */
int
AG_LookupStyleSheet(AG_StyleSheet *_Nonnull css, void *_Nonnull obj,
    const char *_Nonnull key, char *_Nonnull *_Nonnull rv)
{
	AG_ObjectClass **hier;
	AG_StyleBlock *blk;
	AG_StyleEntry *ent;
	const char *clName;
	const AG_Object *parent = OBJECT(obj)->parent;
	int nHier;

	if (AG_ObjectGetInheritHier(obj, &hier, &nHier) != 0) {
		return (0);
	}
	clName = hier[nHier - 1]->name;

	TAILQ_FOREACH(blk, &css->blks, blks) {   /* E>F (higher precedence) */
		if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, OBJECT(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0) {
				break;
			}
		} else if (blk->selector == AG_SELECTOR_CHILD_OF_CLASS) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, AGOBJECT_CLASS(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0) {
				break;
			}
		}
	}

	if (blk == NULL) {                     /* Just E (lower precedence) */
		TAILQ_FOREACH(blk, &css->blks, blks) {
			if (blk->selector == AG_SELECTOR_CLASS_NAME) {
				if (strcmp(blk->e, clName) == 0) {
					break;
				}
			} else if (blk->selector == AG_SELECTOR_CLASS_PATTERN) {
				if (AG_OfClass(obj, blk->e)) {
					break;
				}
			}
		}
	 	if (blk == NULL)
			goto fail;
	}

	TAILQ_FOREACH(ent, &blk->ents, ents) {
		if (strcmp(ent->key, key) == 0) {
			*rv = ent->value;
			break;
		}
	}
	if (ent == NULL) {
		goto fail;
	}
	free(hier);
	return (1);
fail:
	free(hier);
	return (0);
}
