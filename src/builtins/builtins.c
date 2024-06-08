#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../util/util.h"

ssize_t execute_exit(char* input){
      char* arguments = input + 4;
      errno = 0;
      char* endptr;
      int exit_status = strtol(arguments, &endptr, 10);


      if(errno != 0){
        perror("strtol");
        exit(EXIT_FAILURE);
      }

      if(endptr == arguments){
        fprintf(stderr, "No exit status was found\n");
        return -1;
      }

      exit(exit_status);
  }

ssize_t execute_echo(char* input){
    printf("%s\n", input + 5);
    return 0;
  }

ssize_t execute_type(char* input){
    char builtins[][16] = {"echo", "type", "exit"};
    char* arguments = input + 5;

    for(size_t i = 0; i < sizeof(builtins) / 16; i++){
      if(strcmp(builtins[i], arguments) == 0){
        printf("%s is a shell builtin\n", arguments);
        return 0;
      }
    }

    char* result = command_path(arguments);
    if(result != NULL){
      printf("%s is %s\n", arguments, result);
      free(result);
      return 0;
    }

    printf("%s: not found\n", arguments);
    return -1;
  }

struct BuiltIn** create_builtins(size_t* size){
  struct BuiltIn* _echo = malloc(sizeof(struct BuiltIn));
  strcpy(_echo->command, "echo");
  _echo->execute = execute_echo;

  struct BuiltIn* _exit = malloc(sizeof(struct BuiltIn));
  strcpy(_exit->command, "exit");
  _echo->execute = execute_exit;

  struct BuiltIn* _type = malloc(sizeof(struct BuiltIn));
  strcpy(_type->command, "type");
  _echo->execute = execute_type;


  *size = 3;
  struct BuiltIn** builtins = malloc(sizeof(struct BuiltIn*) * (*size));
  builtins[0] = _echo;
  builtins[1] = _exit;
  builtins[2] = _type;

  return builtins;
}
