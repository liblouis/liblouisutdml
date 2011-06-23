static int
formatBackBlock (void)
{
xmlNode *newPara;
xmlNode *textNode;
  int k;
  if (ud->text_length == 0)
    return 1;
  ud->text_buffer[ud->text_length] = 32;
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
  ud->text_length = 0;
  return 1;
}
