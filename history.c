#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char* his[1000];
static int n_his = 0;

char* replaceEx(char* line, char* ex_str);

int historyNewCommand(char* line)
/*
    Replace !! in line of command if possible, then add that command into history stack.

    Parameter:
    -   line (char*): user inputted command.

    Return: (int) 0 if history is empty error occur, else return 1.
*/
{
    if (strstr(line, "!!") != NULL){
        // Return when call this command while the history stack is empty
        if (n_his == 0)
        {
            printf("history: history is empty.\n");
            return 0;
        }

        // Replace every "!!" in line with previous command
        char* tmp = replaceEx(line, his[n_his-1]);
        strcpy(line, tmp);
        free(tmp);
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
    return 1;
}

void printHistory(char* n)
/*
    Print history to the screen.

    Parameter:
    -   n (char*): argument of history command (should be number).
*/
{
    int i = 0;
    // If second arg specified, this will be number of recent command listed in the console
    if (n != NULL)
        i = n_his - atoi(n);
    if (i < 0) i = 0;

    // Print history
    for (; i < n_his; i++)
    {
        printf("%d %s\n",i+1, his[i]);
    }
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