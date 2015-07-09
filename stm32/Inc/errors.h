#pragma once

typedef enum {
	err_ok          = 0,
	err_proto       = 5,
	err_cmd         = 9,
	err_param       = 10,
	err_state       = 15,
	err_overheat    = 20,
	err_power       = 21,
	err_malfunction = 22,
	err_internal    = 100,
} err_t;
