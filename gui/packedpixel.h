/*	Public domain	*/

#ifndef _AGAR_GUI_PACKEDPIXEL_H_
#define _AGAR_GUI_PACKEDPIXEL_H_
#include <agar/gui/begin.h>

/*
 * Routines for manipulating general surfaces in packed-pixel
 * formats of up to 32 bits per pixel.
 */

#if AG_BYTEORDER == AG_BIG_ENDIAN

/* Extract a 32-bit pixel value (big endian) */
# define AG_PACKEDPIXEL_GET(Bpp,cDst,p) 			\
	switch (Bpp) {						\
	case 4:							\
		(cDst) = *(Uint32 *)(p);			\
		break;						\
	case 3:							\
		(cDst) = ((p)[0]<<16) + ((p)[1]<<8) + (p)[2];	\
		break;						\
	case 2:							\
		(cDst) = *(Uint16 *)(p);			\
		break;						\
	default:						\
		(cDst) = *(p);					\
		break;						\
	}

/* Write a 32-bit pixel value (big endian) */
#define AG_PACKEDPIXEL_PUT(Bpp,pDst,c)		\
	switch (Bpp) {				\
	case 4:					\
		*(Uint32 *)(pDst) = (c);	\
		break;				\
	case 3:					\
		(pDst)[0] = ((c)>>16) & 0xff;	\
		(pDst)[1] = ((c)>>8) & 0xff;	\
		(pDst)[2] = (c) & 0xff;		\
		break;				\
	case 2:					\
		*(Uint16 *)(pDst) = (c);	\
		break;				\
	default:				\
		*(pDst) = (c);			\
		break;				\
	}

#else /* AG_LITTLE_ENDIAN */

/* Extract a 32-bit pixel value (little endian) */
# define AG_PACKEDPIXEL_GET(Bpp,cDst,p) 			\
	switch (Bpp) {						\
	case 4:							\
		(cDst) = *(Uint32 *)(p);			\
		break;						\
	case 3:							\
		(cDst) = ((p)[0] + ((p)[1]<<8) + ((p)[2]<<16));	\
		break;						\
	case 2:							\
		(cDst) = *(Uint16 *)(p);			\
		break;						\
	default:						\
		(cDst) = *(p);					\
		break;						\
	}

/* Write a 32-bit pixel value (little endian) */
# define AG_PACKEDPIXEL_PUT(Bpp,pDst,c)		\
	switch (Bpp) {				\
	case 4:					\
		*(Uint32 *)(pDst) = (c);	\
		break;				\
	case 3:					\
		(pDst)[2] = ((c)>>16) & 0xff;	\
		(pDst)[1] = ((c)>>8) & 0xff;	\
		(pDst)[0] = (c) & 0xff;		\
		break;				\
	case 2:					\
		*(Uint16 *)(pDst) = (c);	\
		break;				\
	default:				\
		*(pDst) = (c);			\
		break;				\
	}

#endif /* AG_BYTEORDER */

/* Write a 32-bit pixel value at specified coordinates. */
#define AG_PACKEDPIXEL_PUT2(fb, pitch, Bpp, x,y, c) do {	\
	Uint8 *p = (Uint8 *)(fb) + (y)*(pitch) + (x)*(Bpp);	\
	AG_PACKEDPIXEL_PUT((Bpp),p,(c));			\
} while (0)

__BEGIN_DECLS
int AG_PackedPixelFlip(Uint8 *_Nonnull, Uint, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_PACKEDPIXEL_H_ */
