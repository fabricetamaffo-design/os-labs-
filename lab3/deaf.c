
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    printf("PID %d: working... (try to stop me)\n", getpid());
    fflush(stdout);

    for (;;) {
        sleep(1);
    }
}
