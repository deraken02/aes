#include <cypher.h>
#include <decypher.h>
#include <key.h>

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>

uint32_t SHA_LEN = SHA_LEN_128;
uint32_t EXPEND_KEY_WORDS_NB = EXPEND_KEY_WORDS_NB_128;
uint32_t ROUND_KEY_EXPANSION = ROUND_KEY_EXPANSION_128;
uint32_t NB_OF_ROUND = NB_OF_ROUND_128;

int32_t compute(cypher_block_arg_st arg,int32_t io_fd[2],bool decypher, int32_t thread_nb)
{
    int32_t sta = 0;
    uint8_t readed = read(io_fd[0], arg.block, 16);
    while(readed == 16)
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
    int32_t sta = 0;
    char key_pass[256] = {0};
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
            } else
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
    sta = expend_key(key_pass, arg.expended_key);
    if(sta != 0)
    {
        printf("Something went wrong with OpenSSL\n");
    } else
    {
        sta = compute(arg, io_fd, decypher, 1);    
    }

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

