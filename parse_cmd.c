#include <stdio.h>
#include <string.h>

int parseCommand(char* line, char* cmd[])
/*
    Return number and array of arguments from user inputted command line.

    Parameter:
    -   line (char*): user inputted command line.
    -   cmd (char* []): empty array used for returning arguments parsed from command line.

    Return: (int) number of arguments parsed from command line.
*/
{
    char * pch;
    int len = strlen(line) - 1; // Use temporarily for length of line


    //Remove trailing character at the end of line
    while (line[len] == '\r' || line[len] == '\n' || line[len] == ' ')
    {
        line[len--] = '\0';
        if (len < 0) break;
    }


    len = 0; // Change len back to the length of array of arguments
    pch = strtok(line," ");
    while (pch != NULL)
    {
        cmd[len++] = pch;
        pch = strtok (NULL, " ");
    }
    cmd[len] = NULL; // Array of arguments must be NULL terminated ( exec() requirement )
    return len;
}