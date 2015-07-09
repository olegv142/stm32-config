#include "cfg_pool.h"
#include "crc16.h"
#include <stddef.h>
#include <string.h>

#define MARKER_SZ sizeof(struct cfg_rec_marker)

/* Validator values */
#define VALID   0 
#define INVALID 0xff 

/* Record status bits (active level is 0) */
#define STA_COMPLETE_BIT 1
#define STA_CHAINED_BIT 0x80

#define STA_COMPLETE    ((uint8_t)~STA_COMPLETE_BIT)
#define STA_CHAINED     ((uint8_t)~STA_CHAINED_BIT)
#define STA_UNUSED_BITS ((uint8_t)~(STA_COMPLETE_BIT|STA_CHAINED_BIT))

/* The config pool contains the array of equally sized configuration items. The implementation uses checksum to
 * ensure data integrity and addresses the problem of repeatability of reads by carefully designed record structure.
 * The problem is that in case writing of the data were interrupted the resulting content as seen by reads may by unstable
 * due to the floating gate charge being close to the threshold value between 0 and 1. As a consequence we should not rely
 * on the checksum without some additional measures against unstable content. Moreover - we should not rely on the storage
 * as being erased since it may has unstable content as a consequence of the interrupted write.
 * The pool records have the following structure:
 *
 * data | alignment | crc16 | validator | status | next data
 *                                |          |
 *                            00000000   01111110
 *                                       |      |
 *                                  complete  chained
 *                                    flag     flag
 *
 * The validator allow us to tell if the crc16 was written completely. If it has 0 bits we can be sure
 * that writing of the crc16 bytes were not interrupted. In case the complete flag is set we can be sure that
 * validator is itself valid. If the chained flag is not set we have no more data and the next byte was never
 * written.
 */

/* Check if the area past the given offset is erased */
static int cfg_pool_erased(struct cfg_pool* p, unsigned off)
{
	unsigned const *ptr = (unsigned const*)(p->flash->base + off), *end = (unsigned const*)(p->flash->base + p->flash->size);
	for (; ptr < end; ++ptr) {
		if (~*ptr)
			return 0;
	}
	return 1;
}

/* Initialize pool on boot */
int cfg_pool_validate(struct cfg_pool* p)
{
	uint8_t last_status = STA_CHAINED;
	unsigned off, base = p->flash->base;
	unsigned rec_size = p->item_sz_aligned + MARKER_SZ;
	unsigned max_off = p->flash->size - rec_size;
	int erased, valid = 0;

	/* Scan data items */
	for (off = 0; off <= max_off; off += rec_size)
	{
		unsigned addr = base + off;
		crc16_t chksum = crc16((void const*)addr, p->item_sz);
		struct cfg_rec_marker const* m = (struct cfg_rec_marker const*)(addr + p->item_sz_aligned);
		valid = m->validator == INVALID ? 0 : chksum == m->chksum;
		erased = !valid && cfg_pool_erased(p, off);
		if (erased && (last_status & STA_CHAINED_BIT)) {
			/* If chained flag is not set we never write to the next byte */
			break;
		}
		if (
			(last_status & STA_CHAINED_BIT)                         || /* Chained flag is not set on the previous item */
			(m->status & STA_UNUSED_BITS) != STA_UNUSED_BITS        || /* Unused status bits have unexpected content */
			(m->validator != INVALID && !valid)                     || /* Checksum mismatch */
			(m->validator != VALID && !(m->status & STA_COMPLETE_BIT)) /* Validator byte have unexpected content */
		) {
			/* The sector has either invalid or partially erased content */
			cfg_pool_reset(p);
			return 0;
		}
		last_status = m->status;
		p->last_off = off;
		if (valid) {
			/* Remember last valid item offset for lookups */
			p->valid_off = off;
		}
		if (erased) {
			/* Further area were never written */
			break;
		}
	}
	if (valid && (last_status & STA_COMPLETE_BIT)) {
		/* 
		 * Fixup marker to avoid unrepeatable reads. Note that we still have the repeatability problem with
		 * records treated as invalid. It can't be solved at the pool level since the only way to avoid dealing with
		 * invalid records is to commit new valid record after the invalid one. It is impossible at this level since the
		 * storage space is limited. So this solution is possible at cfg_storage level since it has 2 pools to manipulate with.
		 */
		struct cfg_rec_marker v = {
			.validator = VALID,
			.status = STA_COMPLETE
		};
		return p->flash->write_bytes(
				p->flash,
				p->last_off + p->item_sz_aligned + offsetof(struct cfg_rec_marker, validator),
				&v.validator,
				MARKER_SZ - offsetof(struct cfg_rec_marker, validator)
			);
	} else {
		return 0;
	}
}

/* Initialize pool on boot */
int cfg_pool_init(struct cfg_pool* p, unsigned item_sz, struct flash_sec const*	flash)
{
	p->item_sz = item_sz;
	p->item_sz_aligned = (item_sz + 3) & ~3;
	p->flash = flash;
	p->put_cnt = p->erase_cnt = 0;
	cfg_pool_reset(p);
	return cfg_pool_validate(p);
}

/* Physically erase pool */
int cfg_pool_erase(struct cfg_pool* p)
{
	cfg_pool_reset(p);
	if (p->flash->erase(p->flash)) {
		return -1;
	}
	++p->erase_cnt;
	return 0;
}

/* Compute CRC of the sequence of 0xff bytes */
static crc16_t crc_ff(unsigned sz)
{
	crc16_t crc = CRC16_INIT;
	for (; sz; --sz) {
		crc = crc16_up(crc, 0xff);
	}
	return crc;
}

/* Put next item. Caller may provide data in 2 parts. In case the hdr = 0 the corresponding storage
 * bytes will not be written, so they will keep 0xff values. Return 0 on success, -1 on flash writing error.
 */
int cfg_pool_put(struct cfg_pool* p, void const* hdr, unsigned hdr_sz, void const* tail)
{
	unsigned off = cfg_pool_next_offset(p);
	struct cfg_rec_marker m = {
		.chksum = hdr ? crc16(hdr, hdr_sz) : crc_ff(hdr_sz),
		.validator = VALID,
		.status = STA_COMPLETE
	};
	if (off) {
		/* Update status byte on the previous item */
		uint8_t sta = STA_CHAINED;
		if (p->flash->write_bytes(p->flash, off - 1, &sta, 1)) {
			goto err;
		}
	}
	if (hdr && p->flash->write(p->flash, off, hdr, hdr_sz)) {
		goto err;
	}
	if (hdr_sz < p->item_sz) {
		unsigned tail_sz = p->item_sz - hdr_sz;
		m.chksum = crc16_up_buff(m.chksum, tail, tail_sz);
		if (p->flash->write(p->flash, off + hdr_sz, tail, tail_sz)) {
			goto err;
		}
	}
	if (p->flash->write_bytes(p->flash, off + p->item_sz_aligned, &m, MARKER_SZ)) {
		goto err;
	}
	if (
		crc16((const void*)(p->flash->base + off), p->item_sz) != m.chksum ||
		memcmp(&m, (const void*)(p->flash->base + off + p->item_sz_aligned), sizeof(m))
	) {
		goto err;
	}
	p->last_off = p->valid_off = off;
	++p->put_cnt;
	return 0;
err:
	cfg_pool_reset(p);
	return -1;
}

/* Put data item to the pool erasing it if necessary. Return 0 on success, -1 on flash writing error. */
int cfg_pool_commit(struct cfg_pool* p, void const* data)
{
	if ((!cfg_pool_valid(p) || !cfg_pool_has_room(p)) && cfg_pool_erase(p)) {
		return -1;
	}
	return cfg_pool_put(p, data, p->item_sz, 0);
}
