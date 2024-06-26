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

ssize_t execute_exit(char **input, size_t count)
{
  if (count == 1)
  {
    fprintf(stderr, "No exit status was found\n");
    return -1;
  }

  errno = 0;
  char *endptr;
  int exit_status = strtol(input[1], &endptr, 10);

  if (errno != 0)
  {
    perror("strtol");
    exit(EXIT_FAILURE);
  }

  exit(exit_status);
}

ssize_t execute_echo(char **input, size_t count)
{
  printf("%s\n", input[1]);
  return 0;
}

ssize_t execute_type(char **input, size_t count)
{
  char builtins[][16] = {"echo", "type", "exit", "cd"};
  char *program_name = input[0];

  for (size_t i = 0; i < sizeof(builtins) / 16; i++)
  {
    if (strncmp(builtins[i], program_name, strlen(builtins[i])) == 0)
    {
      printf("%s is a shell builtin\n", program_name);
      return 0;
    }
  }

  char *result = command_path(program_name);
  if (result != NULL)
  {
    printf("%s is %s\n", program_name, result);
    free(result);
    return 0;
  }

  printf("%s: not found\n", program_name);
  return -1;
}

char current_path[PATH_MAX];
#define ENV_MAX PATH_MAX + 8
char envpath[ENV_MAX];

void init_pwd()
{
  char *pathname = getcwd((char *)current_path, PATH_MAX);

  if (pathname == NULL)
  {
    fprintf(stderr, "getcwd error\n");
    exit(1);
  }
}

ssize_t execute_pwd(char **input, size_t count)
{
  if (strlen(current_path) == 0)
    init_pwd();
  printf("%s\n", current_path);
  return 0;
}

ssize_t update_cwd(char *newpath)
{
  errno = 0;
  int ok = access(newpath, X_OK);
  if (ok == -1)
  {
    if (errno == EACCES)
    {
      fprintf(stderr, "cd: permission denied: %s\n", newpath);
    }
    else
    {
      fprintf(stderr, "cd: invalid path: %s\n", newpath);
    }
    return -1;
  }

  int chdirok = chdir(newpath);
  if (chdirok == -1)
  {
    fprintf(stderr, "error chdir\n");
    return -1;
  }

  char *pathname = getcwd((char *)current_path, PATH_MAX);
  if (pathname == NULL)
  {
    fprintf(stderr, "error setting cwd\n");
    return -1;
  }

  snprintf(envpath, ENV_MAX, "PWD=%s", current_path);
  int okenv = putenv(envpath); // the envpath needs to live afterwards as putenv doesn't copy the string but gives a pointer to it
  if (okenv == -1)
  {
    fprintf(stderr, "error cannot set env\n");
    return -1;
  }

  return 0;
}

ssize_t execute_cd(char **input, size_t count)
{
  if (count == 1 || input[1][0] == '~')
  {
    char *home = getenv("HOME");
    if (home == NULL)
    {
      fprintf(stderr, "error can't get home directory\n");
      return -1;
    }

    ssize_t ok = update_cwd(home);
    return ok;
  }

  ssize_t ok = update_cwd(input[1]);
  return ok;
}

void free_builtins(struct BuiltIn **builtins, size_t size)
{
  for (int i = 0; i < size; i++)
  {
    free(builtins[i]);
  }
  free(builtins);
}

struct BuiltIn *builtin(char *command_name, ssize_t (*execute)(char **input, size_t count))
{
  struct BuiltIn *_builtin = malloc(sizeof(struct BuiltIn));
  strcpy(_builtin->command, command_name);
  _builtin->execute = execute;
  return _builtin;
};

struct BuiltIn **create_builtins(size_t *size)
{
  struct BuiltIn *_echo = builtin("echo", execute_echo);
  struct BuiltIn *_exit = builtin("exit", execute_exit);
  struct BuiltIn *_type = builtin("type", execute_type);
  struct BuiltIn *_pwd = builtin("pwd", execute_pwd);
  struct BuiltIn *_cd = builtin("cd", execute_cd);

  // dynamic array might be better
  *size = 5;
  struct BuiltIn **builtins = malloc(sizeof(struct BuiltIn *) * (*size));
  builtins[0] = _echo;
  builtins[1] = _exit;
  builtins[2] = _type;
  builtins[3] = _pwd;
  builtins[4] = _cd;

  return builtins;
}
