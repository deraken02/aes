#include <cypher.h>
#include <decypher.h>
#include <key.h>

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

uint32_t SHA_LEN = SHA_LEN_256;
uint32_t EXPEND_KEY_WORDS_NB = EXPEND_KEY_WORDS_NB_256;
uint32_t ROUND_KEY_EXPANSION = ROUND_KEY_EXPANSION_256;
uint32_t NB_OF_ROUND = NB_OF_ROUND_256;

int32_t compute_multithreaded(cypher_block_arg_st *arg, int32_t io_fd[2], bool decypher, int32_t thread_nb)
{
    int32_t sta = 0;
    cypher_block_arg_st *ret;
    pthread_t thread_id[thread_nb];
    uint8_t readed= 16;
    int32_t i;
    char *eof;
    uintptr_t last_char_pos;
    bool first_loop = true;
    while(readed == 16)
    {
        for(i=0; i < thread_nb; i++)
        { 
            readed = read(io_fd[0], arg[i].block, 16);
            if(readed == 0)
            {
                break;
            }
            if((first_loop != false) && (i != 0))
            { 
                memcpy(arg[i].expended_key, arg[0].expended_key, sizeof(uint32_t)*EXPEND_KEY_WORDS_NB_256);
            }
            if(decypher != true)
            {
                pthread_create(&thread_id[i], NULL, cypher_block, (void *)&arg[i]);
            } else
            {
                pthread_create(&thread_id[i], NULL, decypher_block, (void *)&arg[i]);
            }
            if(readed != 16)
            {
                i++;
                break;
            }
        }
        first_loop=false;
        for(int32_t j = 0;  j<i; j++)
        {
            pthread_join(thread_id[j],(void **)&ret);
            if((decypher != false) && (ret->block[15] == '\0'))
            {
                eof = strchr((char *)ret->block, '\0');
                last_char_pos = ((uintptr_t)eof - (uintptr_t)ret->block);
                if(last_char_pos < 16ULL)
                {
                    write(io_fd[1], ret->block, (size_t)last_char_pos);
                } else
                {
                    write(io_fd[1], ret->block, 16);
                }
            } else
            {
                write(io_fd[1], ret->block, 16);
                memset(&(ret->block), 0, 16);
            }
        }
    }
    return 0;
}

int32_t compute_monothreaded(cypher_block_arg_st *arg,int32_t io_fd[2],bool decypher)
{
    int32_t sta = 0;
    uint8_t readed = read(io_fd[0], arg->block, 16);
    char *eof;
    uintptr_t last_char_pos;
    while(readed == 16)
    {
        if(decypher != true)
        {
            cypher_block(arg);
        } else
        {
            decypher_block(arg);
            if(arg->block[15] == '\0')
            {
                eof = strchr((char *)arg->block, '\0');
                last_char_pos = ((uintptr_t)eof - (uintptr_t)arg->block);
                if(last_char_pos < 16ULL)
                {
                    write(io_fd[1], arg->block, (size_t)last_char_pos);
                } else
                {
                    write(io_fd[1], arg->block, 16);
                }
                break;
            }
        }
        write(io_fd[1], arg->block, 16);
        memset(&(arg->block), 0, 16);
        readed = read(io_fd[0], arg->block, 16);
    }
    if ((readed != 0) && (decypher != true))
    {
        cypher_block(arg);
        write(io_fd[1], arg->block, 16);
    } else
    {
        ;/* Do nothing */
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
    cypher_block_arg_st *arg;
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
                char *endptr;
                if(argv[i][8] == '=')
                {
                    thread_nb = (int)strtol(&argv[i][9], &endptr, 10);

                } else
                {
                    i++;
                    thread_nb = (int)strtol(argv[i], &endptr, 10);
                }
                if(endptr[0] != '\0')
                {
                    thread_nb = -1;
                    printf("Invalide thread number\n");
                    break;
                }
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
    if(password_found != true || thread_nb <= 0)
    {
        if(password_found != true)
        {
            perror("A password is mandatory");
        }
        if(io_fd[0] != 0)
        {
            close(io_fd[0]);
        }
        if(io_fd[1] != 1) {
            close(io_fd[1]);
        }
        return -1;
    }
    arg = calloc(thread_nb, sizeof(cypher_block_arg_st));
    sta = expend_key(key_pass, arg->expended_key);
    if(sta != 0)
    {
        printf("Something went wrong with OpenSSL\n");
    } else if(thread_nb == 1)
    {
        sta = compute_monothreaded(arg, io_fd, decypher);    
    } else
    {
        sta = compute_multithreaded(arg, io_fd, decypher, thread_nb);    
    }
    free(arg);

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

