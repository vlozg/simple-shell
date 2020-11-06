#ifndef MAIN_HEADER
#define MAIN_HEADER

int parseCommand(char* line, char* cmd[]);
void parseRedirectCommand(char* line);
int historyNewCommand(char* line);
void printHistory(char* n);

#endif