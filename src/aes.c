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

/*
 * Shift the row equally to the line rank
 */
int32_t shift_rows(uint8_t matrix[16])
{
    for(uint8_t line=1; line<4; line++)
    {
        uint8_t first_row = 4*line;
        uint8_t last_row  = first_row + 3;    
        for(uint8_t round = 0; round < line; round++)
        {
            uint8_t buff = matrix[first_row];
            matrix[first_row] = matrix[first_row + 1];
            matrix[first_row + 1] = matrix[first_row + 2];
            matrix[first_row + 2] = matrix[last_row];
            matrix[last_row]=buff;
        }
    }
    return 0;
}

void print_matrix(uint8_t matrix[16])
{
    for(uint8_t i=0; i<16; i++)
    {
        printf("%x ", matrix[i]);
        if(i%4 == 3)
        {
            putchar('\n');
        }
    }
    putchar('\n');
}

int32_t mix_columns(uint8_t matrix[16])
{
    uint8_t copy[16];
    memcpy(copy, matrix, 16*sizeof(uint8_t));
    for(uint8_t row=0; row<4; row++)
    {
        for(uint8_t line=0; line<4; line++)
        {
            uint32_t res = 0;
            for(uint8_t product = 0; product<4; product++)
            {
                 res += (copy[row + (4*product)] * column_mixer[(line*4) + product]); 
            }
            matrix[line*4+row] = res % 255;
        }
    }
    return 0;
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
    uint8_t matrix[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    print_matrix(matrix);
    shift_rows(matrix);
    print_matrix(matrix);
    mix_columns(matrix);
    print_matrix(matrix);
    return sta;
}
