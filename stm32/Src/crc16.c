#include "crc16.h"

// CRC16 CCITT  version
static inline crc16_t crc16_up_(crc16_t crc, unsigned char data)
{
	crc16_t t;
	data ^= crc & 0xff;
	data ^= data << 4;
	t = (((crc16_t)data << 8) | ((crc>>8) & 0xff));
	t ^= (unsigned char)(data >> 4);
	t ^= ((crc16_t)data << 3);
	return t;
}

crc16_t crc16_up(crc16_t crc, unsigned char data)
{
	return crc16_up_(crc, data);
}

crc16_t crc16_up_buff(crc16_t crc, const void* data, unsigned len)
{
	const unsigned char* ptr = data;
	for (; len; ++ptr, --len)
		crc = crc16_up_(crc, *ptr);
	return crc;
}

crc16_t crc16_str(const char* str)
{
	crc16_t crc = CRC16_INIT;
	for (; *str; ++str)
		crc = crc16_up_(crc, *str);
	return crc;
}
