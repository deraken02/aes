#include <cypher.h>
#include <decypher.h>

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>

#define SHA_LEN_256 (256/8)
#define EXPEND_KEY_WORDS_NB_256  60
#define ROUND_KEY_EXPANSION_256  (256/32)
#define NB_OF_ROUND_256  14

#define SHA_LEN_128 (128/8)
#define EXPEND_KEY_WORDS_NB_128 44
#define ROUND_KEY_EXPANSION_128 (128/32)
#define NB_OF_ROUND_128 10

typedef struct cypher_block_arg_t
{
    uint8_t block[16];
    uint32_t expended_key[EXPEND_KEY_WORDS_NB_256];
} cypher_block_arg_st; 

static uint32_t SHA_LEN = SHA_LEN_128;
static uint32_t EXPEND_KEY_WORDS_NB = EXPEND_KEY_WORDS_NB_128;
static uint32_t ROUND_KEY_EXPANSION = ROUND_KEY_EXPANSION_128;
static uint32_t NB_OF_ROUND = NB_OF_ROUND_128;

static int32_t generate_256bits_key(char *keypass, uint8_t out_key[SHA_LEN])
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

int32_t compute(cypher_block_arg_st arg,int32_t io_fd[2],bool decypher, int32_t thread_nb)
{
    int32_t sta = 0;
    uint8_t readed = read(io_fd[0], arg.block, 16);
    while( readed == 16)
    {
        if(decypher != true)
        {
            cypher_block(&arg);
        } else
        {
            decypher_block(&arg);
        }
        write(io_fd[1], arg.block, 16);
        memset(&(arg.block), 0, 16);
        readed = read(io_fd[0], arg.block, 16);
    }
    if(readed != 0)
    {
        if(decypher != true)
        {
            cypher_block(&arg);
        } else
        {
            decypher_block(&arg);
        }
        write(io_fd[1], arg.block, 16);
    }
    return sta;
}

int32_t main(int32_t argc, char *argv[])
{
    uint8_t sha_key[SHA_LEN_256] = {0};
    char key_pass[256] = {0};
    int32_t sta = 0;
    int32_t io_fd[2] = {STDIN_FILENO, STDOUT_FILENO};
    bool password_found = false;
    bool decypher = false;
    int32_t thread_nb = 1;
    cypher_block_arg_st arg;
    memset(&arg, 0, sizeof(cypher_block_arg_st)); 
    if(argc == 1)
    {
        printf("Enter password: ");
        fflush(NULL);
        sta = read(STDIN_FILENO, key_pass, 256);
        if(sta == 0)
        {
            perror("Empty password\n");
            return -1;
        } else
        {
            password_found = true;
        }
    } else
    {
        for(uint8_t i = 1; i<argc; i++)
        {
            if(strncmp(argv[i], "--passwd", 8) == 0)
            {
                if(argv[i][8] == '=')
                {
                    strncpy(key_pass, &argv[i][9], 256);
                } else
                {
                    i++;
                    strncpy(key_pass, argv[i], 256);
                }

                password_found = true;
            } else if(strncmp(argv[i], "--file_in", 9) == 0)
            {
                if(argv[i][9] == '=')
                {
                    sta = open(&argv[i][10], O_RDONLY);
                } else
                {
                    i++;
                    sta = open(argv[i], O_RDONLY);
                }
                if (sta > 2)
                {
                    io_fd[0] = sta;
                    sta = 0;
                } else
                {
                    perror("Can't open file");
                }
            } else if(strncmp(argv[i], "--file_out", 10) == 0)
            {
                if(argv[i][10] == '=')
                {
                    sta = open(&argv[i][11], O_CREAT|O_WRONLY|O_TRUNC, 420);
                } else
                {
                    i++;
                    sta = open(argv[i], O_CREAT|O_WRONLY|O_TRUNC, 420);
                }
                if (sta > 2)
                {
                    io_fd[1] = sta;
                    sta = 0;
                } else
                {
                    perror("Can't open file");
                }
            } else if(strncmp(argv[i], "--thread", 8) == 0)
            {
                i++;
                /* To be done */
            } else if(strncmp(argv[i], "--cypher", 8) == 0)
            {
                decypher = false;
            } else if(strncmp(argv[i], "--decypher", 10) == 0)
            {
                decypher = true;
            } else if(strncmp(argv[i], "-d", 2) == 0)
            {
                decypher = true;
            }
            {
                printf("Unknown options %s\n", argv[i]);
            }
        }
    }
    if(password_found != true)
    {
        perror("A password is mandatory");
        if(io_fd[0] != 0)
        {
            close(io_fd[0]);
        }
        if(io_fd[1] != 1) {
            close(io_fd[1]);
        }
        return -1;
    }
    sta = generate_256bits_key(key_pass, sha_key);
    if(sta != 0)
    {
        perror("Something went wrong with OpenSSL");
    } else
    {
        sta = expand_key(sha_key, arg.expended_key);
    }

    sta = compute(arg, io_fd, decypher, 1);    

    if(io_fd[0] != 0)
    {
        close(io_fd[0]);
    }
    if(io_fd[1] != 1)
    {
        close(io_fd[1]);
    }
    return sta;
}

