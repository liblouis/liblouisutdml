static int
insertEscapeChars (int number)
{
  int k;
  if (number <= 0)
    return 0;
  if ((ud->text_length + number) >= MAX_LENGTH)
    return 0;
  for (k = 0; k < number; k++)
    ud->text_buffer[ud->text_length++] = (widechar) escapeChar;
  return 1;
}

static int
makeParagraph (void)
{
  int translationLength = 0;
  int translatedLength;
  int charactersWritten = 0;
  int pieceStart;
  int k;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <=
	 32 && ud->text_buffer[ud->text_length - 1] != escapeChar)
    ud->text_length--;
  if (ud->text_length == 0)
    return 1;
  ud->text_buffer[ud->text_length] = 0;
  k = 0;
  while (k < ud->text_length)
    {
      if (ud->text_buffer[k] == *litHyphen
	  && ud->text_buffer[k + 1] == 10
	  && ud->text_buffer[k + 2] != escapeChar)
	k += 2;
      if (k > translationLength)
	ud->text_buffer[translationLength] = ud->text_buffer[k];
      k++;
      translationLength++;
    }
  translatedLength = MAX_TRANS_LENGTH;
  if (!lou_backTranslateString (ud->main_braille_table,
				ud->text_buffer, &translationLength,
				&ud->translated_buffer[0],
				&translatedLength,
				(char *) ud->typeform, NULL, 0))
    return 0;
  for (k = 0; k < translatedLength; k++)
    if (ud->translated_buffer[k] == 0)
      ud->translated_buffer[k] = 32;
  while (charactersWritten < translatedLength)
    {
      int lineLength;
      if ((charactersWritten + ud->back_line_length) > translatedLength)
	lineLength = translatedLength - charactersWritten;
      else
	{
	  lineLength = ud->back_line_length;
	  while (lineLength > 0
		 && ud->translated_buffer[charactersWritten +
					  lineLength] != 32)
	    lineLength--;
	  if (lineLength == 0)
	    {
	      lineLength = ud->back_line_length;
	      while ((charactersWritten + lineLength) < translatedLength
		     && ud->translated_buffer[charactersWritten +
					      lineLength] != 32)
		lineLength++;
	    }
	}
      pieceStart = charactersWritten;
	{
	  if (!insertWidechars
	      (&ud->translated_buffer[charactersWritten], lineLength))
	    return 0;
	}
      charactersWritten += lineLength;
      if (ud->translated_buffer[charactersWritten] == 32)
	charactersWritten++;
      if (charactersWritten < translatedLength)
	{
	  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	    return 0;
	}
    }
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  write_outbuf ();
  ud->text_length = 0;
  return 1;
}

static int
handlePrintPageNumber (void)
{
  int k, kk;
  int numberStart = 0;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <= 32)
    ud->text_length--;
  for (k = ud->text_length - 1; k > 0; k--)
    {
      if (ud->text_buffer[k] == 10)
	break;
      if (ud->text_buffer[k] != '-')
	numberStart = k;
    }
  if ((numberStart - k) < 12)
    return 1;
  k++;
  if (ud->back_text == html)
    {
      widechar holdNumber[20];
      int kkk = 0;
      for (kk = numberStart; kk < ud->text_length; kk++)
	holdNumber[kkk++] = ud->text_buffer[kk];
      ud->text_length = k;
      if (!insertEscapeChars (1))
	return 0;
      for (kk = 0; kk < kkk; kk++)
	ud->text_buffer[ud->text_length++] = holdNumber[kk];
      if (!insertEscapeChars (2))
	return 0;
    }
  else
    {
      for (kk = numberStart; kk < ud->text_length; kk++)
	ud->text_buffer[k++] = ud->text_buffer[kk];
      ud->text_length = k;
    }
  return 1;
}

static int
discardPageNumber (void)
{
  int lastBlank = 0;
  int k;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <= 32)
    ud->text_length--;
  for (k = ud->text_length - 1; k > 0 && ud->text_buffer[k] != 10; k--)
    {
      if (!lastBlank && ud->text_buffer[k] == 32)
	lastBlank = k;
      if (lastBlank && ud->text_buffer[k] > 32)
	break;
    }
  if (k > 0 && ud->text_buffer[k] != 10 && (lastBlank - k) > 2)
    ud->text_length = k + 2;
  return 1;
}

int
utd_back_translate_file (void)
{
xmlNode *addPara = makeDaisyDoc ();
xmlNode *newPara;
xmlNode *textNode;
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int printPage = 0;
  int newPage = 0;
  widechar outbufx[BUFSIZE];
  if (!start_document ())
    return 0;
  ud->outbuf = outbufx;
  ud->outlen = MAX_LENGTH;
    ud->output_encoding = utf8;
  while ((ch = fgetc (ud->inFile)) != EOF)
    {
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == escapeChar)
	ch = 32;
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (ch == 10 && printPage)
	{
	  handlePrintPageNumber ();
	  printPage = 0;
	}
      if (ch == 10 && newPage)
	{
	  discardPageNumber ();
	  newPage = 0;
	}
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
	  makeParagraph ();
	  leadingBlanks = 0;
	}
      if (!printPage && ppch == 10 && pch == '-' && ch == '-')
	printPage = 1;
      if (!newPage && pch == 10 && ch == ud->pageEnd[0])
	{
	  discardPageNumber ();
	  newPage = 1;
	  continue;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->text_length >= MAX_LENGTH)
	makeParagraph ();
      ud->text_buffer[ud->text_length++] = ch;
    }
  makeParagraph ();
  return 1;
}

int
back_translate_braille_string (void)
{
xmlNode *addPara = makeDaisyDoc ();
xmlNode *newPara;
xmlNode *newNode;
  return 1;
}
