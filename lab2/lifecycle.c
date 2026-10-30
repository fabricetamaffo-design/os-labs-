
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    printf("parent pid=%d\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        printf("child pid=%d parent=%d\n",
               getpid(), getppid());

        execlp("ls", "ls", "/nothing", NULL);

        perror("exec");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    printf("child exited with %d\n",
           WEXITSTATUS(status));

    return 0;
}
