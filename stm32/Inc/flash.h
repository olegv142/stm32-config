#pragma once

int flash_erase_sec(int sec_no);
int flash_write(unsigned addr, void const* data, unsigned sz);
int flash_write_bytes(unsigned addr, void const* data, unsigned sz);

