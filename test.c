#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"sleep", "10", NULL,NULL};
        execvp(args[0], args);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        printf("Failed to fork\n");
        exit(1);
    }
    return 0;
}

