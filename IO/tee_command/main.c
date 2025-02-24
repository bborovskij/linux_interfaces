#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef DEBUG
#define LOG_DEBUG(...) printf(DEBUG_LVL __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#define MAX_BUFF  64
#define ERROR_VAL -1
#define DEBUG_LVL "DEBUG: "
#define SUPPORTED_SHORT_OPTS "ah"
#define USAGE "Usage:\n\
./my_tee file.txt <<< \"one two three\" -- to create or overwrite a file.\n\
./my_tee -a file.txt <<< \"four five six\" -- to append to file\n"

extern int errno;
extern int optind;

int main(int argc, char **argv) {
    int ret = EXIT_SUCCESS;

    /* 1. Exit early if misused. */
    if ((argc < 2) || (argc > 3)) {
        fprintf(stderr, "incorrect number of arguments: %d.\n" USAGE, argc);
        exit(EXIT_FAILURE);
    }

    /* 2. Check if there is -a added so that we don't re-write the file. */
    LOG_DEBUG("reading options\n");
    int opt = ERROR_VAL;
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
    while ((opt = getopt(argc, argv, SUPPORTED_SHORT_OPTS)) != ERROR_VAL) {
        switch (opt) {
            case 'a':
                flags &= ~O_TRUNC;
                flags |= O_APPEND;
                LOG_DEBUG("-a used. Avoid truncating. Append to file\n");
                break;
            
            case 'h':
                printf(USAGE);
                exit(EXIT_SUCCESS);
            
            default:
                printf(USAGE);
                exit(EXIT_FAILURE);
        }
    }
    LOG_DEBUG("optind=%d, argc=%d\n", optind, argc);
    if (optind >= argc) {
        fprintf(stderr, "Missing file arg.\n" USAGE);
        exit(EXIT_FAILURE);
    }


    /* 3. Open the text file with needed flags and permission. */
    LOG_DEBUG("opening text file\n");
    int fd = ERROR_VAL;
    int mod = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if ((fd = open(argv[optind], flags, mod)) == ERROR_VAL) {
        perror("error opening file");
        ret = EXIT_FAILURE;
        goto exit;
    }
    LOG_DEBUG("file opened. fd=%d\n", fd);

    /* 4. Read from stdint and write into stdout and text file. */
    LOG_DEBUG("reading from stdin\n");
    ssize_t read_num = 0, written_num = 0;
    /* null terminate after read if needed to print buffer. */
    char buff[MAX_BUFF] = {0};
    while ((read_num = read(STDIN_FILENO, buff, MAX_BUFF)) > 0) {
        LOG_DEBUG("chunk read: %ld\n", read_num);

        written_num = 0;
        while (written_num < read_num) {
            ssize_t n = write(STDOUT_FILENO, buff + written_num, read_num - written_num);
            if (n == ERROR_VAL) {
                ret = EXIT_FAILURE;
                perror("error writing to stdout");
                goto exit;
            }
            written_num += n;
        }
        written_num = 0;
        while (written_num < read_num) {
            ssize_t n = write(fd, buff + written_num, read_num - written_num);
            if (n == ERROR_VAL) {
                ret = EXIT_FAILURE;
                perror("error writing to text file");
                goto exit;
            }
            written_num += n;
        }
    }
    if (read_num == ERROR_VAL) {
        ret = EXIT_FAILURE;
        perror("error reading from stdin");
        goto exit;
    }
    LOG_DEBUG("read from stdin and wrote to files\n");
    
    /* 5. Teardown. */
exit:
    if (fd != ERROR_VAL) {
        close(fd);
    }
    
    return ret;
}
