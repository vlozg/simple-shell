#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

void parseRedirectCommand(char* line, int* ifd, int* ofd)
/*
    Extract redirect command from user inputted command line and redirect the stdout/stdin.

    Parameter:
    -   line (char*): user inputted command line. 
        (Will be modified after this function but it length will remain intact)
    -   ifd (int*): pointer used to return in file descriptor (if redirection occur).
    -   ofd (int*): pointer used to return out file descriptor (if redirection occur).
*/
{
    int c_ifd = STDIN_FILENO,
        c_ofd = STDOUT_FILENO;
    
    int i, j;
    int direct_type = 0; // 0: no direct, 1: stdin redirect, 2: stdout redirect, 3: stdout append redirect
    char tmp[4000];
    for (i = 0; i < strlen(line); i++)
    {
        /*
            Recognize redirect command and remove trace of command after extract
        */
        if (line[i] == '<')
        {
            direct_type = 1;    line[i] = ' ';
        }
        else if (line[i] == '>')
        {
            line[i++] = ' ';
            if (line[i] == '>') 
            {
                direct_type = 3;    line[i] = ' ';
            }
            else 
            {
                direct_type = 2;    i--;
            }
        } 
        else continue;

        /*
            Recognize argument and remove trace of argument after extract
        */
        j = i+1;
        while (line[j] == ' ' || line[j] == '\t') j++;  // Iterate through space between symbol and argument
        i = j;
        while (line[j] != ' ' && line[j] != '\t' && line[j] != '\0') j++;  // Iterate through argument until meet a space character
        
        // Extract file path
        strncpy(tmp, &line[i], j-i);
        tmp[j-i] = '\0';
        memset(&line[i], ' ', j-i);

        /*
            Redirect stdin/stdout using dup2
        */
        switch (direct_type)
        {
        case 1:
            *ifd = open(tmp, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
            dup2(*ifd, c_ifd);
            c_ifd = *ifd;
            break;
        case 2:
            *ofd = open(tmp, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(*ofd, c_ofd);
            c_ofd = *ofd;
            break;
        case 3:
            *ofd = open(tmp, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
            dup2(*ofd, c_ofd);
            c_ofd = *ofd;
            break;
        }
    }
}