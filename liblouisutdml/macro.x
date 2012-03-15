/* Beginning of macro processing */

static int
doSemanticActions (xmlNode * node, int *posx)
{
  int pos = *posx;
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  char *macro = nodeEntry->macro;
  int length = strlen (macro);
  char *paramStart = NULL;
  int retVal = 1;
  int semNum = atoi (macro[pos]);
  for (; isdigit (macro[pos]) && pos < length; pos++);
  if (macro[pos] == '(')
    paramStart = macro[++pos];
  paramLength = find_group_length ("()", paramStart - 1);
  switch (semNum)
    {
    case no:
      if (ud->text_length > 0 && ud->text_length < MAX_LENGTH
	  && ud->text_buffer[ud->text_length - 1] > 32)
	ud->text_buffer[ud->text_length++] = 32;
      break;
    case skip:
      retVal = -1;
    case markhead:
      ud->head_node = node;
      break;
    case configtweak:
{
  int k;
  int kk = 0;
  xmlChar configString[2 * MAXNAMELEN];
  configString[kk++] = ud->string_escape;
  for (k = 0; k < paramLength; k++)
    {
      if (paramStart[k] == '=')
	configString[kk++] = ' ';
      else if (paramStart[k] == ';')
	configString[kk++] = '\n';
      else
	configString[kk++] = (xmlChar) paramStart[k];
    }
  configString[kk++] = '\n';
  configString[kk] = 0;
  if (!config_compileSettings ((char *) configString))
    return 0;
}
      ud->main_braille_table = ud->contracted_table_name;
      if (!lou_getTable (ud->main_braille_table))
	{
	  lou_logPrint ("Cannot open main table %s", ud->main_braille_table);
	  kill_safely ();
	}
      break;
    case htmllink:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 0);
      break;
    case htmltarget:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 1);
      break;
    case boxline:
      do_boxline (node);
      break;
    case blankline:
      do_blankline ();
      break;
    case linespacing:
      do_linespacing (node);
      break;
    case softreturn:
      do_softreturn ();
      break;
    case righthandpage:
      do_righthandpage ();
      break;
    case code:
      transcribe_computerCode (node, 0);
      break;
    case math:
      transcribe_math (node, 0);
      break;
    case graphic:
      transcribe_graphic (node, 0);
      break;
    case chemistry:
      transcribe_chemistry (node, 0);
      break;
    case music:
      transcribe_music (node, 0);
      break;
    case changetable:
      change_table (node);
      break;
    case pagenum:
      do_pagenum ();
      break;
    default:
      retVal = 0;
    }
  *posx = pos;
  return retVal;
}

static int
compileMacro (HashEntry * nodeEntry)
{
  xmlChar *uncompiledMacro = nodeEntry->macro;
  xmlChar compiledMacro[4 * MAXNAMELEN];
  int unPos = 0;
  int pos = 0;
  while (unPos < strlen (uncompiledMacro))
    {
    }
  strcpy (uncompiledMacro, compiledMacro);
  return 1;
}

int
start_macro (xmlNode * node)
{
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  xmlChar *macro;
  int pos = 0;
  int hasStyle = 0;
  if (nodeEntry == NULL || nodeEntry->macro == NULL)
    return 0;
  /*compile macro the first time it is used. */
  if (isalpha (nodeEntry->macro[0]))
    compileMacro (nodeEntry);
  macro = nodeEntry->macro;
  if (macro[0] == '!')
    /* Contains errors */
    return -1;
  while (pos < strlen (macro))
    {
      if (isdigit (macro[pos]))
	doSemanticActions (node, &pos);
      if (mcro[pos] == ',')
	pos++;
      if (macro[pos] == '~')
	{
	  start_style (nodeEntry->style, node);
	  hasStyle = `;
	  pos++;
	}
    }
  return hasStyle;
}

end_macro ()
{
}

/* End of macro processing */
