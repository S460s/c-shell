#ifndef BUILTIN_SEEN
#define BUILTIN_SEEN
#include <stdio.h>
#include <sys/types.h>

struct BuiltIn
{
  char command[32];
  ssize_t (*execute)(char **input, size_t count);
};

struct BuiltIn **create_builtins(size_t *size);
void free_builtins(struct BuiltIn** builtins ,size_t size);

void init_pwd();

#endif // !BUILTIN_SEEN
