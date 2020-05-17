/*
 * Copyright (c) 2010-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Loader for JPEG images via libjpeg.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/surface.h>

#include <agar/config/have_jpeg.h>
#ifdef HAVE_JPEG

#include <jpeglib.h>
#include <errno.h>
#include <setjmp.h>

struct ag_jpg_errmgr {
	struct jpeg_error_mgr errmgr;
	jmp_buf escape;
};

struct ag_jpg_sourcemgr {
	struct jpeg_source_mgr pub;
	AG_DataSource *ds;
	Uint8 buffer[4096];
};

/*
 * Callbacks
 */
static void
AG_JPG_InitSource(j_decompress_ptr cinfo)
{
	/* no-op */
}
static int
AG_JPG_FillInputBuffer(j_decompress_ptr cinfo)
{
	struct ag_jpg_sourcemgr *sm = (struct ag_jpg_sourcemgr *)cinfo->src;
	AG_Size rv;

	if (AG_ReadP(sm->ds, sm->buffer, sizeof(sm->buffer), &rv) == -1) {
		return (FALSE);
	}
	if (rv == 0) {					/* Reached EOF */
		sm->buffer[0] = 0xff;
		sm->buffer[1] = (Uint8)JPEG_EOI;
		rv = 2;
	}
	sm->pub.next_input_byte = sm->buffer;
	sm->pub.bytes_in_buffer = rv;
	return (TRUE);
}
static void
AG_JPG_SkipInputData(j_decompress_ptr cinfo, long len)
{
	struct ag_jpg_sourcemgr *sm = (struct ag_jpg_sourcemgr *)cinfo->src;

	if (len > 0) {
		while (len > (long)sm->pub.bytes_in_buffer) {
			len -= (long)sm->pub.bytes_in_buffer;
			sm->pub.fill_input_buffer(cinfo);
		}
		sm->pub.next_input_byte += (AG_Size)len;
		sm->pub.bytes_in_buffer -= (AG_Size)len;
	}
}
static void
AG_JPG_TermSource(j_decompress_ptr cinfo)
{
	/* no-op */
}
static void
AG_JPG_ErrorExit(j_common_ptr cinfo)
{
	struct ag_jpg_errmgr *err = (struct ag_jpg_errmgr *)cinfo->err;
	longjmp(err->escape, 1);
}
static void
AG_JPG_OutputMessage(j_common_ptr cinfo)
{
	/* no-op */
}

/* Load a surface from a JPEG image file. */
AG_Surface *
AG_SurfaceFromJPEG(const char *_Nonnull path)
{
	AG_DataSource *ds;
	AG_Surface *s;

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		return (NULL);
	}
	if ((s = AG_ReadSurfaceFromJPEG(ds)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		AG_CloseFile(ds);
		return (NULL);
	}
	AG_CloseFile(ds);
	return (s);
}

/* Export a surface to a JPEG image file. */
int
AG_SurfaceExportJPEG(const AG_Surface *_Nonnull S, const char *_Nonnull path,
    int quality, Uint8 flags)
{
	const Uint BytesPerPixel = S->format.BytesPerPixel;
	struct jpeg_error_mgr jerrmgr;
	struct jpeg_compress_struct jcomp;
	JSAMPROW row[1];
	Uint8 *jcopybuf;
	FILE *f;
	int x;

	if ((f = fopen(path, "wb")) == NULL) {
		AG_SetError("fdopen: %s", strerror(errno));
		return (-1);
	}

	jcomp.err = jpeg_std_error(&jerrmgr);

	jpeg_create_compress(&jcomp);

	jcomp.image_width = S->w;
	jcomp.image_height = S->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, quality, TRUE);

	if (flags & AG_EXPORT_JPEG_JDCT_ISLOW) { jcomp.dct_method = JDCT_ISLOW; }
	if (flags & AG_EXPORT_JPEG_JDCT_IFAST) { jcomp.dct_method = JDCT_IFAST; }
	if (flags & AG_EXPORT_JPEG_JDCT_FLOAT) { jcomp.dct_method = JDCT_FLOAT; }

	jpeg_stdio_dest(&jcomp, f);

	if ((jcopybuf = TryMalloc(S->w * 3)) == NULL) {
		jpeg_destroy_compress(&jcomp);
		fclose(f);
		return (-1);
	}
	
	/* TODO block copiable and optimized cases */

	jpeg_start_compress(&jcomp, TRUE);
	while (jcomp.next_scanline < jcomp.image_height) {
		Uint8 *p = S->pixels + jcomp.next_scanline*S->pitch;
		Uint8 *jp = jcopybuf;

		for (x = 0; x < S->w; x++) {
			AG_Color c;
			
			AG_GetColor(&c, AG_SurfaceGet_At(S,p), &S->format);

			*jp++ = c.r;
			*jp++ = c.g;
			*jp++ = c.b;

			p += BytesPerPixel;
		}
		row[0] = jcopybuf;
		jpeg_write_scanlines(&jcomp, row, 1);
	}
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);

	fclose(f);
	Free(jcopybuf);
	return (0);
}

/* Load surface contents from a JPEG image file. */
AG_Surface *
AG_ReadSurfaceFromJPEG(AG_DataSource *ds)
{
	struct jpeg_decompress_struct cinfo;
	JSAMPROW rowptr[1];
	AG_Surface *volatile S = NULL;
	AG_Offset start = AG_Tell(ds);
	struct ag_jpg_errmgr jerrmgr;
	struct ag_jpg_sourcemgr *sm;

	cinfo.err = jpeg_std_error(&jerrmgr.errmgr);
	jerrmgr.errmgr.error_exit = AG_JPG_ErrorExit;
	jerrmgr.errmgr.output_message = AG_JPG_OutputMessage;
	if (setjmp(jerrmgr.escape)) {
		jpeg_destroy_decompress(&cinfo);
		if (S != NULL) {
			AG_SurfaceFree(S);
		}
		AG_SetError("Error loading JPEG file");
		goto fail;
	}

	jpeg_create_decompress(&cinfo);

	if (cinfo.src == NULL) {
		cinfo.src = (struct jpeg_source_mgr *)
		    (*cinfo.mem->alloc_small)((j_common_ptr)&cinfo,
		    JPOOL_PERMANENT,
		    sizeof(struct ag_jpg_sourcemgr));
		sm = (struct ag_jpg_sourcemgr *)cinfo.src;
	}
	sm = (struct ag_jpg_sourcemgr *)cinfo.src;
	sm->ds = ds;
	sm->pub.init_source = AG_JPG_InitSource;
	sm->pub.fill_input_buffer = AG_JPG_FillInputBuffer;
	sm->pub.skip_input_data = AG_JPG_SkipInputData;
	sm->pub.resync_to_restart = jpeg_resync_to_restart;
	sm->pub.term_source = AG_JPG_TermSource;
	sm->pub.bytes_in_buffer = 0;
	sm->pub.next_input_byte = NULL;

	jpeg_read_header(&cinfo, TRUE);
	
	if (cinfo.num_components == 4) {		/* CMYK -> RGBA */
		cinfo.out_color_space = JCS_CMYK;
		cinfo.quantize_colors = FALSE;
		jpeg_calc_output_dimensions(&cinfo);

		S = AG_SurfaceRGBA(cinfo.output_width, cinfo.output_height,
		    32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		    0x0000ff00, 0x00ff0000, 0xff000000, 0x000000ff
#else
		    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
#endif
		);
		Debug(NULL,
		    "JPEG image (%ux%u RGBA) at 0x%lx (->%p) via libjpeg-%d\n",
		    S->w, S->w, (Ulong)AG_Tell(ds), S, JPEG_LIB_VERSION);
	} else {
		cinfo.out_color_space = JCS_RGB;		/* RGB */
		cinfo.quantize_colors = FALSE;
		jpeg_calc_output_dimensions(&cinfo);

		S = AG_SurfaceRGB(cinfo.output_width, cinfo.output_height,
		    24, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		    0xff0000, 0x00ff00, 0x0000ff
#else
		    0x0000ff, 0x00ff00, 0xff0000
#endif
		);
		Debug(NULL,
		    "JPEG image (%ux%u RGB) at 0x%lx (->%p) via libjpeg-%d\n",
		    S->w, S->w, (Ulong)AG_Tell(ds), S, JPEG_LIB_VERSION);
	}
	if (S == NULL) {
		jpeg_destroy_decompress(&cinfo);
		goto fail;
	}

	jpeg_start_decompress(&cinfo);
	while (cinfo.output_scanline < cinfo.output_height) {
		rowptr[0] = (JSAMPROW)S->pixels +
		    cinfo.output_scanline * S->pitch;
		jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return (S);
fail:
	AG_Seek(ds, start, AG_SEEK_SET);
	return (NULL);
}

#else /* !HAVE_JPEG */

AG_Surface *
AG_SurfaceFromJPEG(const char *path)
{
	AG_SetError(_("Agar not compiled with JPEG support"));
	return (NULL);
}
int
AG_SurfaceExportJPEG(const AG_Surface *su, const char *path, int quality,
    Uint8 flags)
{
	AG_SetError(_("Agar not compiled with JPEG support"));
	return (-1);
}
AG_Surface *
AG_ReadSurfaceFromJPEG(AG_DataSource *ds)
{
	AG_SetError(_("Agar not compiled with JPEG support"));
	return (NULL);
}

#endif /* HAVE_JPEG */
