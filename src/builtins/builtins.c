#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../util/util.h"
#include "builtins.h"

ssize_t execute_exit(char *input)
{
  char *arguments = input + 4;
  errno = 0;
  char *endptr;
  int exit_status = strtol(arguments, &endptr, 10);

  if (errno != 0)
  {
    perror("strtol");
    exit(EXIT_FAILURE);
  }

  if (endptr == arguments)
  {
    fprintf(stderr, "No exit status was found\n");
    return -1;
  }

  exit(exit_status);
}

ssize_t execute_echo(char *input)
{
  printf("%s\n", input + 5);
  return 0;
}

ssize_t execute_type(char *input)
{
  char builtins[][16] = {"echo", "type", "exit"};
  char *arguments = input + 5;

  for (size_t i = 0; i < sizeof(builtins) / 16; i++)
  {
    if (strncmp(builtins[i], arguments, strlen(builtins[i])) == 0)
    {
      printf("%s is a shell builtin\n", arguments);
      return 0;
    }
  }

  char *result = command_path(arguments);
  if (result != NULL)
  {
    printf("%s is %s\n", arguments, result);
    free(result);
    return 0;
  }

  printf("%s: not found\n", arguments);
  return -1;
}

char current_path[PATH_MAX];
char envpath[PATH_MAX];

void init_pwd()
{
  char pwd[PATH_MAX];
  char *pathname = getcwd((char *)current_path, PATH_MAX);

  if (pathname == NULL)
  {
    fprintf(stderr, "getcwd error\n");
    exit(1);
  }
}

ssize_t execute_pwd(char *input)
{
  printf("TEST\n");
  printf("%s\n", current_path);
  return 0;
}

ssize_t execute_cd(char *input)
{
  char *new_path = input + 3;
  int ok = access(new_path, X_OK);
  if (ok == -1)
  {
    fprintf(stderr, "invalid path: %s\n", new_path);
    return -1;
  }

  strncpy(current_path, new_path, PATH_MAX - 16); // I had switched destination and src and it crashed the whole system
  snprintf(envpath, PATH_MAX, "PWD=%s", current_path);
  printf("newenv %s\n", envpath);
  int okenv = putenv(envpath); // the envpath needs to live afterwards as putenv doesn't copy the string but gives a pointer to it
  if (okenv == -1)
  {
    fprintf(stderr, "error cannot set env\n");
    return -1;
  }
  return 0;
}

struct BuiltIn **create_builtins(size_t *size)
{
  struct BuiltIn *_echo = malloc(sizeof(struct BuiltIn));
  strcpy(_echo->command, "echo");
  _echo->execute = execute_echo;

  struct BuiltIn *_exit = malloc(sizeof(struct BuiltIn));
  strcpy(_exit->command, "exit");
  _exit->execute = execute_exit;

  struct BuiltIn *_type = malloc(sizeof(struct BuiltIn));
  strcpy(_type->command, "type");
  _type->execute = execute_type;

  struct BuiltIn *_pwd = malloc(sizeof(struct BuiltIn));
  strcpy(_pwd->command, "pwd");
  _pwd->execute = execute_pwd;

  struct BuiltIn *_cd = malloc(sizeof(struct BuiltIn));
  strcpy(_cd->command, "cd");
  _cd->execute = execute_cd;

  *size = 5;
  struct BuiltIn **builtins = malloc(sizeof(struct BuiltIn *) * (*size));
  builtins[0] = _echo;
  builtins[1] = _exit;
  builtins[2] = _type;
  builtins[3] = _pwd;
  builtins[4] = _cd;

  return builtins;
}
