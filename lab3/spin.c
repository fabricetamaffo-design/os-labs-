
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int seconds = argc > 1 ? atoi(argv[1]) : 10;
    volatile unsigned long x;

    for (int s = 0; s < seconds; s++) {
        unsigned long loops = 0;
        time_t end = time(NULL) + 1;

        while (time(NULL) < end) {
            for (x = 0; x < 100000; x++);
            loops++;
        }

        printf("[pid %d] second %2d: %6lu units of work\n",
               getpid(), s + 1, loops);
        fflush(stdout);
    }

    return 0;
}
