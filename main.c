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
        printf("osh>");
        fflush(stdout);
        
        // Read user command line
        fgets(line, MAX_LINE, stdin);
        //Remove trailing character at the end of line
        int len = strlen(line) - 1;
        while (line[len] == '\r' || line[len] == '\n' || line[len] == ' ')
        {
            line[len--] = '\0';
            if (len < 0) break;
        }

        // Extract single command in between symbol & and symbol |
        // Single command is a command that only have arguments and redirect sub-commands, does not contain bg (&) and pipe command (|)
        int brk = 0;
        for (int i = 0; i < strlen(line); i++){
            if (line[i] == '&')
            {
                // Extract single command and set a new break point
                strncpy(tmp, &line[brk], i-brk);
                tmp[i-brk] = '\0';
                brk = i+1;
                
                // Execute that command
                terminated = executeSingleCmd(tmp, 0);
            }
            else if (line[i] == '|')
            {

            }
        }
        
        // Execute the remained command after & and | (if exist)
        if (brk < strlen(line))
        {
            strncpy(tmp, &line[brk], strlen(line)-brk);
            tmp[strlen(line)-brk] = '\0';
            terminated = executeSingleCmd(tmp, 1);
        }
    }
    return 0;
}

int executeSingleCmd(char* line, int wait_flag)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int o = dup(STDOUT_FILENO),
        i = dup(STDIN_FILENO);
    // Redirect then parse arguments
    parseRedirectCommand(line);
    int n_args = parseCommand(line, args);
    
    // Check for exit command
    if (strcmp(args[0], "exit") == 0
        || strcmp(args[0], "q") == 0
        || strcmp(args[0], "quit") == 0)
    {
        dup2(i, STDIN_FILENO);
        dup2(o, STDOUT_FILENO);
        close(i); close(o);
        return 1;
    }

    // Check for history argument or !! argument
    if (strcmp(args[0], "history") == 0
        || strcmp(args[0], "!!") == 0)
    {
        dup2(i, STDIN_FILENO);
        dup2(o, STDOUT_FILENO);
        close(i); close(o);
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
                dup2(i, STDIN_FILENO);
                dup2(o, STDOUT_FILENO);
                close(i); close(o);
                printf("%s: command not found\n", args[0]);
                return 1;
            }
            break;
        default:
            if (wait_flag == 1)
                waitpid(new_pid ,&status, 0);
            break;
    }

    dup2(i, STDIN_FILENO);
    dup2(o, STDOUT_FILENO);
    close(i);
    close(o);
    return 0;
}