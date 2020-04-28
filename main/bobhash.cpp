#include "bobhash.h"

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

uint32_t bobhash(uint32_t key, uint32_t initval) {
	const char * str = (const char *)&key;
	uint32_t a, b, c;

	/* Set up the internal state */
	a = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	b = 0xf8a69423;  //随意设置的数，原本的算法是b=a 
	c = initval;     /* the previous hash value */

	a += ((uint32_t)str[3] << 24);
	a += ((uint32_t)str[2] << 16);
	a += ((uint32_t)str[1] << 8);
	a += str[0];

	mix(a, b, c);

	return c;
}


const uint32_t Bob::arbitrary[] = {
	18381973, 99997867, 0xf0f0f0f0,
	0x01234567, 97, 10000019,
	20717, 2018815, 5046500,
};
