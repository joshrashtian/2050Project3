#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * PROJECT REQUIREMENTS
 *  exit
 *  cd <name>
 *  ls
 *  mkdir <name>
 *  touch <name>
 * */

int get_cmd(char *string) {
  if (string == NULL)
    exit(1);
  char command[64];
  int i = 0;
  while (*string != ' ' && *string != '\0' && *string != '\n' && i < 63) {
    command[i] = *string;
    string++;
    i++;
  }
  command[i] = '\0';

  if (strcmp(command, "exit") == 0)
    return 0;
  if (strcmp(command, "cd") == 0)
    return 1;
   if (strcmp(command, "ls") == 0)
    return 2;
   if (strcmp(command, "mkdir") == 0)
    return 3;
   if (strcmp(command, "touch") == 0)
    return 4;
  
  return -1;
}

char *get_argument(char *ns) {
  if (*ns == NULL)
    exit(1);
  // since we will only ever need one argument, we can outright just start
  // aftert the first space.
  while (*ns == ' ')
    ns++;
  while (*ns != '\0' && *ns != ' ')
    ns++; // skip the cmd
  while (*ns == ' ')
    ns++; // spaces before arg

  if (*ns == '\0' || *ns == '\n')
    return NULL;

  char *start = ns;
  while (*ns != '\0' && *ns != ' ' && *ns != '\n')
    ns++;
  *ns = '\0';
  return start;
}

int cd(char * path) {

}

int main(int argc, char **argv) {
  int running = 1;
  // for now, just make the buffer 64
  char buf[64];
  char * path = ".";

  while (running) {
    if (!fgets(buf, sizeof buf, stdin))
      break;
    int outcome = get_cmd(buf);
    char *args = get_argument(buf);
    
    switch (outcome) {
        case 0: running = 0;
        case 1: 
        case 2:
        case 3:
        case 4:
        default:
    }

    
  }

  return 0;
}
