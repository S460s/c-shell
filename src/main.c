#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>

#include "builtins/builtins.h"
#include "util/util.h"

ssize_t handle_input(char **input, size_t count)
{
  char *program_name = input[0];
  ssize_t ok = 0;

  size_t size = 0;
  struct BuiltIn **builtins = create_builtins(&size);
  for (size_t i = 0; i < size; i++)
  {
    struct BuiltIn *current = builtins[i];
    if (strncmp(current->command, program_name, strlen(current->command)) == 0)
    {
      ok = current->execute(input, count);
      free_builtins(builtins, size);
      return ok;
    }
  }
  free_builtins(builtins, size);

  char *program = command_path(program_name);
  if (program)
  {
    pid_t pid = fork();

    switch (pid)
    {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      ok = execv(program, input);
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
        {
          fprintf(stderr, "Pid %d failed\n", done);
          return -1;
        }
      }
    }
  }
  else
  {
    fprintf(stderr, "%s: command not found\n", program_name);
    ok = -1;
  }

  free(program);
  return ok;
}

// returned value should be freed
char **parse_input(char *input, size_t *count)
{
  char *command_name = strtok(input, " ");
  if (command_name == NULL)
  {
    free(input);
    return NULL;
  }

  (*count)++;
  char **argv = malloc(sizeof(char *) * 16); // will need to be dynamic
  argv[0] = command_name;

  // there needs to be a dynamic array or a linked list to add to the new data
  size_t i = 1;
  char *param;
  while ((param = strtok(NULL, " ")))
  {
    (*count)++;
    if (param[0] == '$')
    {
      char *envvar = getenv(param + 1);
      argv[i] = envvar;
      continue;
    }
    argv[i] = param;
  }

  return argv;
}

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

int main()
{
  ssize_t ok = 0;
  while (1)
  {
    char pwd[PATH_MAX];
    char *pathname = getcwd((char *)pwd, PATH_MAX);

    if (ok == -1)
      printf("%s[%s]$ %s", ANSI_COLOR_RED, pwd, ANSI_COLOR_RESET);
    else
      printf("[%s]$ ", pwd);

    fflush(stdout);
    char *input = read_line();

    size_t count = 0;
    char **parsed = parse_input(input, &count);
    if (parsed == NULL)
      continue;

    ok = handle_input(parsed, count);
    free(parsed[0]);
    free(parsed);
  }

  return 0;
}
