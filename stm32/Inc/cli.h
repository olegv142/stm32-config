#pragma once

#include <stdint.h>

int8_t cli_receive(uint8_t* Buf, uint32_t *Len);
void   cli_run(void);