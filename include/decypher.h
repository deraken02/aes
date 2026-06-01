#ifndef _DECYPHER_H
#define _DECYPHER_H

#include <aes.h>

int32_t invert_substitution(uint8_t matrix[16]);
int32_t invert_shift_rows(uint8_t matrix[16]);
int32_t unmix_columns(uint8_t matrix[16]);
#endif
