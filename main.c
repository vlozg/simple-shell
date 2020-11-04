#include "main_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_LINE 200 /* The maximum length command */

int executeSingleCmd(char* line, int wait_flag);

int main(void)
{
    char line[MAX_LINE]; /* user command */
    char tmp[MAX_LINE];
    int terminated = 0; /* flag to determine when to exit program */

    while (!terminated) {
        wait_flag = 1;
        printf("osh>");
        fflush(stdout);
        
        // Read user command line
        fgets(line, MAX_LINE, stdin);

        for (int i = 0, brk = 0; i < strlen(line); i++){
            if (line[i] == '&')
            {
                
            }
            else if (line[j] == '|')
            {

            }
        }
    }
    return 0;
}

int executeSingleCmd(char* line, int wait_flag)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    
    // Redirect then parse arguments
    parseRedirectCommand(line);
    int n_args = parseCommand(line, args);
    
    // Check for exit command
    if (strcmp(args[0], "exit") == 0
        || strcmp(args[0], "q") == 0
        || strcmp(args[0], "quit") == 0)
    {
        return 0;
    }

    // Check for history argument or !! argument
    if (strcmp(args[0], "history") == 0
        || strcmp(args[0], "!!") == 0)
    {
        return 0;
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
    return 1;
}