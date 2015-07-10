#include "flash_sec.h"
#include "flash.h"

int flash_sec_erase(struct flash_sec const* sec)
{
	return flash_erase_sec(sec->no);
}

int flash_sec_write(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	return flash_write(sec->base + off, data, sz);
}

int flash_sec_write_bytes(struct flash_sec const* sec, unsigned off, void const* data, unsigned sz)
{
	return flash_write_bytes(sec->base + off, data, sz);
}

void flash_sec_init(struct flash_sec* sec, unsigned no, unsigned base, unsigned size)
{
	sec->no = no;
	sec->base = base;
	sec->size = size;
	sec->erase = flash_sec_erase;
	sec->write = flash_sec_write;
	sec->write_bytes = flash_sec_write_bytes;
}
