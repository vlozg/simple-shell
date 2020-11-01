#include <stdio.h>
#include <string.h>

int parseCommand(char* line, char* cmd[])
{
    char * pch;
    int len = 0;

    pch = strtok(line," ");
    while (pch != NULL)
    {
        cmd[len++] = pch;
        pch = strtok (NULL, " ");
    }
    return len;
}