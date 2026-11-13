
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void on_signal(int sig) {
    if (sig == SIGINT) {
        const char msg[] = "Nice try! (SIGINT)\n";
        write(1, msg, sizeof(msg) - 1);
    }

    if (sig == SIGTERM) {
        const char msg[] = "Asked politely (SIGTERM), cleaning up...\n";
        write(1, msg, sizeof(msg) - 1);
        _exit(0);
    }
}

int main(void) {
    printf("PID %d: press Ctrl-C, or send me signals from another terminal\n", getpid());
    fflush(stdout);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    if (signal(SIGKILL, on_signal) == SIG_ERR) {
        perror("cannot catch SIGKILL");
    }

    for (;;) {
        pause();
    }
}

