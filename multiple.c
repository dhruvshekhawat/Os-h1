#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>
int main()
{
  char *ls[] = {"ls", "-la", NULL};
  char *grep[] = {"grep", "readfile", NULL};
  char **cmd[] = {ls, grep, grep, grep, grep, NULL};

  loop_pipe(cmd);
  return (0);
}

