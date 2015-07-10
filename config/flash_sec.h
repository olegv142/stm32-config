#pragma once

/*
 * Flash sector manipulation API
 */

struct flash_sec {
	unsigned no;   /* sector number */
	unsigned base; /* base address */
	unsigned size; /* sector size in bytes */
	/* The following functions are expected to return 0 on success, -1 on failure */
	int (*erase)(struct flash_sec const*);
	int (*write)(struct flash_sec const*, unsigned off, void const* data, unsigned sz);
	int (*write_bytes)(struct flash_sec const*, unsigned off, void const* data, unsigned sz);
};

int flash_sec_erase(struct flash_sec const* sec);
int flash_sec_write(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz);
int flash_sec_write_bytes(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz);

void flash_sec_init(struct flash_sec* sec, unsigned no, unsigned base, unsigned size);

#define FLASH_SEC_INITIALIZER(no, base, size) {no, base, size, flash_sec_erase, flash_sec_write, flash_sec_write_bytes}
