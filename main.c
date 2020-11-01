#include <stdio.h>
#include <unistd.h>
#include "main_header.h"
#define MAX_LINE 200 /* The maximum length command */

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char line[MAX_LINE]; /* user command */
    int terminated = 0; /* flag to determine when to exit program */

    while (!terminated) {
        printf("osh>");
        fflush(stdout);
        
        // Read user command line and parse
        fgets(line, MAX_LINE, stdin);
        int n_args = parseCommand(line, args);
        
        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */
    }
    return 0;
}