#ifndef _KEY_H
#define _KEY_H

#include <aes.h>

int32_t expend_key(char keypass[256], uint32_t out_expanded_key[EXPEND_KEY_WORDS_NB]);

#endif

