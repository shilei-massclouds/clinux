#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#define LINELEN 512
#define _PATH_PROCNET_DEV       "/proc/net/dev"

const char REQUEST[] = "\
GET / HTTP/1.1\r\n\
Host: ident.me\r\n\
Accept: */*\r\n\
\r\n";

int procnetdev_vsn = 1;

/*
static void display_net_dev() {
	int ret;
	char line[LINELEN];
	FILE *fstatus = NULL;
    unsigned int lock_sz = 0;

	fstatus = fopen("/proc/net/dev", "r");
	if (fstatus == NULL)
        printf("Open dev status failed\n");

	while (fgets(line, LINELEN, fstatus) != NULL) {
        printf("line: %s\n", line);
    }

	fclose(fstatus);
}
*/

static int procnetdev_version(char *buf)
{
    if (strstr(buf, "compressed"))
    return 3;
    if (strstr(buf, "bytes"))
    return 2;
    return 1;
}

char *get_name(char *name, char *p)
{
    while (isspace(*p))
    p++;
    while (*p) {
    if (isspace(*p))
        break;
    if (*p == ':') {    /* could be an alias */
        char *dot = p++;
        while (*p && isdigit(*p)) p++;
        if (*p == ':') {
            /* Yes it is, backup and copy it. */
            p = dot;
            *name++ = *p++;
            while (*p && isdigit(*p)) {
                *name++ = *p++;
            }
        } else {
            /* No, it isn't */
            p = dot;
        }
        p++;
        break;
    }
    *name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

/* Set a certain interface flag. */
static int set_flag(int fd, char *ifname, short flag)
{
    struct ifreq ifr;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, "%s: ERROR while getting interface flags: %s\n",
                ifname, strerror(errno));
        return (-1);
    }
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags |= flag;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        return -1;
    }
    printf("set_ok\n");
    return (0);
}

static int if_readlist_proc(char *target)
{
    FILE *fh;
    char buf[512];
    struct interface *ife;
    int err;
    int fd;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
        fprintf(stderr, "Warning: cannot open %s (%s). Limited output.\n",
            _PATH_PROCNET_DEV, strerror(errno));
        return -2;
    }
    fgets(buf, sizeof buf, fh); /* eat line */
    fgets(buf, sizeof buf, fh);

    procnetdev_vsn = procnetdev_version(buf);

    while (fgets(buf, sizeof buf, fh)) {
        char *s, name[IFNAMSIZ];
        s = get_name(name, buf);
        printf("DEV_NAME: %s\n", name);
    }

    fd = socket(PF_INET, SOCK_STREAM, 6);
    if (fd < 0) {
        perror("Bad socket!\n");
        exit(EXIT_FAILURE);
    }

    if (set_flag(fd, "eth0", (IFF_UP | IFF_RUNNING)) < 0) {
        perror("Bad flag!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

static int test_net() {
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

int main(int argc, char *argv[]) {
    const char *source = "proc";
    const char *target = "/proc";

    /* mount proc filesystem */
    if (mount(source, target, "proc", MS_MGC_VAL, NULL) == -1) {
        if (errno != EEXIST) {
            perror("mount");
            return -1;
        }
    }
    printf("Proc filesystem mounted successfully\n");

    if_readlist_proc(NULL);
    //display_net_dev();

    test_net();

    return 0;
}
