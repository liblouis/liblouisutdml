#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liblouisutdml.h"
#define BUFSIZE 10000

int
main (int argc, char **argv)
{
  char inbuf[BUFSIZE];
  widechar outbuf[BUFSIZE];
  FILE *inFile;
  int inlen = 0;
  int outlen = BUFSIZE;
  int k;
  if (!(inFile = fopen (argv[1], "rb")))
    {
      fprintf (stderr, "Can't open infput file '%s'\n", argv[1]);
      exit (1);
    }
  while ((inbuf[inlen++] = fgetc (inFile)) != EOF && inlen < BUFSIZE);
  if (inlen == BUFSIZE)
    {
      fprintf (stderr, "File '%s' too large\n", argv[1]);
      exit (1);
    }
  if (!lbu_translateString ("preferences.cfg", inbuf, inlen, outbuf,
			    &outlen, NULL, NULL, 0))
    {
      fprintf (stderr, "Translation failed.\n");
      exit (1);
    }
  for (k = 0; k < outlen; k++)
    printf ("%c", outbuf[k]);
  return 0;
}
