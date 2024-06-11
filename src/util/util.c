#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include "util.h"

char *read_line()
{
  size_t size = 0; // getline automatically "realloc"s more space
  char *buffer = NULL;
  char *buffer_addr = buffer; 
  ssize_t status = getline(&buffer, &size, stdin);
  if (status == -1)
  {
    fprintf(stderr, "[ERROR] cannot read input from stding\n");
  }

  for (size_t i = 0; i < size; i++)
  {
    if (buffer[i] == '\n' || buffer[i] == '\r')
    {
      buffer[i] = '\0';
      break;
    }
  }

  printf("buffer later: %p\n", buffer);
  return buffer;
}

char *command_path(char *name)
{
  char *env_path = getenv("PATH");

  char *PATH = malloc(strlen(env_path)); // strtok mutates the PATH string
  strcpy(PATH, env_path);

  char *dirpath = strtok(PATH, ":");

  while (dirpath)
  {
    DIR *directory = opendir(dirpath);

    if (directory == NULL)
    {
      dirpath = strtok(NULL, ":");
      continue;
    }

    struct dirent *file;
    while ((file = readdir(directory)))
    {
      if (strcmp(file->d_name, name) == 0)
      {
        char *result = malloc(strlen(name) + strlen(dirpath) + strlen(file->d_name) + 8);
        sprintf(result, "%s/%s", dirpath, file->d_name);
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
