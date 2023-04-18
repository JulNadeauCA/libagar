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
#include <agar/gui/window.h>
#include <agar/gui/style_data.h>

#include <ctype.h>

/* #define DEBUG_CSS */

AG_StyleSheet agDefaultCSS;

AG_StaticCSS *agBuiltinStyles[] = {
	&agStyleDefault
};
const int agBuiltinStylesCount = sizeof(agBuiltinStyles)/sizeof(agBuiltinStyles[0]);

void
AG_InitStyleSheet(AG_StyleSheet *css)
{
	TAILQ_INIT(&css->blks);
	TAILQ_INIT(&css->blksCond);
}

static __inline__ void
FreeStyleSheetBlock(AG_StyleBlock *blk)
{
	AG_StyleEntry *ent, *entNext;

	for (ent = TAILQ_FIRST(&blk->ents);
	     ent != TAILQ_END(&blk->ents);
	     ent = entNext) {
		entNext = TAILQ_NEXT(ent, ents);
		free(ent);
	}
	free(blk);
}

void
AG_DestroyStyleSheet(AG_StyleSheet *css)
{
	AG_StyleBlock *blk, *blkNext;

	for (blk = TAILQ_FIRST(&css->blksCond);
	     blk != TAILQ_END(&css->blksCond);
	     blk = blkNext) {
		blkNext = TAILQ_NEXT(blk, blks);
		FreeStyleSheetBlock(blk);
	}
	for (blk = TAILQ_FIRST(&css->blks);
	     blk != TAILQ_END(&css->blks);
	     blk = blkNext) {
		blkNext = TAILQ_NEXT(blk, blks);
		FreeStyleSheetBlock(blk);
	}
}

/* Variant of strchr() which ignores any occurences between '(' and ')'. */
static __inline__ char *
Strchr_NotInParens(const char *p, char ch)
{
	int inParens = 0;

	for (; ; ++p) {
		if (*p == '(') {
			++inParens;
		} else if (*p == ')') {
			--inParens;
		}
		if (inParens > 0) {
			continue;
		}
		if (*p == ch) {
			return (char *)(p);
		} else if (*p == '\0') {
			return (NULL);
		}
	}
	/* NOTREACHED */
}

static void
ParseCond(AG_StyleBlock *blk, char *cond, enum ag_style_condition_type condType)
{
	while (isspace(*cond))
		++cond;

	blk->cond = condType;

	if (cond[0] == '<') {
		blk->x = 0;
		if (cond[1] == '=') {
			blk->y = (int)strtol(&cond[2], NULL, 10);
		} else {
			blk->y = (int)strtol(&cond[1], NULL, 10) - 1;
		}
	} else if (cond[0] == '>') {
		if (cond[1] == '=') {
			blk->x = (int)strtol(&cond[2], NULL, 10);
		} else {
			blk->x = (int)strtol(&cond[1], NULL, 10) + 1;
		}
		blk->y = (AG_INT_MAX - 1);
	} else if (cond[0] == '=') {
		char *ep;

		blk->x = (int)strtol(&cond[1], &ep, 10);
		if (ep != NULL && ep[0] == '-' && ep[1] != '\0') {
			blk->y = (int)strtol(&ep[1], NULL, 10);
		} else {
			blk->y = blk->x;
		}
	} else {
#ifdef DEBUG_CSS
		Debug(NULL, "CSS Syntax Error (near condition \"%s\")\n", cond);
#endif
		blk->cond = AG_SELECTOR_COND_NONE;
	}
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
			char *f, *cond;

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

			if ((cond = strchr(c, '(')) != NULL) {
				++cond;
				if (strncmp(cond, "width",5) == 0) {
					ParseCond(blk, &cond[5],
					    AG_SELECTOR_COND_WIDTH);
				} else if (strncmp(cond, "height",6) == 0) {
					ParseCond(blk, &cond[6],
					    AG_SELECTOR_COND_HEIGHT);
				} else if (strncmp(cond, "zoom",4) == 0) {
					ParseCond(blk, &cond[4],
					    AG_SELECTOR_COND_ZOOM);
				} else {
					AG_SetErrorS(_("Bad conditional"));
					goto fail_parse;
				}
			} else {
				blk->cond = AG_SELECTOR_COND_NONE;
			}
			
			Strlcpy(blk->e, c, sizeof(blk->e));

			if (blk->cond != AG_SELECTOR_COND_NONE &&
			    ((cond = strstr(blk->e, " (")) != NULL ||
			     (cond = strchr(blk->e, '(')) != NULL))
				*cond = '\0';

			f = Strchr_NotInParens(blk->e, '>');       /* E > F */
			if (f != NULL && f[1] != '\0') {
				char *fPrev = (char *)&f[-1];

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
			if (blk->cond != AG_SELECTOR_COND_NONE) {
				TAILQ_INSERT_TAIL(&css->blksCond, blk, blks);
			} else {
				TAILQ_INSERT_TAIL(&css->blks, blk, blks);
			}
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

/* Test the condition of a stylesheet block against widget obj. */
static __inline__ int
TestSelectorCondition(AG_StyleBlock *blk, void *obj)
{
	switch (blk->cond) {
	case AG_SELECTOR_COND_ZOOM:
		return (WIDGET(obj)->window->zoom >= blk->x &&
		        WIDGET(obj)->window->zoom <= blk->y);
	case AG_SELECTOR_COND_WIDTH:
		return (WIDTH(obj) >= blk->x &&
		        WIDTH(obj) <= blk->y);
	case AG_SELECTOR_COND_HEIGHT:
		return (HEIGHT(obj) >= blk->x &&
		        HEIGHT(obj) <= blk->y);
	default:
		break;
	}
	return (0);
}

/*
 * Search a style sheet for an attribute "key" applicable to widget obj
 * (given its current geometry and the zoom level of its parent window).
 *
 * Returns a pointer to a read-only (internally-managed) string into rv.
 * Return 1 on success or 0 if the attribute was not found. A pointer may
 * be written to rv even in the case of an unsuccessful lookup.
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

	/* Conditional Selector `E > F' */
	TAILQ_FOREACH(blk, &css->blksCond, blks) {
		if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, OBJECT(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0 &&
			    TestSelectorCondition(blk, obj)) {
				break;
			}
		} else if (blk->selector == AG_SELECTOR_CHILD_OF_CLASS) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, AGOBJECT_CLASS(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0 &&
			    TestSelectorCondition(blk, obj)) {
				break;
			}
		}
	}
	if (blk != NULL) {
#ifdef DEBUG_CSS
		Debug(obj, "CSS (%s > %s) Cond#%d (%d - %d)\n",
		    blk->e, blk->f, blk->cond, blk->x, blk->y);
#endif
		TAILQ_FOREACH(ent, &blk->ents, ents) {
			if (strcmp(ent->key, key) == 0) {
				*rv = ent->value;
				break;
			}
		}
		if (ent != NULL) {
			goto out;                            /* Match found */
		} else {
			goto match_uncond_EF;
		}
	}

	/* Conditional Selector `E' */
	TAILQ_FOREACH(blk, &css->blksCond, blks) {
		if (blk->selector == AG_SELECTOR_CLASS_NAME) {
			if (strcmp(blk->e, clName) == 0 &&
			    TestSelectorCondition(blk, obj)) {
				break;
			}
		} else if (blk->selector == AG_SELECTOR_CLASS_PATTERN) {
			if (AG_OfClass(obj, blk->e) &&
			    TestSelectorCondition(blk, obj)) {
				break;
			}
		}
	}
	if (blk != NULL) {
#ifdef DEBUG_CSS
		Debug(obj, "CSS (%s) Cond#%d (%d - %d)\n",
		    blk->e, blk->cond, blk->x, blk->y);
#endif
		TAILQ_FOREACH(ent, &blk->ents, ents) {
			if (strcmp(ent->key, key) == 0) {
				*rv = ent->value;
				break;
			}
		}
		if (ent != NULL) {
			goto out;                            /* Match found */
		} else {
			goto match_uncond_E;   /* Try `E' with no condition */
		}
	}

match_uncond_EF:
	/* Unconditional Selector `E > F' */
	TAILQ_FOREACH(blk, &css->blks, blks) {
		if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, OBJECT(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0)
				break;
		} else if (blk->selector == AG_SELECTOR_CHILD_OF_CLASS) {
			if (parent == NULL) {
				continue;
			}
			if (strcmp(blk->f, AGOBJECT_CLASS(obj)->name) == 0 &&
			    strcmp(blk->e, AGOBJECT_CLASS(parent)->name) == 0)
				break;
		}
	}
	if (blk != NULL) {
		TAILQ_FOREACH(ent, &blk->ents, ents) {
			if (strcmp(ent->key, key) == 0) {
				*rv = ent->value;
				break;
			}
		}
		if (ent != NULL)
			goto out;                            /* Match found */
	}

match_uncond_E:
	/* Unconditional Selector `E' */
	TAILQ_FOREACH(blk, &css->blks, blks) {
		if (blk->selector == AG_SELECTOR_CLASS_NAME) {
			if (strcmp(blk->e, clName) == 0)
				break;
		} else if (blk->selector == AG_SELECTOR_CLASS_PATTERN) {
			if (AG_OfClass(obj, blk->e))
				break;
		}
	}
 	if (blk == NULL) {
		goto fail;
	}
	TAILQ_FOREACH(ent, &blk->ents, ents) {
		if (strcmp(ent->key, key) == 0) {
			*rv = ent->value;
			break;
		}
	}
	if (ent == NULL)
		goto fail;
out:
	free(hier);
	return (1);
fail:
	free(hier);
	return (0);
}
