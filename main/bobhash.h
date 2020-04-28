#pragma once

#include <cstdint>
uint32_t bobhash(uint32_t key, uint32_t initval = 0x137f137f);

class Bob {
public:
	const static  uint32_t arbitrary[];
	uint32_t  initval;
	Bob(uint32_t c = 2) : initval(c) {};
	uint32_t operator()(uint32_t key)
	{
		return bobhash(key, initval);
	}
};
