#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef DEBUG
#define LOG_DEBUG(...) printf("DEBUG: " __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#define ERR_BAD_ARG_1    -1
#define ERR_BAD_ARG_2    -2
#define ERR_FAILED_DUP   -3

#define MIN_FD_AFTER_STD (STDERR_FILENO + 1)

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "file.txt"
#endif

extern int errno;

static int custom_dup(int old_fd);
static int custom_dup2(int old_fd, int new_fd);

static void test_custom_dup_positive(void);
static void test_custom_dup2_positive(void);


int main(int argc, char *argv[]) {
    test_custom_dup_positive();
    test_custom_dup2_positive();
    return EXIT_SUCCESS;
}


static int custom_dup(int old_fd) {
    if (fcntl(old_fd, F_GETFD) == -1) {
        errno = EBADF;
        return ERR_BAD_ARG_1;
    }
    LOG_DEBUG("received old_fw %d\n", old_fd);
    int new_fd = fcntl(old_fd, F_DUPFD, MIN_FD_AFTER_STD);
    if (new_fd == -1) {
        return ERR_FAILED_DUP;
    }
    LOG_DEBUG("new_fs is %d\n", new_fd);
    
    return new_fd;
}


static int custom_dup2(int old_fd, int new_fd) {
    if ((fcntl(old_fd, F_GETFD) == -1)) {
        errno = EBADF;
        return ERR_BAD_ARG_1;
    }
    if (new_fd < MIN_FD_AFTER_STD) {
        errno = EBADF;
        return ERR_BAD_ARG_2;
    }
    if ((fcntl(new_fd, F_GETFD) != -1)) {
        close(new_fd);
    }
    LOG_DEBUG("received old_fw %d new_fd %d\n", old_fd, new_fd);
    int tmp = -1;
    tmp = fcntl(old_fd, F_DUPFD, new_fd);
    if (tmp != new_fd) {
        return ERR_FAILED_DUP;
    }
    LOG_DEBUG("new_fd is %d\n", new_fd);
    return new_fd;
}

static void test_custom_dup_positive(void) {
    /* 1. setup */
    int err_loc = 0, old_fd = -1, new_fd = -1;

    /* 2. Call */
    old_fd = open(TEST_FILE_NAME, O_RDWR|O_TRUNC|O_CREAT, S_IWUSR|S_IRUSR);
    if (old_fd == -1) {
        err_loc = 1;
        goto teardown;
    }
    /* my custom dups return not only -1 on error */
    new_fd = custom_dup(old_fd);
    if (new_fd < 0) {
        err_loc = 2;
        goto teardown;
    }
    if (write(new_fd, "hello world\n", strlen("hello world\n")) == -1) {
        err_loc = 3;
        goto teardown;
    }
    off_t off_old = lseek(old_fd, 0, SEEK_CUR);
    if (off_old == -1) {
        err_loc = 4;
        goto teardown;
    }
    off_t off_new = lseek(new_fd, 0, SEEK_CUR);
    if (off_new == -1) {
        err_loc = 5;
        goto teardown;
    }
    assert(off_new == off_old);
    char to_write[] = "new word\n";
    if (write(old_fd, to_write, strlen(to_write)) == -1) {
        err_loc = 6;
        goto teardown;
    }
    char to_read[sizeof(to_write)] = {0};
    if (pread(new_fd, to_read, strlen(to_write), off_old) == -1) {
        err_loc = 7;
        goto teardown;
    } 
    assert(strncmp(to_write, to_read, strlen(to_write)) == 0);
    printf("test_custom_dup_positive PASSED\n");

    /* 3. Teardown */
teardown:
    if (old_fd != -1) {
        close(old_fd);
    }
    if (new_fd != -1) {
        close(new_fd);
    } 
    if (err_loc != 0) {
        char buff[32] = {0};
        snprintf(buff, sizeof(buff), "error in location %d", err_loc);
        perror(buff);
    }
}

static void test_custom_dup2_positive(void) {
    /* 1. Setup */
    int err_loc = 0, old_fd = -1, new_fd = -1;

    /* 2. Call */
    FILE *tmp_file = tmpfile();
    if (tmp_file == NULL) {
        err_loc = 1;
        goto teardown;
    }
    int tmp_fd = fileno(tmp_file);
    if (tmp_fd == -1) {
        err_loc = 2;
        goto teardown;
    }
    old_fd = open(TEST_FILE_NAME, O_RDWR|O_TRUNC|O_CREAT, S_IWUSR|S_IRUSR);
    if (old_fd == -1) {
        err_loc = 3;
        goto teardown;
    }
    new_fd = custom_dup2(old_fd, tmp_fd);
    /* my custom dups return not only -1 on error */
    if (new_fd < 0) {
        err_loc = 5;
        goto teardown;
    }

    char to_write[] = "this goes to test file via duplicated fd rather than tempfile\n";
    if (write(new_fd, to_write, strlen(to_write)) == -1) {
        err_loc = 6;
        goto teardown;
    }
    char to_read[sizeof(to_write)] = {0};
    if (pread(old_fd, to_read, strlen(to_write), (off_t)0) == -1) {
        err_loc = 7;
        goto teardown;
    }

    assert(tmp_fd == new_fd);
    assert(strncmp(to_write, to_read, strlen(to_write)) == 0);
    printf("test_custom_dup2_positive PASSED\n");

    /* 3. Teardown */
teardown:
    if (old_fd != -1) {
        close(old_fd);
    }
    if (new_fd != -1) {
        close(new_fd);
    } 
    if (err_loc != 0) {
        char buff[32] = {0};
        snprintf(buff, sizeof(buff), "error in location %d", err_loc);
        perror(buff);
    }
}
