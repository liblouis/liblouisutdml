static int
processBuiltTree (void)
{
  xmlNode *rootElement = xmlDocGetRootElement (ud->doc);
  int haveSemanticFile;
  if (rootElement == NULL)
    {
      lou_logPrint ("Document is empty");
      return 0;
    }
  haveSemanticFile = compile_semantic_table (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  transcribe_document (rootElement);
  return 1;
}

static int
handleChar (int ch, unsigned char *buf int *posx)
{
  pos = *posx;
  if (ch > 127 && ud->input_encoding == ascii8)
    {
      buf[pos++] = 0xc1;
      buf[pos++] = ch;
    }
  else if (ch == '<' || ch == '&')
    {
      buf[pos++] = '&';
      if (ch == '<')
	{
	  buf[pos++] = 'l';
	  buf[pos++] = 't';
	}
      else
	{
	  buf[pos++] = 'a';
	  buf[pos++] = 'm';
	  buf[pos++] = 'p';
	}
      buf[pos++] = ';';
    }
  else
    buf[pos] = ch;
  *posx = pos;
  return 1;
}

int
utd_transcribe_text_string (void)
{
  xmlNode *addPara = makeDaisyDoc ();
  xmlNode *newPara;
  xmlNode *textNode;
  int charsProcessed = 0;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  FormatFor oldFormat = ud->format_for;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  ud->format_for = textDevice;
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      while (charsProcessed < ud->inlen)
	{
	  ch = ud->inbuf[charsProcessed++];
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  pch = ch;
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  handleChar (ch, paragraphBuffer, &charsInParagraph);
	}
      ch = ud->inbuf[charactersProcessed++];
      if (charsInParagraph == 0)
	break;
      paragraphBuffer[charsInParagraph] = 0;
      newPara = xmlNewNode (NULL, (xmlChar *) "p");
      textNode = xmlTextNode (paragraphBuffer);
      xmlAddChild (newPara, textNode);
      xmlAddChild (addPara, newPara);
      if (ch == 10)
	do_blankline ();
      charsInParagraph = 0;
      pch = 0;
      handleChar (ch, paragraphBuffer, &charsInParagraph);
    }
  processBuiltTree ();
  ud->input_encoding = utf8;
  ud->format_for = oldFormat;
  return 1;
}

int
utd_transcribe_text_file (void)
{
  xmlNode *addPara = makeDaisyDoc ();
  xmlNode *newPara;
  xmlNode *textNode;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  FormatFor oldFormat = ud->format_for;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  ud->format_for = textDevice;
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      while ((ch = fgetc (ud->in_file)) != EOF)
	{
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  pch = ch;
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  handleChar (ch, paragraphBuffer, &charsInParagraph);
	}
      if (charsInParagraph == 0)
	break;
      ch = fgetc (ud->inFile);
      paragraphBuffer[charsInParagraph] = 0;
      newPara = xmlNewNode (NULL, (xmlChar *) "p");
      textNode = xmlTextNode (paragraphBuffer);
      xmlAddChild (newPara, textNode);
      xmlAddChild (addPara, newPara);
      if (ch == 10)
	do_blankline ();
      charsInParagraph = 0;
      pch = 0;
      handleChar (ch, paragraphBuffer, &charsInParagraph);
    }
  processBuiltTree ();
  ud->input_encoding = utf8;
  ud->format_for = oldFormat;
  return 1;
}
