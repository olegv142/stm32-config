#pragma once

#include "io430.h"

#define FLASH_SEG_SZ 512

static inline void flash_unlock(void)
{
	FCTL2 = FWKEY|FSSEL1|FN1; // Configure clock: SMCLK/3 (optimal for 1MHz clock)
	FCTL3 = FWKEY;            // Clear Lock bit
}

static inline void flash_lock(void)
{
	FCTL3 = FWKEY + LOCK; // Set Lock bit
}

static inline void flash_wr_enable(void)
{
	FCTL1 = FWKEY + WRT; // Set Write bit
}

static inline void flash_wr_disable(void)
{
	FCTL1 = FWKEY; // Clear Write bit
}

static inline void flash_wait(void)
{
	// Since we are executing code from the flash wait is not required
	// The CPU just stops till operation completed
	// while (FCTL3 & BUSY) __no_operation();
}

static inline void flash_erase(unsigned base, unsigned nsegs)
{
	unsigned i;
	char *ptr = (char*)base;
	flash_wait();
	flash_unlock();
	for (i = 0; i < nsegs; ++i) {
		FCTL1 = FWKEY + ERASE;	// Set Erase bit
		// Dummy write to erase segment
		*ptr = 0;
		ptr += FLASH_SEG_SZ;
		flash_wait();
	}
	flash_lock();
}

// Here we are expecting the address to be aligned but the size may be not 
static inline void flash_write(unsigned addr, void const* data, unsigned sz)
{
	flash_wait();
	flash_unlock();
	flash_wr_enable();
	for (; addr % sizeof(unsigned) && sz >= 1; sz -= 1, addr += 1) {
		// Write data to flash
		*(unsigned char*)addr = *(unsigned char const*)data;
		data = (unsigned char const*)data + 1;
		flash_wait();
	}
	for (; sz >= sizeof(unsigned); sz -= sizeof(unsigned), addr += sizeof(unsigned)) {
		// Write data to flash
		*(unsigned*)addr = *(unsigned const*)data;
		data = (unsigned const*)data + 1;
		flash_wait();
	}
	for (; sz >= 1; sz -= 1, addr += 1) {
		// Write data to flash
		*(unsigned char*)addr = *(unsigned char const*)data;
		data = (unsigned char const*)data + 1;
		flash_wait();
	}
	flash_wr_disable();
	flash_lock();
}

// Here we are expecting the address to be aligned but the size may be not 
static inline void flash_write_bytes(unsigned addr, void const* data, unsigned sz)
{
	flash_wait();
	flash_unlock();
	flash_wr_enable();
	for (; sz >= 1; sz -= 1, addr += 1) {
		// Write data to flash
		*(unsigned char*)addr = *(unsigned char const*)data;
		data = (unsigned char const*)data + 1;
		flash_wait();
	}
	flash_wr_disable();
	flash_lock();
}
