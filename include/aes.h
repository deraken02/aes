#ifndef _AES_HEADER
#define _AES_HEADER

#include <openssl/sha.h>

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define SIZEOF_BLOCK (sizeof(uint8_t) * 16)
#define UINT8_TO_UINT32( a, b, c, d)    (a<<24 | b <<16 | c<<8 | d)

#define TRANSFORM(x)    ((x*2) ^ (((x>>7)&1) * 0x1b))

#endif
