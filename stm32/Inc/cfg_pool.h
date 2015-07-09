#pragma once

#include "flash_sec.h"
#include <stdint.h>

/* The config pool contains the array of equally sized configuration items */
struct cfg_pool {
	unsigned		item_sz;
	unsigned		item_sz_aligned;
	int			last_off;
	int			valid_off;
	unsigned		put_cnt;
	unsigned		erase_cnt;
	struct flash_sec const*	flash;
};

/* Record marker is written after data to control integrity */
struct cfg_rec_marker {
	uint16_t chksum;
	uint8_t  validator;
	uint8_t  status;	
};

/* Return 1 if the pool is empty, 0 otherwise */
static inline int cfg_pool_empty(struct cfg_pool const* p)
{
	return p->last_off < 0;
}

/* Return 1 if the pool has valid items, 0 otherwise */
static inline int cfg_pool_valid(struct cfg_pool const* p)
{
	return p->valid_off >= 0;
}

/* The last item of the sealed pool is valid. So we can guarantee that cfg_pool_get always return the same value. */
static inline int cfg_pool_sealed(struct cfg_pool const* p)
{
	return p->valid_off >= 0 && p->valid_off == p->last_off;
}

/* Returns pointer to the last valid data item */
static inline void const* cfg_pool_get(struct cfg_pool const* p)
{
	return cfg_pool_valid(p) ? (void const*)(p->flash->base + p->valid_off) : (void const*)0;
}

/* Return offset of the next item */
static inline unsigned cfg_pool_next_offset(struct cfg_pool* p)
{
	return cfg_pool_empty(p) ? 0 : p->last_off + p->item_sz_aligned + sizeof(struct cfg_rec_marker);
}

/* Check if we have space for the next item */
static inline int cfg_pool_has_room(struct cfg_pool* p)
{
	return cfg_pool_next_offset(p) + p->item_sz_aligned + sizeof(struct cfg_rec_marker) <= p->flash->size;
}

/* Reset pool state to empty */
static inline void cfg_pool_reset(struct cfg_pool* p)
{
	p->last_off = p->valid_off = -1;
}

/* Physically erase pool. Return 0 on success, -1 on flash erase error. */
int cfg_pool_erase(struct cfg_pool* p);

/* Initialize pool on boot. Return 0 on success, -1 on flash writing error. */
int cfg_pool_init(struct cfg_pool* p, unsigned item_sz, struct flash_sec const*	flash);

/* Put next item. Caller may provide data in 2 parts. In case the hdr = 0 the corresponding storage
 * bytes will not be written, so they will keep 0xff values. Return 0 on success, -1 on flash writing error.
 */
int cfg_pool_put(struct cfg_pool* p, void const* hdr, unsigned hdr_sz, void const* tail);

/* Put data item to the pool erasing it if necessary. Return 0 on success, -1 on flash writing error. */
int cfg_pool_commit(struct cfg_pool* p, void const* data);
