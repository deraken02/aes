#include <aes.h>
#include <stdio.h>

#define SHA256_LEN (256/8)
#define EXPEND_KEY_WORDS_NB 60

#define CONCAT_KEY_WORD( a, b, c, d)    (a<<24 | b <<16 | c<<8 | d)
/**
 * Error code:
 *  -  0 : No errors
 *  - -1 : Error from sha256 generation
 */
int32_t generate_256bits_key(char *keypass, uint8_t out_key[SHA256_LEN])
{
    int32_t sta = 0;
    if(SHA256(keypass, strlen(keypass), out_key) == NULL)
    {
        sta = -1;
    }
    return sta;
}

uint32_t core(uint32_t key_part)
{
    uint32_t ret = key_part<<8 | ((key_part>>24)&0xff); 
    ret = CONCAT_KEY_WORD(s_box[(ret >> 24)&0xff], s_box[(ret>>16)&0xff], s_box[(ret>>8)&0xff], s_box[ret&0xff]);
    return ret;
}

int32_t expand_key(uint8_t key[SHA256_LEN], uint32_t out_expanded_key[EXPEND_KEY_WORDS_NB])
{
    int32_t sta = 0;
    for(uint8_t i = 0; i < EXPEND_KEY_WORDS_NB; i++)
    {
        if(i < 8)
        {
            out_expanded_key[i] = CONCAT_KEY_WORD(key[i*4], key[i*4+1], key[i*4+2], key[i*4+3]);
        } else if (i%4 == 0)
        {
            out_expanded_key[i] = out_expanded_key[i-8] ^ core(out_expanded_key[i-1]);
        } else
        {
            out_expanded_key[i] = out_expanded_key[i-8] ^ out_expanded_key[i-1];
        }
    }
    return sta;
}

int32_t main(int32_t argc, char *argv[])
{
    uint8_t sha256_key[SHA256_LEN] = {0};
    uint32_t expanded_key[EXPEND_KEY_WORDS_NB] = {0};
    int32_t sta = 0;
    if(argc != 2)
    {
        perror("Bad usage: need password in parameter");
        return EBADMSG;
    }
    sta = generate_256bits_key(argv[1], sha256_key);
    if(sta != 0)
    {
        perror("Something went wrong with OpenSSL");
    } else
    {
        sta = expand_key(sha256_key, expanded_key);

    }

    return sta;
}
