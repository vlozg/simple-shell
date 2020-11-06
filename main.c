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
    int stdi = dup(STDIN_FILENO),
        stdo = dup(STDOUT_FILENO);

    while (!terminated) {
        printf("osh>");
        fflush(stdout);
        
        // Read user command line
        fgets(line, MAX_LINE, stdin);
        // Remove trailing character at the end of line
        int len = strlen(line) - 1;
        while (line[len] == '\r' || line[len] == '\n' || line[len] == ' ')
        {
            line[len--] = '\0';
            if (len < 0) break;
        }

        // Try to replace !! in command and add it to history stack
        if (historyNewCommand(line) == 0)
            continue;

        // Extract single command in between symbol & and symbol |
        // Single command is a command that only have arguments and redirect sub-commands, does not contain bg (&) and pipe command (|)
        int brk = 0;
        int i;
        for (i = 0; i < strlen(line); i++){
            if (line[i] == '&')
            {
                // Extract single command and set a new break point
                strncpy(tmp, &line[brk], i-brk);
                tmp[i-brk] = '\0';
                brk = i+1;
                
                // Execute that command
                terminated = executeSingleCmd(tmp, 0);

                // Restore stdin in case that command is executed after | symbol
                dup2(stdi, STDIN_FILENO);
            }
            else if (line[i] == '|')
            {
                // Initiate a new pipe
                int data_pipe[2];
                if (pipe(data_pipe) == -1)
                {
                    printf("Fail to create a new pipe.\n");
                    return 0;
                }

                // Redirect write pipe for command before | symbol
                // Stdin remain intact
                dup2(data_pipe[1], STDOUT_FILENO);
                close(data_pipe[1]);

                // Extract single command and set a new break point
                strncpy(tmp, &line[brk], i-brk);
                tmp[i-brk] = '\0';
                brk = i+1;
                
                // Execute that command
                terminated = executeSingleCmd(tmp, 0);
                
                // Restore Stdout for the command after | symbol
                // Redirect read pipe
                dup2(stdo, STDOUT_FILENO);
                dup2(data_pipe[0], STDIN_FILENO);
                close(data_pipe[0]);
            }
        }
        
        // Execute the remained command after & and | (if exist)
        if (brk < strlen(line))
        {
            strncpy(tmp, &line[brk], strlen(line)-brk);
            tmp[strlen(line)-brk] = '\0';
            terminated = executeSingleCmd(tmp, 1);
            // Restore stdin in case that command is executed after | symbol
            dup2(stdi, STDIN_FILENO);
        }
    }
    close(stdi); close(stdo);
    return 0;
}

int executeSingleCmd(char* line, int wait_flag)
/*
    Execute single command from string.

    Parameter:
    -   line (char*): single command.
    -   wait_flag (int): 
            1 = wait for child process.
            0 = run parallel

    Return: (int) whether to continue running or terminate the shell
        0 = continue
        1 = terminate
*/
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int obacked = dup(STDOUT_FILENO),
        ibacked = dup(STDIN_FILENO);
    
    // Redirect then parse arguments
    parseRedirectCommand(line);
    int n_args = parseCommand(line, args);

    // Check for history argument
    if (strcmp(args[0], "history") == 0)
    {
        printHistory(args[1]);
        dup2(ibacked, STDIN_FILENO);
        dup2(obacked, STDOUT_FILENO);
        close(ibacked); close(obacked);
        return 0;
    }

    // Check for exit command
    if (strcmp(args[0], "exit") == 0
        || strcmp(args[0], "q") == 0
        || strcmp(args[0], "quit") == 0)
    {
        dup2(ibacked, STDIN_FILENO);
        dup2(obacked, STDOUT_FILENO);
        close(ibacked); close(obacked);
        return 1;
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
                dup2(ibacked, STDIN_FILENO);
                dup2(obacked, STDOUT_FILENO);
                close(ibacked); close(obacked);
                printf("%s: command not found\n", args[0]);
                return 1;
            }
            break;
        default:
            if (wait_flag == 1)
                waitpid(new_pid ,&status, 0);
            break;
    }

    dup2(ibacked, STDIN_FILENO);
    dup2(obacked, STDOUT_FILENO);
    close(ibacked); close(obacked);
    return 0;
}