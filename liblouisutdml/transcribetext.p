int
utd_transcribe_text_string (void)
{
xmlNode *addPara = makeDaisyDoc ();
xmlNode *newPara;
xmlNode *newNode;
  int charsProcessed = 0;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  FormatFor oldFormat = ud->format_for;
  unsigned char paragraphBuffer[BUFSIZE];
  StyleType *docStyle = lookup_style ("document");
  StyleType *paraStyle = lookup_style ("para");
  ud->format_for = textDevice;
  if (!start_document ())
    return 0;
  ud->input_encoding = ud->input_text_encoding;
  start_style (docStyle, NULL);
  while (1)
    {
      while (charsProcessed < ud->inlen)
	{
	  start_style (paraStyle, NULL);
	  ch = ud->inbuf[charsProcessed++];
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  if (charsInParagraph == 0 && ch <= 32)
	    continue;
	  pch = ch;
	  if (ch == 10)
	    ch = ' ';
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  paragraphBuffer[charsInParagraph++] = ch;
	}
      if (charsInParagraph == 0)
	break;
      ch = ud->inbuf[charsProcessed++];
      paragraphBuffer[charsInParagraph] = 0;
      if (!insert_utf8 (paragraphBuffer))
	return 0;
      if (!insert_translation (ud->main_braille_table))
	return 0;
      if (ch == 10)
	do_blankline ();
      end_style ();
      charsInParagraph = 0;
      pch = 0;
      if (ch > 32)
	paragraphBuffer[charsInParagraph++] = ch;
    }
  ud->input_encoding = utf8;
  end_style ();
  end_document ();
  ud->format_for = oldFormat;
  return 1;
}

int
udt_transcribe_text_file (void)
{
xmlNode *addPara = makeDaisyDoc ();
xmlNode *newPara;
xmlNode *newNode;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char paragraphBuffer[BUFSIZE];
  FormatFor oldFormat = ud->format_for;
  widechar outbufx[BUFSIZE];
  int outlenx = MAX_LENGTH;
  StyleType *docStyle = lookup_style ("document");
  StyleType *paraStyle = lookup_style ("para");
  ud->format_for = textDevice;
  if (!start_document ())
    return 0;
  start_style (docStyle, NULL);
  ud->outbuf = outbufx;
  ud->outlen = outlenx;
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      start_style (paraStyle, NULL);
      while ((ch = fgetc (ud->inFile)) != EOF)
	{
	  if (ch == 0 || ch == 13)
	    continue;
	  if (pch == 10 && ch == 10)
	    break;
	  if (charsInParagraph == 0 && ch <= 32)
	    continue;
	  pch = ch;
	  if (ch < 32)
	    ch = ' ';
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  paragraphBuffer[charsInParagraph++] = ch;
	}
      if (charsInParagraph == 0)
	break;
      ch = fgetc (ud->inFile);
      paragraphBuffer[charsInParagraph] = 0;
      if (!insert_utf8 (paragraphBuffer))
	return 0;
      if (!insert_translation (ud->main_braille_table))
	return 0;
      if (ch == 10)
	do_blankline ();
      end_style ();
      charsInParagraph = 0;
      pch = 0;
      if (ch > 32)
	paragraphBuffer[charsInParagraph++] = ch;
    }
  ud->input_encoding = utf8;
  end_style ();
  end_document ();
  ud->format_for = oldFormat;
  return 1;
}
