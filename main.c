#include "main_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_LINE 200 /* The maximum length command */

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char line[MAX_LINE]; /* user command */
    int terminated = 0; /* flag to determine when to exit program */
    int wait_flag = 1; /* flag to determine whether to wait for child process or not */

    while (!terminated) {
        wait_flag = 1;
        printf("osh>");
        fflush(stdout);
        
        // Read user command line and parse
        fgets(line, MAX_LINE, stdin);
        int n_args = parseCommand(line, args);
        
        // Check for exit command
        if (strcmp(args[0], "exit") == 0
            || strcmp(args[0], "q") == 0
            || strcmp(args[0], "quit") == 0)
        {
            terminated = 1;
            continue;
        }

        // Check for & argument
        
        if (strcmp(args[n_args-1], "&") == 0)
        {
            wait_flag = 0;
            args[--n_args] = NULL;
        }

        // Fork a child process
        pid_t new_pid;
        int status;
        new_pid = fork();
        switch (new_pid)
        {
            case -1: 
                printf( "Cannot fork any new process." ); 
                break;
            case 0:
                if (execvp(args[0], args) < 0) 
                {
                    printf("%s: command not found\n", args[0]);
                    return 0;
                }
                break;
            default:
                if (wait_flag == 1)
                    waitpid(new_pid ,&status, 0);
                break;
        }
    }
    return 0;
}