#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liblouisutdml.h"

int
main (int argc, char **argv)
{
  lbu_translateFile ("preferences.cfg", argv[1], "trans1", NULL, NULL, 
  0);
  lbu_translateFile ("preferences.cfg", argv[1], "trans2", NULL, NULL, 
  0);
  lbu_translateFile ("preferences.cfg", argv[1], "trans3", NULL, NULL, 
  0);
  lbu_free ();
  return 0;
}
