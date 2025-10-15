#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

ssize_t write(int fd, const void *buf, size_t count) {
    static ssize_t (*original_write)(int, const void*, size_t) = NULL;
    if (!original_write) {
        original_write = dlsym(RTLD_NEXT, "write");
        if (!original_write) {
            fprintf(stderr, "Error: Could not find the original write function.\n");
            return -1;
        }
    }

    fprintf(stderr, "\n--- [LOGGER] Intercepted write() call ---\n");
    fprintf(stderr, "    File Descriptor: %d\n", fd);
    fprintf(stderr, "    Bytes to Write: %zu\n", count);

    return original_write(fd, buf, count);
}

