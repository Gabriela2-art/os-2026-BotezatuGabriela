#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command: ls /usr/bin | grep x | wc -l */
static void close_all(int a[2], int b[2])
{
    close(a[0]); close(a[1]);
    close(b[0]); close(b[1]);
}

int main(void)
{
    int a[2], b[2];  /* a: ls -> grep, b: grep -> wc */
    if (pipe(a) < 0 || pipe(b) < 0) {
        perror("pipe");
        return 1;
    }

    if (fork() == 0) {
        close(1);
        dup(a[1]);
        close_all(a, b);
        execlp("ls", "ls", "/usr/bin", (char *)NULL);
        perror("exec ls");
        exit(127);
    }

    if (fork() == 0) {
        close(0);
        dup(a[0]);
        close(1);
        dup(b[1]);
        close_all(a, b);
        execlp("grep", "grep", "x", (char *)NULL);
        perror("exec grep");
        exit(127);
    }

    if (fork() == 0) {
        close(0);
        dup(b[0]);
        close_all(a, b);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }

    close_all(a, b);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    return 0;
}
