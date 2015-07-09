#pragma once

#include "cfg_pool.h"

/*
 * Configuration storage with atomic updates
 */

struct cfg_storage {
	struct cfg_pool	pool[2];
	uint8_t		epoch;
};

/* Initialize pool on boot. Return 0 on success, -1 on flash writing error. */
int cfg_stor_init(struct cfg_storage* stor, unsigned item_sz, struct flash_sec const flash[2]);

/* Get last committed item */
void const* cfg_stor_get(struct cfg_storage const* stor);

/* Commit data item. Return 0 on success, -1 on flash writing error. */
int cfg_stor_commit(struct cfg_storage* stor, void const* data);

/* Erase storage content. Return 0 on success, -1 on flash writing error. */
int cfg_stor_erase(struct cfg_storage* stor);
