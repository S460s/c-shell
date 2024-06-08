#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>


// returns a string with a \n before the \0 and should be freed
char* read_line(){
  size_t size = 1; // getline automatically "realloc"s more space
  char* buffer = malloc(sizeof(char) * size);
  ssize_t status = getline(&buffer, &size, stdin);
  if(status == -1) {
    fprintf(stderr, "[ERROR] cannot read input from stding\n");
  }
  
  for(size_t i = 0; i < size; i++){
    if(buffer[i] == '\n' || buffer[i] == '\r'){
      buffer[i] = '\0';
      break;
    }
  }

  return buffer;
}

char* command_path(char* name){
    char* env_path = getenv("PATH");

    char* PATH = malloc(strlen(env_path));// strtok mutates the PATH string 
    strcpy(PATH, env_path);

    char* dirpath = strtok(PATH, ":"); 

    while(dirpath){
      DIR* directory = opendir(dirpath);

      if(directory == NULL){
        dirpath = strtok(NULL, ":");
        continue;
      }

      struct dirent* file;
      while((file = readdir(directory))){
        if(strcmp(file->d_name, name) == 0){
          char* result = malloc(strlen(name) + strlen(dirpath) + strlen(file->d_name) + 8);
          sprintf(result ,"%s/%s", dirpath, file->d_name);
          closedir(directory);
          free(PATH);
          return result;
        }
      }

        dirpath = strtok(NULL, ":");
        closedir(directory);
    }

    free(PATH);
    return NULL;
}

void handle_input(char* input){
  char* copy_input = malloc(strlen(input));
  strcpy(copy_input, input);

  char* program_name = strtok(copy_input, " ");
  char* program = command_path(program_name);

  if(program){
    // args should hold pointers to chunks of "strtok"ed string so freeing the string directly should be fine
    char** args = malloc(sizeof(char*) * 16);
    char* copy_input = malloc(strlen(input));
    strcpy(copy_input, input);
    char* program_name = strtok(copy_input, " ");
    args[0] = program_name;

    char* arg;
    size_t i;
    for(i = 1; (arg = strtok(NULL, " ")) != NULL; i++){
      args[i] = arg;
    }
    args[i] = NULL;

    pid_t pid = fork();

    switch (pid) {
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
      case 0:
        execv(program, args);
        exit(EXIT_SUCCESS);
    }


    while(1){
    int status;
    pid_t done = wait(&status);
    if(done == -1){
      if(errno == ECHILD) break;
    }else {
      if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){ // 
        fprintf(stderr, "Pid %d failed\n", done);
        exit(1);
        }
      }
    }
    free(copy_input);
    free(args);
  }
  else if(strncmp("exit", input, 4) == 0)
  {
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
        return;
      }

      exit(exit_status);
  }
  else if(strncmp("echo", input, 4) == 0){
    printf("%s\n", input + 5);
  }
  else if(strncmp("type", input, 4) == 0){
    char builtins[][16] = {"echo", "type", "exit"};
    char* arguments = input + 5;

    for(size_t i = 0; i < sizeof(builtins) / 16; i++){
      if(strcmp(builtins[i], arguments) == 0){
        printf("%s is a shell builtin\n", arguments);
        return;
      }
    }

    char* result = command_path(arguments);
    if(result != NULL){
      printf("%s is %s\n", arguments, result);
      free(result);
      return;
    }

    printf("%s: not found\n", arguments);
  }
  else
  {
      fprintf(stderr ,"%s: command not found\n", input);
  }  
  free(copy_input);
  free(program);
}

int main()
{
  while (1) {
    printf("$ ");
    fflush(stdout);
    char* input = read_line();
    handle_input(input);
    free(input);
  }

  return 0;
}
