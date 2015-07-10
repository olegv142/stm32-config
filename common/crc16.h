#pragma once

#define CRC16_INIT 0xffff
#define CRC16_CHK_STR "123456789"
#define CRC16_CHK_VALUE 0x6F91

typedef unsigned short crc16_t;

crc16_t crc16_up(crc16_t crc, unsigned char data);
crc16_t crc16_up_buff(crc16_t crc, const void* data, unsigned len);
crc16_t crc16_str(const char* str);

static inline crc16_t crc16(const void* data, unsigned len)
{
	return crc16_up_buff(CRC16_INIT, data, len);
}
