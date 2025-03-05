/* Init process */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

int main()
{
    int fd;

    printf("Test socket ...\n");
    fd = socket(PF_INET, SOCK_STREAM, 6);
    if (fd < 0) {
        perror("Bad socket!\n");
        exit(-1);
    }

    printf("Close socket ...\n");
    close(fd);
    printf("Test socket ok!\n");
    return 0;
}
