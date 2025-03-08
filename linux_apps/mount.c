#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    const char *source = "proc";
    const char *target = "/proc";

    // mount proc filesystem
    if (mount(source, target, "proc", MS_MGC_VAL, NULL) == -1) {
        if (errno != EEXIST) {
            perror("mount");
            return -1;
        }
    }
    printf("File system mounted successfully\n");

    /*
    pid_t pid = vfork();
    if (pid == 0) {
        printf("Child is running ...\n");
        execl("/btp/sbin/procfs", "procfs", NULL);
        exit(0);
    } else {
        int ret = 0;
        waitpid(pid, &ret, 0);
        printf("Parent gets code [%d]\n", ret);
    }
    */
    return 0;
}
