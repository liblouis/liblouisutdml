/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by the BRLTTY Team

   Copyright (C) 2004, 2005, 2006, 2009
   ViewPlus Technologies, Inc. www.viewplus.com and
   JJB Software, Inc. www.jjb-software.com

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Maintained by John J. Boyer john.boyer@jjb-software.com
   */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"
#include <getopt.h>
#include "progname.h"
#include "version-etc.h"

static const struct option longopts[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"config-file", required_argument, NULL, 'f'},
  {"backward", no_argument, NULL, 'b'},
  {"reformat", no_argument, NULL, 'r'},
  {"poorly-formatted", no_argument, NULL, 'p'},
  {"html", no_argument, NULL, 't'},
  {"text", no_argument, NULL, 'T'},
  {"paragraph-line", no_argument, NULL, 'P'},
  {"log-file", no_argument, NULL, 'l'},
  {"config-setting", required_argument, NULL, 'C'},
  {"writeable-path", required_argument, NULL, 'w'},
  {NULL, 0, NULL, 0}
};

const char version_etc_copyright[] =
  "Copyright %s %d ViewPlus Technologies, Inc. and JJB Software, Inc.";

#define AUTHORS "John J. Boyer"

static void
print_help (void)
{
  printf ("\
Usage: %s [OPTION] [inputFile] [outputFile]\n", program_name);

  fputs ("\
Translate an xml or a text file into an embosser-ready braille file.\n\
This includes translation into grade two, if desired, mathematical \n\
codes, etc. It also includes formatting according to a built-in \n\
style sheet which can be modified by the user.\n\
\n\
If inputFile is not specified or '-' input is taken from stdin. If outputFile\n\
is not specified the output is sent to stdout.\n\n", stdout);

  fputs ("\
  -h, --help          	  display this help and exit\n\
  -v, --version       	  display version information and exit\n\
  -f, --config-file       name a configuration file that specifies\n\
                          how to do the translation\n\
  -b, --backward      	  backward translation\n\
  -r, --reformat      	  reformat a braille file\n\
  -T, --text		  Treat as text even if xml\n\
  -t, --html              html document, not xhtml\n\
  -p, --poorly-formatted  translate a poorly formatted file\n\
  -P, --paragraph-line    treat each block of text ending in a newline\n\
                          as a paragraph. If there are two newline characters\n\
                          a blank line will be inserted before the next paragraph\n\
  -C, --config-setting    specify particular configuration settings\n\
                          They override any settings that are specified in a\n\
                          config file\n\
  -w  --writeable-path    path for temp files and log file\n\
  -l, --log-file          write errors to file2brl.log instead of stderr\n",
  stdout);

  printf ("\n");
  printf ("\
Report bugs to <%s>.\n", PACKAGE_BUGREPORT);
}

int
main (int argc, char **argv)
{
  int mode = 0;
  char *configFileList = NULL;
  char *inputFileName = "stdin";
  char *outputFileName = "stdout";
  char tempFileName[MAXNAMELEN];
  char logFileName[MAXNAMELEN];
  char whichProc = 0;
  char *configSettings = NULL;
  FILE *inputFile = NULL;
  FILE *tempFile;
  int ch = 0;
  int pch = 0;
  int nch = 0;
  int charsRead = 0;
  int k;
  UserData *ud;

  int optc;
  lbu_setLogLevel(LOU_LOG_WARN);
  lou_setLogLevel(LOU_LOG_WARN);
  set_program_name (argv[0]);
  logFileName[0] = 0;
  
  while ((optc =
	  getopt_long (argc, argv, "hvf:brpPtlw:TC:", longopts, NULL)) != 
	  -1)
    switch (optc)
      {
	/* --help and --version exit immediately, per GNU coding standards.  */
      case 'v':
	version_etc (stdout, program_name, PACKAGE_NAME, VERSION, AUTHORS,
		     (char *) NULL);
	exit (EXIT_SUCCESS);
	break;
      case 'h':
	print_help ();
	exit (EXIT_SUCCESS);
	break;
      case 'l':
	strcpy (logFileName, "file2brl.log");
	break;
      case 't':
	mode |= htmlDoc;
	break;
      case 'f':
	configFileList = optarg;
	break;
      case 'b':
      case 'p':
      case 'P':
      case 'r':
      case 'T':
      case '0':
	whichProc = optc;
	break;
      case 'C':
	if (configSettings == NULL)
	  {
	    configSettings = malloc (BUFSIZE);
	    configSettings[0] = 0;
	  }
	strcat (configSettings, optarg);
	strcat (configSettings, "\n");
	break;
       case 'w':
        lbu_setWriteablePath (optarg);
        break;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_name);
	exit (EXIT_FAILURE);
	break;
      }

  if (optind < argc)
    {
      if (optind == argc - 1)
	{
	  inputFileName = argv[optind];
	}
      else if (optind == argc - 2)
	{
	  if (strcmp (argv[optind], "-") != 0)
	    inputFileName = argv[optind];
	  outputFileName = argv[optind + 1];
	}
      else
	{
	  fprintf (stderr, "%s: extra operand: %s\n",
		   program_name, argv[optind + 2]);
	  fprintf (stderr, "Try `%s --help' for more information.\n",
		   program_name);
	  exit (EXIT_FAILURE);
	}
    }

  if (whichProc == 0)
    whichProc = '0';
  if (logFileName[0] != 0)
  {
  strcpy (logFileName, lbu_getWriteablePath());
  strcat (logFileName, "file2brl.log");
  }
  if (configSettings != NULL)
    for (k = 0; configSettings[k]; k++)
      if (configSettings[k] == '=' && configSettings[k - 1] != ' ')
	configSettings[k] = ' ';
  if (configFileList == NULL)
    configFileList = "preferences.cfg";
  if ((ud = lbu_initialize (configFileList, logFileName, 
  configSettings)) == NULL)
    exit (EXIT_FAILURE);
  if (strcmp (inputFileName, "stdin") != 0)
    {
      if (!(inputFile = fopen (inputFileName, "r")))
	{
	  logMessage (LOU_LOG_FATAL, "Can't open input file %s.\n", inputFileName);
	  exit (EXIT_FAILURE);
	}
    }
  else
    inputFile = stdin;
  /*Create somewhat edited temporary file to facilitate use of stdin. */
  strcpy (tempFileName, lbu_getWriteablePath ());
  strcat (tempFileName, "file2brl.temp");
  if (!(tempFile = fopen (tempFileName, "w")))
    {
      logMessage (LOU_LOG_FATAL, "Can't open temporary file.\n");
      exit (EXIT_FAILURE);
    }
  if (whichProc == 'p')
    {
      int ppch = 0;
      int firstCh = 0;
      int skipit = 0;
      while ((ch = fgetc (inputFile)) != EOF)
	{
	  if (firstCh == 0)
	    firstCh = ch;
	  if (ch == 12 || ch == 13)
	    continue;
	  if (ch == '<' && firstCh == '<')
	    {
	      skipit = 1;
	      continue;
	    }
	  if (skipit)
	    {
	      if (ch == '>')
		skipit = 0;
	      continue;
	    }
	  if (ch == '-')
	    {
	      nch = fgetc (inputFile);
	      if (nch == 10)
		continue;
	      ungetc (nch, inputFile);
	    }
	  if (!((pch == 10 && ch == 10) || (ppch == 10 && pch == 10)))
	    {
	      if (ch <= 32 && pch <= 32)
		continue;
	      if (!
		  (pch == 10 && ((ppch >= 97 && ppch <= 122) || ppch == ',')))
		{
		  if (pch == 10 && ch < 97)
		    fputc (10, tempFile);
		}
	    }
	  ppch = pch;
	  pch = ch;
	  fputc (ch, tempFile);
	  charsRead++;
	}
    }
  else
    while ((ch = fgetc (inputFile)) != EOF)
      {
	if (charsRead == 0 && ch <= ' ')
	  continue;
	if (ch == 13)
	  continue;
	if (charsRead == 0)
	  {
	    if (ch != '<' && whichProc == '0')
	      whichProc = 'T';
	    nch = fgetc (inputFile);
	    if (!(mode & htmlDoc) && whichProc == '0' && nch != '?')
	      fprintf (tempFile, "%s\n", ud->xml_header);
	  }
	if (pch == '>' && ch == '<')
	  fputc (10, tempFile);
	if (whichProc == 'P' && ch == 10 && pch != 10)
	  fputc (10, tempFile);
	fputc (ch, tempFile);
	pch = ch;
	charsRead++;
	if (charsRead == 1)
	  {
	    fputc (nch, tempFile);
	    charsRead++;
	  }
      }
  fclose (tempFile);
  if (inputFile != stdin)
    fclose (inputFile);
  if (charsRead > 2)
    switch (whichProc)
      {
      case 'b':
	if (!lbu_backTranslateFile (configFileList, tempFileName,
				    outputFileName, logFileName, configSettings,
				    mode))
	  exit (EXIT_FAILURE);
	break;
      case 'r':
	{
	  char temp2FileName[MAXNAMELEN];
	  strcpy (temp2FileName, lbu_getWriteablePath ());
	  strcat (temp2FileName, "file2brl2.temp");
	  if (!lbu_backTranslateFile (configFileList, tempFileName,
				      temp2FileName, logFileName,
				      configSettings, mode))
	    exit (EXIT_FAILURE);
	  if (ud->back_text == html)
	    if (!lbu_translateFile (configFileList, temp2FileName,
				    outputFileName, logFileName, configSettings,
				    mode))
	      exit (EXIT_FAILURE);
	  else
	    if (!lbu_translateTextFile (configFileList, temp2FileName,
					outputFileName, logFileName,
					configSettings, mode))
	      exit (EXIT_FAILURE);
	}
	break;
      case 'p':
	if (!lbu_translateTextFile (configFileList, tempFileName,
				    outputFileName, logFileName, configSettings,
				    mode))
	  exit (EXIT_FAILURE);
	break;
      case 'T':
	if (!lbu_translateTextFile (configFileList, tempFileName,
				    outputFileName,
				    logFileName, configSettings, mode))
	  exit (EXIT_FAILURE);
	break;
      case 't':
      case '0':
	if (!lbu_translateFile (configFileList, tempFileName, outputFileName,
				logFileName, configSettings, mode))
	  exit (EXIT_FAILURE);
	break;
      default:
	logMessage (LOU_LOG_FATAL, "Program bug %c\n", whichProc);
	break;
      }
  lbu_free ();
  if (configSettings != NULL)
    free (configSettings);
  return 0;
}
