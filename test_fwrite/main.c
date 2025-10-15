#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const char *filename = "testfile.txt";
    const char *content = "This is a test written by fwrite.";

    printf("fbegin\n");
    FILE *f = fopen(filename, "w");
    if (f) fwrite(content, sizeof(char), strlen(content), f);
    fclose(f);
    printf("fend\n");

    printf("begin\n");
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) write(fd, content, strlen(content));
    close(fd);
    printf("end\n");

    return 0;
}
