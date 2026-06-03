#ifndef _AES_HEADER
#define _AES_HEADER

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define SHA_LEN_256             (256/8)
#define EXPEND_KEY_WORDS_NB_256 (60)
#define ROUND_KEY_EXPANSION_256 (256/32)
#define NB_OF_ROUND_256         (14)

#define SHA_LEN_128             (128/8)
#define EXPEND_KEY_WORDS_NB_128 (44)
#define ROUND_KEY_EXPANSION_128 (128/32)
#define NB_OF_ROUND_128         (10)

#define UINT8_TO_UINT32(a, b, c, d)     (a<<24 | b <<16 | c<<8 | d)
#define TRANSFORM(x)                    ((x*2) ^ (((x>>7)&1) * 0x1b))

extern uint32_t SHA_LEN;
extern uint32_t EXPEND_KEY_WORDS_NB;
extern uint32_t ROUND_KEY_EXPANSION;
extern uint32_t NB_OF_ROUND;

extern uint8_t s_box[256];

typedef struct cypher_block_arg
{
    uint8_t block[16];
    uint32_t expended_key[EXPEND_KEY_WORDS_NB_256];
} cypher_block_arg_st; 

#endif
