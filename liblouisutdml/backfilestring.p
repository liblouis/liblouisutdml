static xmlNode *addBlock;

static int
formatBackBlock (void)
{
xmlNode *newBlock;
xmlNode *newBrl;
xmlNode *textNode;
  int k;
  if (ud->text_length <= 0)
    return 1;
newBlock = xmlNewNode (NULL, (xmlChar *) "p");
newBrl = xmlNewNode (NULL, (xmlChar *) "brl");
xmlAddChild (newBrl, newBlock);
  ud->text_length = 0;
backTranslateBlock (newBlock);
xmlAddChild (newBlock, addBlock);
  return 1;
}

int
utd_back_translate_file (void)
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
    ud->output_encoding = utf8;
utd_start ();
addBlock = makeDaisyDoc ();
  while ((ch = fgetc (ud->inFile)) != EOF)
    {
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
formatBackBlock ();
	  leadingBlanks = 0;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->text_length >= MAX_LENGTH)
formatBackBlock ();
      ud->text_buffer[ud->text_length++] = ch;
    }
formatBackBlock ();
utd_finish ();
  return 1;
}

int
utd_back_translate_braille_string (void)
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int k;
    ud->output_encoding = utf8;
utd_start ();
addBlock = makeDaisyDoc ();
for (k = 0; k < ud->inlen; k++)
    {
ch = ud->inbuf[k];
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
formatBackBlock ();
	  leadingBlanks = 0;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->text_length >= MAX_LENGTH)
formatBackBlock ();
      ud->text_buffer[ud->text_length++] = ch;
    }
formatBackBlock ();
utd_finish ();
  return 1;
}
