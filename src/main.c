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

#include "builtins/builtins.h"
#include "util/util.h"

void handle_input(char **input)
{
  /* if (strlen(input) == 0)
    return;

  size_t size = 0;
  struct BuiltIn **builtins = create_builtins(&size);
  for (size_t i = 0; i < size; i++)
  {
    struct BuiltIn *current = builtins[i];
    if (strncmp(current->command, input, strlen(current->command)) == 0)
    {
      current->execute(input);
      free_builtins(builtins, size);
      return;
    }
  }
  free_builtins(builtins, size);

*/
  
  // char *copy_input = malloc(strlen(input));
  // strcpy(copy_input, input);

  // char *program_name = strtok(copy_input, " ");
  // char *program = command_path(program_name);

  char* program_name = input[0];
  char *program = command_path(program_name);

  if (program)
  {
    // args should hold pointers to chunks of "strtok"ed string so freeing the string directly should be fine
    // char **args = malloc(sizeof(char *) * 16);
    // char *copy_input = malloc(strlen(input));
    // strcpy(copy_input, input);
    // char *program_name = strtok(copy_input, " ");
    // args[0] = program_name;

    // char *arg;
    // size_t i;
    // for (i = 1; (arg = strtok(NULL, " ")) != NULL; i++)
    // {
      // args[i] = arg;
    // }
    // args[i] = NULL;

    pid_t pid = fork();

    switch (pid)
    {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      execv(program, input);
      exit(EXIT_SUCCESS);
    }

    printf("Hello from handle input\n");

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
  }
  else
  {
    fprintf(stderr, "%s: command not found\n", program_name);
  }

  free(program);
}

// this can later tokenize the inputed string into an array
// returned value should be freed
char** parse_input(char* input, size_t* count){ 
  size_t length = strlen(input) + 1;
  char* copy = malloc(length * (sizeof(char))); // is sizeof char always 1? 
  strncpy(copy, input, length);

  char* command_name = strtok(copy, " ");
  printf("command_name: %s\n", command_name);

  *count = 16;
  char** argv = malloc(sizeof(char*) * (*count)); // will need to be dynamic
  argv[0] = command_name;

  // there needs to be a dynamic array or a linked list to add to the new data
  size_t i = 1;
  char* param;
  while((param = strtok(NULL, " "))) {
      printf("param: %s\n", param);
      if(param[0] == '$'){
        char* envvar = getenv(param + 1);      
        printf("env var: %s\n", envvar);
        argv[i] = envvar;
        continue;
    }
    argv[i] = param;
  }

return argv;
}

int main()
{
  init_pwd();
  while (1)
  {
    printf("$ ");
    fflush(stdout);
    char *input = read_line();
    size_t count = 0;
    char** parsed = parse_input(input, &count);
    handle_input(parsed);

    free(input);
    free(parsed[0]);// parsed is an array of pointers to chucnks of strings
    free(parsed);
  }

  return 0;
}
