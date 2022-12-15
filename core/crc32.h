/*-
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 *
 * $FreeBSD: stable/12/lib/libproc/crc32.h 309593 2016-12-06 04:18:09Z markj $
 */
#ifndef _AGAR_CORE_CRC32_H_
#define _AGAR_CORE_CRC32_H_

#include <agar/gui/begin.h>
#include <agar/config/ag_model.h>
#if AG_MODEL != AG_SMALL

__BEGIN_DECLS
extern Uint32 agCRC32_Table[];

/*
 * Calculate CRC32 checksum of a buffer and given size in bytes.
 * TODO: Architecture-specific optimizations.
 */
static __inline__ Uint32
AG_GetCRC32(const void *_Nonnull buf, AG_Size size)
{
	const Uint8 *p = buf;
	Uint32 crc;

	crc = ~0U;
	while (size--) {
		crc = agCRC32_Table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	}
	return (crc ^ ~0U);
}
__END_DECLS

#endif /* !AG_SMALL */
#include <agar/gui/close.h>

#endif /* _AGAR_CORE_CRC32_H_ */
