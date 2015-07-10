#include "cfg_test.h"
#include "cfg_storage.h"
#include "flash_sec.h"
#include "debug.h"
#include "main.h"

#include "stm32f4xx_hal.h"

#define SECTOR_SZ 0x4000 // 16k
#define SEC1_BASE (FLASH_BASE+1*SECTOR_SZ)
#define SEC2_BASE (FLASH_BASE+2*SECTOR_SZ)
#define SEC3_BASE (FLASH_BASE+3*SECTOR_SZ)
#define TEST_POOL_SZ SECTOR_SZ
#define TEST_STOR_SZ SECTOR_SZ

__no_init __root uint8_t const cfg_sec_1[SECTOR_SZ] @ SEC1_BASE;
__no_init __root uint8_t const cfg_sec_2[SECTOR_SZ] @ SEC2_BASE;
__no_init __root uint8_t const cfg_sec_3[SECTOR_SZ] @ SEC3_BASE;

struct test_item {
	uint16_t cnt;
};

#define TOUT_PRIME 3571
#define TOUT_MIN 32

void cfg_test_pool(void)
{
	int res;
	unsigned tout;
	struct test_item t = {0};
	struct flash_sec cfg_sec[2] = {
		FLASH_SEC_INITIALIZER(2, SEC2_BASE, TEST_POOL_SZ),
		FLASH_SEC_INITIALIZER(3, SEC3_BASE, TEST_POOL_SZ)
	};
	struct cfg_pool  cfg_pool[2];
	struct test_item const* t_last[2];

	res = cfg_pool_init(&cfg_pool[0], sizeof(struct test_item), &cfg_sec[0]); BUG_ON(res);
	res = cfg_pool_init(&cfg_pool[1], sizeof(struct test_item), &cfg_sec[1]); BUG_ON(res);

	t_last[0] = cfg_pool_get(&cfg_pool[0]);
	t_last[1] = cfg_pool_get(&cfg_pool[1]);

	BUG_ON(t_last[0] && t_last[1] && t_last[0]->cnt != t_last[1]->cnt && t_last[0]->cnt != t_last[1]->cnt + 1);
	
	if (t_last[0] && t_last[1]) {
		LED_Off();
	} else {
		LED_On();
	}

	if (t_last[1]) {
		t.cnt = t_last[1]->cnt;
	}
	if (t_last[0]) {
		t.cnt = t_last[0]->cnt;
	}

	tout = TOUT_PRIME * (t.cnt + 1) % 4095;
	if (tout < TOUT_MIN) {
		tout = TOUT_MIN;
	}

	hiwdg.Init.Reload = tout;
	HAL_IWDG_Init(&hiwdg);
	HAL_IWDG_Start(&hiwdg);

	for (;;) {
		res = cfg_pool_commit(&cfg_pool[0], &t); BUG_ON(res);
		res = cfg_pool_commit(&cfg_pool[1], &t); BUG_ON(res);
		t_last[0] = cfg_pool_get(&cfg_pool[0]);
		t_last[1] = cfg_pool_get(&cfg_pool[1]);
		BUG_ON(!t_last[0]);
		BUG_ON(!t_last[1]);
		BUG_ON(t_last[0]->cnt != t.cnt);
		BUG_ON(t_last[1]->cnt != t.cnt);
		++t.cnt;
	}
}

void cfg_test_storage(void)
{
	int res;
	unsigned tout;
	struct test_item t = {0};
	struct flash_sec   cfg_sec1 = FLASH_SEC_INITIALIZER(1, SEC1_BASE, TEST_POOL_SZ);
	struct flash_sec   cfg_sec[2] = {
		FLASH_SEC_INITIALIZER(2, SEC2_BASE, TEST_STOR_SZ),
		FLASH_SEC_INITIALIZER(3, SEC3_BASE, TEST_STOR_SZ)
	};
	struct cfg_pool    cfg_pool;
	struct cfg_storage cfg_stor;
	struct test_item const *p_last, *s_last;

	res = cfg_pool_init(&cfg_pool, sizeof(struct test_item), &cfg_sec1); BUG_ON(res);
	res = cfg_stor_init(&cfg_stor, sizeof(struct test_item), cfg_sec);   BUG_ON(res);

	p_last = cfg_pool_get(&cfg_pool);
	s_last = cfg_stor_get(&cfg_stor);

	BUG_ON(p_last && s_last && p_last->cnt != s_last->cnt && p_last->cnt != s_last->cnt + 1);
	BUG_ON(p_last && !s_last);

	if (p_last && s_last) {
		LED_Off();
	} else {
		LED_On();
	}

	if (s_last) {
		t.cnt = s_last->cnt;
	}
	if (p_last) {
		t.cnt = p_last->cnt;
	}

	tout = TOUT_PRIME * (t.cnt + 1) % 4095;
	if (tout < TOUT_MIN) {
		tout = TOUT_MIN;
	}

	hiwdg.Init.Reload = tout;
	HAL_IWDG_Init(&hiwdg);
	HAL_IWDG_Start(&hiwdg);

	for (;;) {
		res = cfg_pool_commit(&cfg_pool, &t); BUG_ON(res);
		res = cfg_stor_commit(&cfg_stor, &t); BUG_ON(res);
		p_last = cfg_pool_get(&cfg_pool);
		s_last = cfg_stor_get(&cfg_stor);
		BUG_ON(!p_last);
		BUG_ON(!s_last);
		BUG_ON(p_last->cnt != t.cnt);
		BUG_ON(s_last->cnt != t.cnt);
		++t.cnt;
	}
}
