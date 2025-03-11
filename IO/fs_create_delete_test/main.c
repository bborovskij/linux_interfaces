#define _GNU_SOURCE

#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdbool.h>


#define MAX_FILE_NUM 1000000
#define FNAME_LEN    (sizeof("x000000") - 1)
#define ABS_PATH_LEN 63

extern char *optarg;
extern int optind, opterr, optopt;
extern int errno;

static char** fnames_malloc(const int n);
static void fnames_free(char **fname, int n);
static void shuffle_fnames(char **fname, int n);
static void fnames_print(char **fnames, int n) __attribute__((unused));
static inline void usage_print(void);

static int err_loc;


int main(int argc, char *argv[]) {
    int opt;
    int n_files = 0;
    bool incr = false;

    while ((opt = getopt(argc, argv, "hin:")) != -1) {
        switch (opt)
        {
        case 'n':
            n_files = (int)strtol(optarg, NULL, 10);
            break;
        case 'i':
            incr = true;
            break;
        case 'h':
            usage_print();
            return EXIT_SUCCESS;
        default:
            usage_print();
            return EXIT_FAILURE;
        }
    }
    if ((n_files <= 0) || (n_files > MAX_FILE_NUM)) {
        fprintf(stderr, "Incorrect file number: %d\n", n_files);
        usage_print();
        return EXIT_FAILURE;
    }
    if (optind >= argc) {
        fprintf(stderr, "Dir name required\n");
        usage_print();
        return EXIT_FAILURE;
    }
    char** fnames = fnames_malloc(n_files);
    if ((fnames == NULL) || (err_loc != 0)) {
        fnames_free(fnames, n_files);
        fprintf(stderr, "Error fnames_malloc: %d\n", err_loc);
        return EXIT_FAILURE;
    }
    if (incr) {
        printf("Creating files incrementally\n");
    } else {
        printf("Creating files in random order\n");
        shuffle_fnames(fnames, n_files);
        if (err_loc != 0) {
            fnames_free(fnames, n_files);
            fprintf(stderr, "Error shuffle_fnames: %d\n", err_loc);
            return EXIT_FAILURE;
        }
    }

    char *dirname = argv[optind];
    printf("Creating %d files in %s\n", n_files, dirname);
    struct timeval s;
    if (gettimeofday(&s, NULL) != 0) {
        fnames_free(fnames, n_files);
        fprintf(stderr, "Error gettimeofday start create: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    for (int i = 0; i < n_files; i++) {
        char abs_path[ABS_PATH_LEN + 1];
        snprintf(abs_path, ABS_PATH_LEN + 1, "%s/%s", dirname, fnames[i]);
        int fd = open(abs_path, O_CREAT|O_TRUNC|O_WRONLY, 0644);
        if (fd < 0) {
            fprintf(stderr, "Error open %s: %s\n", abs_path, strerror(errno));
            fnames_free(fnames, n_files);
            return EXIT_FAILURE;
        }
        char a = 'A';
        if (write(fd, &a, 1) != 1) {
            fprintf(stderr, "Error write %s: %s\n", abs_path, strerror(errno));
            fnames_free(fnames, n_files);
            return EXIT_FAILURE;
        }
        close(fd);
    }
    struct timeval end;
    if (gettimeofday(&end, NULL) != 0) {
        fnames_free(fnames, n_files);
        fprintf(stderr, "Error gettimeofday end create: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    long seconds, ms;
    seconds = end.tv_sec - s.tv_sec;
    ms = (end.tv_usec - s.tv_usec) / 1000;
    if (ms < 0) {
        --seconds;
        ms = 1000 + ms;
    }
    printf("Created %d files in %ld s %ld ms\n", n_files, seconds, ms);
    if (gettimeofday(&s, NULL) != 0) {
        fnames_free(fnames, n_files);
        fprintf(stderr, "Error gettimeofday start delete: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Delete incrementally
    for (int i = 0; i < n_files; i++) {
        char abs_path[ABS_PATH_LEN + 1];
        snprintf(abs_path, ABS_PATH_LEN + 1, "%s/x%06d", dirname, i);
        if (unlink(abs_path) != 0) {
            fprintf(stderr, "Error unlink file: %s, %s\n", abs_path, strerror(errno));
            fnames_free(fnames, n_files);
            return EXIT_FAILURE;
        }
    }
    if (gettimeofday(&end, NULL) != 0) {
        fnames_free(fnames, n_files);
        fprintf(stderr, "Error gettimeofday end delete: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    seconds = end.tv_sec - s.tv_sec;
    ms = (end.tv_usec - s.tv_usec) / 1000;
    if (ms < 0) {
        --seconds;
        ms = 1000 + ms;
    }
    printf("Deleted %d files in %ld s %ld ms\n", n_files, seconds, ms);
    fnames_free(fnames, n_files);
    return EXIT_SUCCESS;
}


static char** fnames_malloc(const int n) {
    char **fnames = calloc(n, sizeof(char*));
    if (fnames == NULL) {
        err_loc = 1;
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        fnames[i] = malloc(FNAME_LEN + 1);
        if (fnames[i] == NULL) {
            err_loc = 2;
            goto error;
        }
        if (snprintf(fnames[i], FNAME_LEN+1, "x%06d", i) != FNAME_LEN) {
            err_loc = 3;
            goto error;
        }
    }
    return fnames;
error:
    fnames_free(fnames, n);
    return NULL;
}

static void fnames_free(char **fnames, int n) {
    if (fnames == NULL) {
        return;
    }
    for (int i=0; i<n; ++i) {
        if (fnames[i] != NULL) {
            free(fnames[i]);
        }

    }
    free(fnames);
}

static void shuffle_fnames(char **fnames, int n) {
    if (fnames == NULL) {
        err_loc = 1;
        return;
    }
    srand((unsigned) time(NULL));
    for (int i = n - 1; i > 0; --i) {
        if (fnames[i] == NULL) {
            err_loc = 2;
            return;
        }
        int j = rand() % (i + 1);
        if (fnames[j] == NULL) {
            err_loc = 3;
            return;
        }
        char temp[FNAME_LEN + 1] = {0};
        strncpy(temp, fnames[i], FNAME_LEN);
        strncpy(fnames[i], fnames[j], FNAME_LEN);
        strncpy(fnames[j], temp, FNAME_LEN);
    }   
}

static void fnames_print(char  **fnames, int n) {
    if (fnames == NULL) {
        printf("(null)\n");
        return;
    }
    for (int i = 0; i < n; ++i) {
        printf("%s\n", fnames[i]);
    }
}

static inline void usage_print(void) {
    printf("Usage:\n\
        main -n <num> [-i] dir\n\
        where:\n\
        -n: number of files to create (1-1,000,000)\n\
        -i: specify create files incrementatlly rather than in random order\n\
        dir: directory with a mounted FS\n");
}
