#include "cfg_storage.h"

#define TOMBSTONE  0x80
#define EPOCH_MASK ((uint8_t)~TOMBSTONE)

static inline uint8_t get_raw_epoch(void const* item, unsigned item_sz)
{
	return *((uint8_t const*)item + item_sz);
}

static inline uint8_t get_epoch(void const* item, unsigned item_sz)
{
	return get_raw_epoch(item, item_sz) & EPOCH_MASK;
}

static inline uint8_t epoch_next(uint8_t e)
{
	return (e + 1) & EPOCH_MASK;
}

static inline int8_t epoch_diff(uint8_t a, uint8_t b)
{
	return (int8_t)((a - b) << 1) >> 1;
}

/* Get last committed item */
void const* cfg_stor_get(struct cfg_storage const* stor)
{
	struct cfg_pool const* pool = &stor->pool[stor->epoch & 1];
	void const* item = cfg_pool_get(pool);
	if (!item || (get_raw_epoch(item, pool->item_sz - 1) & TOMBSTONE)) {
		return 0;
	}
	return item;
}

/* Initialize storage epoch. Return 0 on success, -1 on flash writing error. */
int cfg_stor_init_epoch(struct cfg_storage* stor, unsigned item_sz)
{
	void const* item[2] = {
		cfg_pool_get(&stor->pool[0]),
		cfg_pool_get(&stor->pool[1])
	};
	uint8_t epoch[2] = {
		item[0] ? get_epoch(item[0], item_sz) : TOMBSTONE,
		item[1] ? get_epoch(item[1], item_sz) : TOMBSTONE,
	};
	/* Handle empty storage case */
	if (!item[0] && !item[1]) {
		stor->epoch = 0;
		return 0;
	}
	/* Verify epoch parity */
	if (
		(item[0] && (epoch[0] & 1) != 0) ||
		(item[1] && (epoch[1] & 1) != 1)
	) {
		return cfg_stor_erase(stor);
	}
	/* First pool is empty ? */
	if (!item[0]) {
		stor->epoch = epoch[1];
		return 0;
	}
	/* Second pool is empty ? */
	if (!item[1]) {
		stor->epoch = epoch[0];
		return 0;
	}
	/* Choose most recently updated pool */
	switch (epoch_diff(epoch[1], epoch[0])) {
	case -1:
		stor->epoch = epoch[0];
		return 0;
	case 1:
		stor->epoch = epoch[1];
		return 0;
	default:
		return cfg_stor_erase(stor);
	}	
}

int cfg_stor_seal(struct cfg_storage* stor)
{
	struct cfg_pool* pool = &stor->pool[stor->epoch & 1];
	if (cfg_pool_sealed(pool)) {
		return 0;
	}
	return cfg_stor_commit(stor, cfg_pool_get(pool));
}

/* Initialize pool on boot. Return 0 on success, -1 on flash writing error. */
int cfg_stor_init(struct cfg_storage* stor, unsigned item_sz, struct flash_sec const flash[2])
{
	/* Initialize pools */
	if (
		cfg_pool_init(&stor->pool[0], item_sz + 1, &flash[0]) ||
		cfg_pool_init(&stor->pool[1], item_sz + 1, &flash[1])
	) {
		return -1;
	}
	if (cfg_stor_init_epoch(stor, item_sz)) {
		return -1;
	}
	if (cfg_stor_seal(stor)) {
		return -1;
	}
	return 0;
}

/* Commit data item */
int cfg_stor_commit(struct cfg_storage* stor, void const* data)
{
	uint8_t epoch;
	struct cfg_pool* pool = &stor->pool[stor->epoch & 1];
	if (!cfg_pool_valid(pool) && cfg_pool_erase(pool)) {
		return -1;
	}
	if (!cfg_pool_has_room(pool)) {
		/* Switch to other pool */
		stor->epoch = epoch_next(stor->epoch);
		pool = &stor->pool[stor->epoch & 1];
		if (cfg_pool_erase(pool)) {
			return -1;
		}
	}
	/* Put user data followed by epoch */
	epoch = stor->epoch;
	if (!data) {
		epoch |= TOMBSTONE;
	}
	return cfg_pool_put(pool, data, pool->item_sz - 1, &epoch);
}

/* Erase storage content. Return 0 on success, -1 on flash writing error. */
int cfg_stor_erase(struct cfg_storage* stor)
{
	if (cfg_pool_erase(&stor->pool[0]) || cfg_pool_erase(&stor->pool[1])) {
		return -1;
	}
	stor->epoch = 0;
	return 0;
}
