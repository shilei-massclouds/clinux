#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define LINELEN 512

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

    display_net_dev();

    return 0;
}
