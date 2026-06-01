#include <cypher.h>
#include <decypher.h>

#include <stdbool.h>
#include <stdio.h>

#if defined(SHA256_KEY_LEY)
#define SHA_LEN (256/8)
#define EXPEND_KEY_WORDS_NB 60
#define ROUND_KEY_EXPANSION (256/32)
#define NB_OF_ROUND 14
#else
#define SHA_LEN (128/8)
#define EXPEND_KEY_WORDS_NB 44
#define ROUND_KEY_EXPANSION (128/32)
#define NB_OF_ROUND 10
#endif


/**
 * Error code:
 *  -  0 : No errors
 *  - -1 : Error from sha256 generation
 */
int32_t generate_256bits_key(char *keypass, uint8_t out_key[SHA_LEN])
{
    int32_t sta = 0;
    if(SHA256((const uint8_t *)keypass, strlen(keypass), out_key) == NULL)
    {
        sta = -1;
    }
    return sta;
}

static const uint8_t round_constant[11] = {0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40,
    0x80, 0x1b, 0x36};

uint32_t core(uint32_t key_part)
{
    uint32_t ret = key_part<<8 | ((key_part>>24)&0xff); 

    ret = UINT8_TO_UINT32(s_box[(ret >> 24)&0xff], s_box[(ret>>16)&0xff], s_box[(ret>>8)&0xff], s_box[ret&0xff]);
    return ret;
}

int32_t expand_key(uint8_t key[SHA_LEN], uint32_t out_expanded_key[EXPEND_KEY_WORDS_NB])
{
    int32_t sta = 0;
    for(uint8_t i = 0; i < EXPEND_KEY_WORDS_NB; i++)
    {

        if(i < ROUND_KEY_EXPANSION)
        {
            out_expanded_key[i] = UINT8_TO_UINT32(key[i*4], key[i*4+1], key[i*4+2], key[i*4+3]);
        } else if (i%4 == 0)
        {
            out_expanded_key[i] = (out_expanded_key[i-ROUND_KEY_EXPANSION] ^ core(out_expanded_key[i-1]) ^ (round_constant[i/ROUND_KEY_EXPANSION]<<24)) ;
        } else
        {
            out_expanded_key[i] = (out_expanded_key[i-ROUND_KEY_EXPANSION] ^ out_expanded_key[i-1]);
        }
    }
    return sta;
}

void print_matrix(uint8_t matrix[16])
{
    for(uint8_t i=0; i<16; i++)
    {
        printf("%2x ", matrix[i]);

        if(i%4 == 3)
        {
            putchar('\n');
        }
    }
    putchar('\n');
}

typedef struct cypher_block_arg_t
{
    uint8_t block[16];
    uint32_t expended_key[EXPEND_KEY_WORDS_NB];
} cypher_block_arg_st; 

void *cypher_block(void *argument)
{
    cypher_block_arg_st *arg = (cypher_block_arg_st *)argument;
    round_key(arg->block, &arg->expended_key[0]);
    for(uint8_t i = 1; i<NB_OF_ROUND; i++)
    {
        substitution(arg->block);
        shift_rows(arg->block);
        mix_columns(arg->block);
        round_key(arg->block, &arg->expended_key[i*4]);
    }
    substitution(arg->block);
    shift_rows(arg->block);
    round_key(arg->block, &arg->expended_key[NB_OF_ROUND*4]);
    return argument;
}

void *decypher_block(void *argument)
{
    cypher_block_arg_st *arg = (cypher_block_arg_st *)argument;
    round_key(arg->block, &arg->expended_key[EXPEND_KEY_WORDS_NB - 4]);
    for(uint8_t i = 0; i<NB_OF_ROUND; i++)
    {
        invert_shift_rows(arg->block);
        invert_substitution(arg->block);
        round_key(arg->block, &arg->expended_key[EXPEND_KEY_WORDS_NB- ((i+2)*4)]);
        if(i < (NB_OF_ROUND - 1))
        {
            unmix_columns(arg->block);
        }
    }
    return argument;
}


int32_t main(int32_t argc, char *argv[])
{
    uint8_t sha_key[SHA_LEN];
    int32_t sta = 0;
    cypher_block_arg_st arg={
        .block= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
    };
    if(argc != 2)
    {
        perror("Bad usage: need password in parameter");
        return EBADMSG;
    }
    memset(sha_key, 48, SHA_LEN);
    memcpy(sha_key,argv[1], strlen(argv[1]));
    //sta = generate_256bits_key(argv[1], sha_key);
    if(sta != 0)
    {
        perror("Something went wrong with OpenSSL");
    } else
    {
        sta = expand_key(sha_key, arg.expended_key);
    }
    printf("Start\n");
    print_matrix(arg.block);
    (void)cypher_block((void *)&arg);
    printf("Cyphered \n");
    print_matrix(arg.block);
    printf("Decypher \n");
    (void)decypher_block((void *)&arg);
    print_matrix(arg.block);
    return sta;
}
