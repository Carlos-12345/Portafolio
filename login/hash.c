#include <stdio.h>
#include <crypt.h>

int
main (int argc, char *argv[])
{
  if (argc != 2)
    {
        printf ("uso: %s <password> \n", argv[0]);
        return 1;
    }
  char salt[32];
  snprintf (salt, sizeof (salt), "$2b$12$%.22s", "abcdefghijklmnopqrstuv");
  char *hash = crypt (argv[1], salt);
  printf ("%s \n", hash); 

  return 0;
}  