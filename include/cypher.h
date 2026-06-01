#ifndef _CYPHER_H
#define _CYPHER_H

#include <aes.h>

extern uint8_t s_box[256];
int32_t substitution(uint8_t matrix[16]);
int32_t shift_rows(uint8_t matrix[16]);
int32_t mix_columns(uint8_t matrix[16]);
int32_t round_key(uint8_t matrix[16], uint32_t *key);

#endif
