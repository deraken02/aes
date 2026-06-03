#include <key.h>
#include <openssl/sha.h>


static const uint8_t round_constant[11] = {0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40,
    0x80, 0x1b, 0x36};

static int32_t generate_256bits_key(char *keypass, uint8_t out_key[SHA_LEN])
{
    int32_t sta = 0;
    if(SHA256((const uint8_t *)keypass, strlen(keypass), out_key) == NULL)
    {
        sta = -1;
    }
    return sta;
}

static uint32_t core(uint32_t key_part)
{
    uint32_t ret = key_part<<8 | ((key_part>>24)&0xff); 

    ret = UINT8_TO_UINT32(s_box[(ret >> 24)&0xff], s_box[(ret>>16)&0xff], s_box[(ret>>8)&0xff], s_box[ret&0xff]);
    return ret;
}

int32_t expend_key(char key_pass[256], uint32_t out_expanded_key[EXPEND_KEY_WORDS_NB])
{
    int32_t sta = 0;
    uint8_t key[SHA_LEN_256] = {0};
    sta = generate_256bits_key(key_pass, key); 
    if(sta != 0)
    {
        sta = -1;
    } else
    {
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
    }
    return sta;
}
