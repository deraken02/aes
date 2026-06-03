#ifndef _CYPHER_H
#define _CYPHER_H

#include <aes.h>

int32_t round_key(uint8_t matrix[16], uint32_t *key);
void *cypher_block(void *argument);
#endif
