/* Init process */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

const char REQUEST[] = "\
GET / HTTP/1.1\r\n\
Host: ident.me\r\n\
Accept: */*\r\n\
\r\n";

int main()
{
    int fd;
    int nread;
    struct addrinfo *ai;
    char buf[1024];

    printf("New socket ...\n");
    fd = socket(PF_INET, SOCK_STREAM, 6);
    if (fd < 0) {
        perror("Bad socket!\n");
        exit(EXIT_FAILURE);
    }

    printf("Get addrinfo ...\n");
    if (getaddrinfo("49.12.234.183", "80", NULL, &ai) != 0) {
        perror("Bad addr!\n");
        exit(EXIT_FAILURE);
    }

    printf("Connect ...\n");
    if (connect(fd, ai->ai_addr, ai->ai_addrlen) != 0) {
        perror("Connect err!\n");
        exit(EXIT_FAILURE);
    }

    printf("Request ...\n");
    if (write(fd, REQUEST, sizeof(REQUEST)) != sizeof(REQUEST)) {
        perror("Request err!\n");
        exit(EXIT_FAILURE);
    }

    printf("Reply ...\n");
    nread = read(fd, buf, sizeof(buf));
    printf("nread: %d\n", nread);
    if (nread == -1 || nread >= sizeof(buf)) {
        perror("Reply error!");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", buf);

    printf("Free addrinfo ...\n");
    freeaddrinfo(ai);

    printf("Close socket ...\n");
    close(fd);
    printf("Test socket ok!\n");
    return 0;
}
