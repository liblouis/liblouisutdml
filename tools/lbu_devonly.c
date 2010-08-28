/* This code is used by the developers for testjng and debugging.
   It is altered from time to time and is undocumented. */

/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   JJB Software, Inc. www.jjb-software.com
   All rights reserved

   This file is free software; you can redistribute it and/or modify it
   under the terms of the Lesser or Library GNU General Public License 
   as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   Library GNU General Public License for more details.

   You should have received a copy of the Library GNU General Public 
   License along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Maintained by John J. Boyer john.boyer@jjb-software.com
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"

/*Memory allocation */
#define STARTSIZE 0x4000
static int memSize = STARTSIZE;
static int memUsed = 0;
static char *memArea = NULL;
static int
allocMem (void)
{
  char *newArea;
  if (memArea == NULL)
    {
      if (!(memArea = malloc (memSize)))
	return 0;
      return 1;
    }
  memSize *= 2;
  newArea = realloc (memArea, memSize);
  if (!newArea)
    return 0;
  memArea = newArea;
  return 1;
}

/*Main program*/
int
main (int argc, char **argv)
{
  int curarg = 1;
  int interactive = 0;
  char configurationFileName[128];
  char choice = 'c';
  char inputFileName[128];
  char *outputFileName;
  FILE *inputFile = stdin;
  FILE *outputFile = stdout;
  int ch = 0;
  widechar *outbufx;
  int outlenx;
  int k;
  strcpy (configurationFileName, "default.cfg");
  strcpy (inputFileName, "stdin");
  outputFileName = "stdout";

  if (argc > curarg)
    if (argv[curarg][0] == '-')
      {
	if ((argv[curarg][1] | 32) == 'h')
	  {
	    printf
	      ("Usage: devonly [-f configFile] [inputFile] [outputFile]\n");
	    printf
	      ("ConfigFile: configuration file name, default: ~/devonly.cfg.\n");
	    printf ("inputFile : input file, '-' means stdin\n");
	    printf ("outputFile : output file\n");
	    printf ("devonly with no argumenst takes input on stdin\n");
	    printf ("and gives output on stdout.\n");
	    exit (0);
	  }
	else if ((argv[curarg][1] | 32) == 'f' && argc > (curarg + 1))
	  {
	    strcpy (configurationFileName, argv[curarg + 1]);
	    curarg = 3;
	  }
	else if ((argv[curarg][1] | 32) == 'i')
	  {
	    interactive = 1;
	    curarg = 100;
	  }
	else
	  {
	    fprintf (stderr, "Invalid option %s.\n", argv[curarg]);
	    exit (1);
	  }
      }
  if (argc > curarg)
    {
      if (argv[curarg][0] != '-')
	strcpy (inputFileName, argv[curarg]);
      else
	{
	  if ((argv[curarg][1] | 32) == 'i')
	    {
	      interactive = 1;
	      curarg = 100;
	    }
	  else
	    {
	      fprintf (stderr, "Invalid option %s\n", argv[curarg]);
	      exit (1);
	    }
	}
      curarg++;
    }
  if (argc > curarg)
    outputFileName = argv[curarg];

  do
    {
      if (interactive)
	{
	  int namelen = 0;
	  switch (choice)
	    {
	    case 'c':
	      printf ("Enter the name of a configuration file.\n");
	      fgets (configurationFileName, sizeof
		     (configurationFileName), stdin);
	      configurationFileName[strlen (configurationFileName) - 1] = 0;
	      choice = 'i';
	      continue;
	    case 'i':
	      printf
		("Enter the name of an input file, or q to quit or c for a configuration file.\n");
	      fgets (inputFileName, sizeof (inputFileName), stdin);
	      namelen = strlen (inputFileName) - 1;
	      inputFileName[namelen] = 0;
	      if (namelen == 1)
		{
		  if (*inputFileName == 'q')
		    interactive = 0;
		  else if (*inputFileName == 'c')
		    {
		      choice = 'c';
		      continue;
		    }
		}
	      break;
	    }
	  if (!interactive)
	    break;
	}
/*Read file into memory*/
      if (strcmp (inputFileName, "stdin") != 0)
	{
	  if (!(inputFile = fopen (inputFileName, "r")))
	    {
	      fprintf (stderr, "Can't open file %s.\n", inputFileName);
	      if (!interactive)
		exit (1);
	      else
		continue;
	    }
	}
      else
	inputFile = stdin;
      if (outputFileName != "stdout")
	{
	  if (!(outputFile = fopen (outputFileName, "w")))
	    {
	      fprintf (stderr, "Can't open file %s.\n", outputFileName);
	      if (!interactive)
		exit (1);
	      else
		continue;
	    }
	}
      else
	outputFile = stdout;
      memSize = STARTSIZE;
      memArea = NULL;
      memUsed = 0;
      allocMem ();
      while ((ch = fgetc (inputFile)) != EOF)
	{
	  if (ch == 13)
	    continue;
	  memArea[memUsed++] = ch;
	  if ((memUsed + 4) > memSize)
	    allocMem ();
	}
      memArea[memUsed] = 0;
      if (inputFile != stdin)
	fclose (inputFile);
      outbufx = malloc (memSize);
      outlenx = memSize;
      if (lbu_translateString (configurationFileName, memArea, 2000,
(unsigned char *) outbufx,
			       &outlenx, NULL, NULL, 0) < 0)
	printf ("Error 1\n");
      outlenx = memSize;
/*      if (lbu_translateString (configurationFileName, memArea, 
(unsigned char *) outbufx,
			       &outlenx, NULL, NULL, 1) < 0)
	printf ("error 2\n");
*/      for (k = 0; k < outlenx; k++)
	fputc ((char) outbufx[k], outputFile);
      if (outputFile != stdout)
	fclose (outputFile);
      free (memArea);
      free (outbufx);
    }
  while (interactive);
  lbu_free ();
  return 0;
}
