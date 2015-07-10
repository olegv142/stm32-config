#include "flash_sec.h"
#include "flash.h"

int flash_sec_erase(struct flash_sec const* sec)
{
	flash_erase(sec->base, 1);
	return 0;
}

int flash_sec_write(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	flash_write(sec->base + off, data, sz);
	return 0;
}

int flash_sec_write_bytes(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	flash_write_bytes(sec->base + off, data, sz);
	return 0;
}

