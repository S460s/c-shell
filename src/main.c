#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtins.h"
#include "util/util.h"

void handle_input(char *input)
{
  if (strlen(input) == 0)
    return;

  size_t size = 0;
  struct BuiltIn **builtins = create_builtins(&size);
  for (size_t i = 0; i < size; i++)
  {
    struct BuiltIn *current = builtins[i];
    if (strncmp(current->command, input, strlen(current->command)) == 0)
    {
      current->execute(input);
      return;
    }
  }

  char *copy_input = malloc(strlen(input));
  strcpy(copy_input, input);

  char *program_name = strtok(copy_input, " ");
  char *program = command_path(program_name);

  if (program)
  {
    // args should hold pointers to chunks of "strtok"ed string so freeing the string directly should be fine
    char **args = malloc(sizeof(char *) * 16);
    char *copy_input = malloc(strlen(input));
    strcpy(copy_input, input);
    char *program_name = strtok(copy_input, " ");
    args[0] = program_name;

    char *arg;
    size_t i;
    for (i = 1; (arg = strtok(NULL, " ")) != NULL; i++)
    {
      args[i] = arg;
    }
    args[i] = NULL;

    pid_t pid = fork();

    switch (pid)
    {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      execv(program, args);
      exit(EXIT_SUCCESS);
    }

    while (1)
    {
      int status;
      pid_t done = wait(&status);
      if (done == -1)
      {
        if (errno == ECHILD)
          break;
      }
      else
      {
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        { //
          fprintf(stderr, "Pid %d failed\n", done);
          exit(1);
        }
      }
    }
    free(copy_input);
    free(args);
  }
  else
  {
    fprintf(stderr, "%s: command not found\n", input);
  }

  free(copy_input);
  free(program);
}

int main()
{
  init_pwd();
  while (1)
  {
    printf("$ ");
    fflush(stdout);
    char *input = read_line();
    handle_input(input);
    free(input);
  }

  return 0;
}
