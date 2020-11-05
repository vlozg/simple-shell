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

char* replaceEx(char* line, char* ex_str)
/*
    Replace !! in command with previous command and return.

    Parameter:
    -   line (char*): single command containt !!.
    -   ex_str (char*): previous command in replace for !! in this command.

    Return: (char*) command with !! have been replaced.
*/
{
    // Count number of !! occurents.
    int count = 0;
    char* p = strstr(line, "!!");
    while (p != NULL)
    {
        count++;
        p = strstr(p+2, "!!");
    }

    // Estimate length of replaced string
    int len = strlen(line)+count*(strlen(ex_str) - 2);
    
    // Make a new string
    char* reslt = (char*) malloc(len*sizeof(char));
    strcpy(reslt, line);
    
    // Iterate and replace
    p = strstr(line, "!!");
    int ir;  
    while (p != NULL)
    {
        ir = strstr(reslt, "!!") - reslt;    /* Start at first occurence of !!*/ 
        strcpy(&reslt[ir],ex_str);
        ir += strlen(ex_str);
        strcpy(&reslt[ir], p+2);
        p = strstr(p+2, "!!");
    }
    reslt[len] = '\0';
    return reslt;
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
    static char* his[1000];
    static int n_his = 0;
    int replace_flag = 0;   /* If !! in command is replaced, must free memory after exec done */
    
    if (strstr(line, "!!") != NULL){
        // Return when call this command while the history stack is empty
        if (n_his == 0)
        {
            printf("history: history is empty.\n");
            return 0;
        }

        // Replace every "!!" in line with previous command
        line = replaceEx(line, his[n_his-1]);
        replace_flag = 1;
    }
    
    // Store history if new command different from the previous
    if (n_his == 0)
    {
        his[n_his] = (char*) malloc(strlen(line)*sizeof(char)+1);
        strcpy(his[n_his], line);
        n_his++;
    } else if (strcmp(his[n_his-1], line) != 0)
    {    
        his[n_his] = (char*) malloc(strlen(line)*sizeof(char)+1);
        strcpy(his[n_his], line);
        n_his++;
    }
    
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int o = dup(STDOUT_FILENO),
        i = dup(STDIN_FILENO);
    // Redirect then parse arguments
    parseRedirectCommand(line);
    int n_args = parseCommand(line, args);

    // Check for history argument
    if (strcmp(args[0], "history") == 0)
    {
        int i = 0;
        // If second arg specified, this will be number of recent command listed in the console
        if (args[1] != NULL)
            i = n_his - atoi(args[1]);
        if (i < 0) i = 0;

        // Print history
        for (; i < n_his; i++)
        {
            printf("%d %s\n",i+1, his[i]);
        }
        dup2(i, STDIN_FILENO);
        dup2(o, STDOUT_FILENO);
        close(i); close(o);
        if (replace_flag == 1)
            free(line);
        return 0;
    }

    // Check for exit command
    if (strcmp(args[0], "exit") == 0
        || strcmp(args[0], "q") == 0
        || strcmp(args[0], "quit") == 0)
    {
        dup2(i, STDIN_FILENO);
        dup2(o, STDOUT_FILENO);
        close(i); close(o);
        if (replace_flag == 1)
            free(line);
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
                dup2(i, STDIN_FILENO);
                dup2(o, STDOUT_FILENO);
                close(i); close(o);
                printf("%s: command not found\n", args[0]);
                if (replace_flag == 1)
                    free(line);
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
    close(i); close(o);
    if (replace_flag == 1)
        free(line);
    return 0;
}