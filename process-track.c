#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>     /* For getopt() execvp()*/

#include "error.h"
 enum
  {
    EXIT_CANCELED = 125, /* Internal error prior to exec attempt.  */
    EXIT_CANNOT_INVOKE = 126, /* Program located, but not usable.  */
    EXIT_ENOENT = 127 /* Could not find program to exec.  */
  };
int main (int argc, char **argv) {
    int opt;
    printf("argc = %d\n", argc);
    char *command_line [100];
    int PID;
    char *c;
    int saved_errno;
    char *token;
    while ((opt = getopt(argc, argv, "o:c:")) != EOF) 
        switch(opt) {
            case 'o':
                printf("optint o: output file name is %s\n",optarg);
                break;
            case 'c':
                printf("run %s\n",optarg);
                c = optarg;
                break;
            default:
                printf("not supported option %s\n",optarg);
                break;
        }

    token = strtok(c, " ");
    int i = 0;
    while (token != NULL) {
        command_line[i++] = token;
        token = strtok(NULL, " ");
    }
    command_line[i++] = NULL;
    pid_t pid = fork();
    if (pid < 0) {
         error (EXIT_CANCELED, errno, "cannot fork");
    } else if (pid == 0) {
        PID = getpid();
        printf ("child PID IS : %d\n",getpid());
        execvp(command_line[0],command_line);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        printf("Failed to fork\n");
        exit(1);
    }

}
