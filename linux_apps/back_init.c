#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_SIZE 1024
#define MAX_ARG_COUNT 9
#define SBIN_PATH "/btp/sbin"

int exec_cmd(char *cmdline)
{
    pid_t pid;
    char *token;
    char *envp[] = { "PATH=/bin:/sbin", NULL };
    char *argv[MAX_ARG_COUNT + 1]; // argv with end 'NULL'
    int argc = 0;

    printf("cmd: %s\n", cmdline);
    token = strtok(cmdline, " \t"); // split by 'space' and 'tab'
    while (token != NULL && argc < MAX_ARG_COUNT) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;

    pid = vfork();
    if (pid == 0) {
        char exe_path[256];
        if (argv[0][0] == '/') {
            strcpy(exe_path, argv[0]);
        } else {
            sprintf(exe_path, "%s/%s", SBIN_PATH, argv[0]);
        }
        execve(exe_path, argv, envp);
        exit(0);
    }
    waitpid(pid, NULL, 0);

    return 0;
}

char *ltrim(char *str)
{
    int start = 0;
    while (isspace(start)) {
        start++;
    }
    return str + start;
}

int run_one_script(const char *path, const char *name)
{
    FILE *fp;
    char script[256];
    char line[LINE_SIZE];

    sprintf(script, "%s/%s", path, name);
    printf("script: %s\n", script);
    fp = fopen(script, "r");
    if (fp == NULL) {
        fprintf(stderr, "open script err");
        return -1;
    }

    while (fgets(line, LINE_SIZE, fp) != NULL) {
        char *p = strchr(line, '\n');
        if (p != NULL) {
            *p = '\0';
        }
        p = ltrim(line);
        if (strlen(p) == 0) {
            break;
        }
        if (p[0] == '#') {
            continue;
        }
        if (exec_cmd(p) < 0) {
            fprintf(stderr, "exec script err");
        }
    }

    fclose(fp);
    return 0;
}

void run_scripts(const char *path)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "bad dir path %s", path);
        exit(-1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (run_one_script(path, entry->d_name) != 0) {
            fprintf(stderr, "run script %s/%s error", path, entry->d_name);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    printf("[initd] startup ..\n");
    run_scripts("/etc/init.d");
    return 0;
}
