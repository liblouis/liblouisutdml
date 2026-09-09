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

   Maintained by John J. Boyer john.boyer@abilitiessoft.org
   */

#include <stdio.h>
#include <string.h>
#include "louisutdml.h"

static int mathTrans ();
static void mathCreateBrlNode ();
static void mathText (xmlNode * node, int action);
static void russianMultiscripts (xmlNode * node);
static void russianIntegerStructure (xmlNode * node);
static void russianRecognizedBracket (xmlNode * node);
static void russianFraction (xmlNode * node);
static void russianMixedNumber (xmlNode * node);
static void russianPeriodicDecimal (xmlNode * node);
static void russianFunction (xmlNode * node);
static void russianFunctionStructure (xmlNode * node);
static void russianFunctionArgument (xmlNode * node);
static void russianGeometryPrefix (xmlNode * node);
static void russianGeometryRelation (xmlNode * node);
static void russianSetOperator (xmlNode * node);
static void russianMark (xmlNode * node);
static void russianContinuedNumber (xmlNode * node);
static void russianFenced (xmlNode * node);
static void russianMathStack (xmlNode * node);
static void russianLongDivision (xmlNode * node);
static xmlNode *firstMathChild (xmlNode * node);
static xmlNode *curLink;

int
transcribe_math (xmlNode * node, int action)
{
  StyleType *style;
  xmlNode *child;
  int branchCount = 0;
  logMessage(LOU_LOG_DEBUG, "Begin transcribe_math");
  if (node == NULL)
    return 0;
  if (action == 0)
    {
      logMessage(LOU_LOG_DEBUG, "Math node action==0");
      insert_translation (ud->main_braille_table);
      curLink = node;
      if (ud->format_for == utd)
        {
          mathCreateBrlNode ();
        }
    }
  else
    {
      logMessage(LOU_LOG_DEBUG, "Math node action!=0");
      push_sem_stack (node);
    }
  switch (ud->stack[ud->top])
    {
    case skip:
      logMessage(LOU_LOG_DEBUG, "Math node skip");
      pop_sem_stack ();
      return 1;
    case reverse:
      logMessage(LOU_LOG_DEBUG, "Math node reverse");
      do_reverse (node);
      break;
    case russianmmultiscripts:
      russianMultiscripts (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianintegerstructure:
      russianIntegerStructure (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianrecognizedbracket:
      russianRecognizedBracket (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianfraction:
      russianFraction (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianmixednumber:
      russianMixedNumber (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianperiodicdecimal:
      russianPeriodicDecimal (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianfunction:
      russianFunction (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianfunctionstructure:
      russianFunctionStructure (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianfunctionargument:
      russianFunctionArgument (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russiangeometryprefix:
      russianGeometryPrefix (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russiangeometryrelation:
      russianGeometryRelation (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russiansetoperator:
      russianSetOperator (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianmark:
      russianMark (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russiancontinuednumber:
      russianContinuedNumber (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianfenced:
      russianFenced (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianmathstack:
      russianMathStack (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    case russianlongdivision:
      russianLongDivision (node);
      pop_sem_stack ();
      if (action == 0)
	mathTrans ();
      return 1;
    default:
      break;
    }
  if ((style = is_style (node)) != NULL)
    {
      logMessage(LOU_LOG_DEBUG, "Math node start style");
      mathTrans ();
      start_style (style, node);
    }
  child = node->children;
  while (child)
    {
      insert_code (node, branchCount);
      branchCount++;
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  transcribe_math (child, 1);
	  break;
	case XML_TEXT_NODE:
	  mathText (child, 1);
	  break;
	case XML_CDATA_SECTION_NODE:
	  transcribe_cdataSection (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  if (style)
    {
      logMessage(LOU_LOG_DEBUG, "Math node end style");
      mathTrans ();
      end_style ();
    }
  pop_sem_stack ();
  if (action == 0)
    mathTrans ();
  logMessage(LOU_LOG_DEBUG, "Finish transcribe_math");
  return 1;
}

static int
translateMathChunk ()
{
  int translationLength;
  int translatedLength;
  int k;
  if (ud->text_length == 0)
    return 1;
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  if (ud->format_for == utd)
    {
      ud->text_buffer[ud->text_length++] = LOU_ENDSEGMENT;
      translationLength++;
      k = lou_translate (ud->mathexpr_table_name,
			 ud->text_buffer,
			 &translationLength,
			 &ud->
			 translated_buffer[ud->translated_length],
			 &translatedLength,
			 ud->typeform, NULL, NULL,
			 NULL, NULL, dotsIO);
      ud->in_sync = 0;
      memset (ud->typeform, 0, sizeof (ud->typeform));
      ud->text_length = 0;
      if (!k)
	{
	  logMessage (LOU_LOG_ERROR, "Could not open table %s", ud->mathexpr_table_name);
	  ud->mathexpr_table_name = NULL;
	  return 0;
	}
      if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
	  ud->translated_length += translatedLength;
      else
	ud->translated_length = MAX_TRANS_LENGTH;
    }
  else
    insert_translation (ud->mathexpr_table_name);
  return 1;
}

static void
appendTranslatedMathMarker (widechar marker)
{
  if (ud->translated_length + 2 >= MAX_TRANS_LENGTH)
    return;
  if (ud->format_for == utd)
    {
      ud->translated_buffer[ud->translated_length++] =
        LOU_DOTS | LOU_DOT_11 | LOU_DOT_1;
      ud->translated_buffer[ud->translated_length++] = marker == 'c' ?
        LOU_DOTS | LOU_DOT_1 | LOU_DOT_4 :
        LOU_DOTS | LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5;
    }
  else
    {
      ud->translated_buffer[ud->translated_length++] = 0x1b;
      ud->translated_buffer[ud->translated_length++] = marker;
    }
}

/* Column-layout markers are formatter instructions, not mathematical text.
 * Keep them outside Liblouis translation so alphabet signs and display-table
 * encodings cannot change the ESC+c / ESC+r contract. */
static int
mathTrans ()
{
  widechar *source;
  int sourceLength;
  int start;
  int k;

  if (ud->text_length == 0)
    return 1;
  ud->needs_editing = 1;
  sourceLength = ud->text_length;
  source = malloc (sourceLength * sizeof (widechar));
  if (source == NULL)
    return translateMathChunk ();
  memcpy (source, ud->text_buffer, sourceLength * sizeof (widechar));

  start = 0;
  for (k = 0; k + 1 < sourceLength; k++)
    if (source[k] == 0x1b && (source[k + 1] == 'c' || source[k + 1] == 'r'))
      {
        int before = ud->translated_length;
        ud->text_length = k - start;
        if (ud->text_length > 0)
          memcpy (ud->text_buffer, &source[start],
                  ud->text_length * sizeof (widechar));
        translateMathChunk ();
        if (start > 0 && ud->translated_length > before &&
            ud->translated_buffer[before] == ' ')
          {
            memmove (&ud->translated_buffer[before],
                     &ud->translated_buffer[before + 1],
                     (ud->translated_length - before - 1) * sizeof (widechar));
            ud->translated_length--;
          }
        appendTranslatedMathMarker (source[k + 1]);
        start = k + 2;
        k++;
      }
  ud->text_length = sourceLength - start;
  if (ud->text_length > 0)
    memcpy (ud->text_buffer, &source[start],
            ud->text_length * sizeof (widechar));
  {
    int before = ud->translated_length;
    translateMathChunk ();
    if (start > 0 && ud->translated_length > before &&
        ud->translated_buffer[before] == ' ')
      {
        memmove (&ud->translated_buffer[before],
                 &ud->translated_buffer[before + 1],
                 (ud->translated_length - before - 1) * sizeof (widechar));
        ud->translated_length--;
      }
  }
  free (source);
  return 1;
}

static void
mathText (xmlNode * node, int action)
{
    insert_utf8 (node->content);
}

static int
isMathElement (xmlNode * node, const char *name)
{
  return node != NULL && node->type == XML_ELEMENT_NODE &&
    xmlStrEqual (node->name, (const xmlChar *) name);
}

static void
insertMathCharacter (widechar character)
{
  if (ud->text_length < MAX_LENGTH)
    ud->text_buffer[ud->text_length++] = character;
}

static xmlNode *
nextMathElement (xmlNode * node)
{
  while (node != NULL && node->type != XML_ELEMENT_NODE)
    node = node->next;
  return node;
}

static xmlNode *
previousMathElement (xmlNode * node)
{
  while (node != NULL && node->type != XML_ELEMENT_NODE)
    node = node->prev;
  return node;
}

static int
isIntegerMathToken (xmlNode * node)
{
  xmlChar *content;
  int isInteger = 0;
  int k;

  if (!isMathElement (node, "mn"))
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL || content[0] == 0)
    goto cleanup;
  isInteger = 1;
  for (k = 0; content[k] != 0; k++)
    if (content[k] < '0' || content[k] > '9')
      {
	isInteger = 0;
	break;
      }

cleanup:
  if (content != NULL)
    xmlFree (content);
  return isInteger;
}

static int
isInfinityMathToken (xmlNode * node)
{
  xmlChar *content;
  int result;

  if (!isMathElement (node, "mo") && !isMathElement (node, "mi"))
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL)
    return 0;
  result = xmlStrEqual (content, (const xmlChar *) "\xe2\x88\x9e");
  xmlFree (content);
  return result;
}

static void
insertLoweredInteger (xmlNode * node)
{
  static const widechar loweredDigits[] = {
    0x2080, 0x2081, 0x2082, 0x2083, 0x2084,
    0x2085, 0x2086, 0x2087, 0x2088, 0x2089
  };
  xmlChar *content = xmlNodeGetContent (node);
  int k;

  if (content == NULL)
    return;
  for (k = 0; content[k] != 0; k++)
    if (content[k] >= '0' && content[k] <= '9')
      insertMathCharacter (loweredDigits[content[k] - '0']);
  xmlFree (content);
}

static void
insertUnnumberedInteger (xmlNode * node)
{
  xmlChar *content = xmlNodeGetContent (node);
  int k;

  if (content == NULL)
    return;
  for (k = 0; content[k] != 0; k++)
    if (content[k] >= '0' && content[k] <= '9')
      {
	/* ru-math.uti uses the shared F401 semantic prefix to suppress the
	 * number sign. Emit the printable digit after the prefix so the stream
	 * matches the current Liblouis structured-math contract. */
	insertMathCharacter (0xf401);
	insertMathCharacter (content[k]);
      }
  xmlFree (content);
}

static int
isSimpleFractionPart (xmlNode * node)
{
  xmlChar *content;
  int simple = 0;
  int k;

  if (isMathElement (node, "mi"))
    return 1;
  if (!isMathElement (node, "mn"))
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL || content[0] == 0)
    goto cleanup;
  simple = 1;
  for (k = 0; content[k] != 0; k++)
    if ((content[k] < '0' || content[k] > '9') && content[k] != ',')
      {
	simple = 0;
	break;
      }
cleanup:
  if (content != NULL)
    xmlFree (content);
  return simple;
}

static int
hasFractionAncestor (xmlNode * node)
{
  xmlNode *ancestor;

  for (ancestor = node == NULL ? NULL : node->parent; ancestor != NULL;
       ancestor = ancestor->parent)
    if (isMathElement (ancestor, "mfrac"))
      return 1;
  return 0;
}

static int
isSignedSimpleNumber (xmlNode *node, xmlNode **sign, xmlNode **number)
{
  xmlNode *first;
  xmlNode *second;
  xmlChar *content;
  int result = 0;

  if (!isMathElement (node, "mrow"))
    return 0;
  first = nextMathElement (node->children);
  second = first == NULL ? NULL : nextMathElement (first->next);
  if (first == NULL || second == NULL || nextMathElement (second->next) != NULL ||
      !isMathElement (first, "mo") || !isSimpleFractionPart (second))
    return 0;
  content = xmlNodeGetContent (first);
  if (content != NULL &&
      (xmlStrEqual (content, (const xmlChar *) "+") ||
       xmlStrEqual (content, (const xmlChar *) "-") ||
       xmlStrEqual (content, (const xmlChar *) "\xe2\x88\x92")))
    result = 1;
  if (content != NULL)
    xmlFree (content);
  if (result)
    {
      *sign = first;
      *number = second;
    }
  return result;
}

/* Manual (2010), section 2.5.  Numeric and elementary algebraic fractions
 * use the compact form.  Nested and compound fractions are explicitly
 * delimited, which also makes recursion unambiguous. */
static void
russianFraction (xmlNode * node)
{
  xmlNode *numerator = nextMathElement (node->children);
  xmlNode *denominator = numerator == NULL ? NULL :
    nextMathElement (numerator->next);
  int nested = hasFractionAncestor (node);
  xmlNode *sign = NULL;
  xmlNode *signedNumber = NULL;

  if (numerator == NULL || denominator == NULL)
    return;
  if (!nested && isIntegerMathToken (numerator) &&
      isIntegerMathToken (denominator))
    {
      transcribe_math (numerator, 1);
      insertLoweredInteger (denominator);
      return;
    }
  if (!nested && isSimpleFractionPart (numerator) &&
      isSimpleFractionPart (denominator))
    {
      transcribe_math (numerator, 1);
      insertMathCharacter (0xf503);
      if (isIntegerMathToken (denominator))
	insertLoweredInteger (denominator);
      else
	transcribe_math (denominator, 1);
      return;
    }
  if (!nested && isSimpleFractionPart (numerator) &&
      isSignedSimpleNumber (denominator, &sign, &signedNumber))
    {
      xmlChar *signContent = xmlNodeGetContent (sign);
      transcribe_math (numerator, 1);
      insertMathCharacter (0xf503);
      insertMathCharacter (signContent != NULL &&
	  xmlStrEqual (signContent, (const xmlChar *) "+") ? 0xf5c2 : 0xf5c1);
      if (isIntegerMathToken (signedNumber))
	insertLoweredInteger (signedNumber);
      else
	transcribe_math (signedNumber, 1);
      if (signContent != NULL)
	xmlFree (signContent);
      return;
    }
  if (!nested && isMathElement (node->parent, "mrow"))
    {
      xmlNode *previous = previousMathElement (node->prev);
      if (previous != NULL && isMathElement (previous, "mo"))
	previous = previousMathElement (previous->prev);
      if (isIntegerMathToken (previous))
	insertMathCharacter (0xf534);
    }
  insertMathCharacter (0xf500);
  transcribe_math (numerator, 1);
  insertMathCharacter (' ');
  insertMathCharacter (0xf503);
  transcribe_math (denominator, 1);
  insertMathCharacter (0xf501);
}

static void
russianMixedNumber (xmlNode * node)
{
  xmlNode *whole = nextMathElement (node->children);
  xmlNode *fraction = whole == NULL ? NULL : nextMathElement (whole->next);
  xmlNode *numerator;
  xmlNode *denominator;

  if (fraction != NULL && isMathElement (fraction, "mo"))
    fraction = nextMathElement (fraction->next);
  if (whole == NULL || fraction == NULL || !isMathElement (fraction, "mfrac"))
    return;
  numerator = nextMathElement (fraction->children);
  denominator = numerator == NULL ? NULL : nextMathElement (numerator->next);
  transcribe_math (whole, 1);
  if (isIntegerMathToken (numerator) && isIntegerMathToken (denominator))
    {
      insertMathCharacter (0xf504);
      insertUnnumberedInteger (numerator);
      insertLoweredInteger (denominator);
    }
  else
    {
      russianFraction (fraction);
    }
}

static void
insertPeriodicDigits (const xmlChar *content)
{
  int k;
  for (k = 0; content != NULL && content[k] != 0; k++)
    if (content[k] >= '0' && content[k] <= '9')
      insertMathCharacter (0xf560 + content[k] - '0');
}

static void
insertPeriodicPrefix (const xmlChar *content, int length)
{
  int comma = -1;
  int k;

  /* Current ru-math.uti defines F504 as an empty structural boundary. Feed
   * the integer prefix to the ordinary numeric rules so they supply the
   * single number sign. F55A keeps a trailing decimal comma numeric even when
   * the periodic digits live in a separate MathML token. */
  for (k = 0; k < length; k++)
    if (content[k] == ',')
      {
	comma = k;
	break;
      }
  for (k = 0; k < (comma < 0 ? length : comma); k++)
    insertMathCharacter (content[k]);
  if (comma >= 0)
    {
      insertMathCharacter (0xf55a);
      for (k = comma + 1; k < length; k++)
	if (content[k] >= '0' && content[k] <= '9')
	  insertMathCharacter (0xf560 + content[k] - '0');
    }
}

static void
russianPeriodicDecimal (xmlNode * node)
{
  xmlChar *content;
  xmlChar *open;
  xmlChar *close;
  xmlNode *decimal;
  xmlNode *period;

  if (isMathElement (node, "mn"))
    {
      content = xmlNodeGetContent (node);
      if (content == NULL)
	return;
      open = (xmlChar *) xmlStrchr (content, '(');
      close = open == NULL ? NULL : (xmlChar *) xmlStrchr (open + 1, ')');
      if (open != NULL && close != NULL)
	{
	  insertPeriodicPrefix (content, open - content);
	  insertMathCharacter (0xf55b);
	  *close = 0;
	  insertPeriodicDigits (open + 1);
	  insertMathCharacter (0xf55c);
	}
      xmlFree (content);
      return;
    }
  decimal = nextMathElement (node->children);
  period = decimal == NULL ? NULL : nextMathElement (decimal->next);
  if (period != NULL && isMathElement (period, "mo"))
    period = nextMathElement (period->next);
  if (decimal == NULL || period == NULL)
    return;
  content = xmlNodeGetContent (decimal);
  if (content == NULL)
    return;
  insertPeriodicPrefix (content, xmlStrlen (content));
  xmlFree (content);
  insertMathCharacter (0xf55b);
  content = xmlNodeGetContent (period);
  insertPeriodicDigits (content);
  if (content != NULL)
    xmlFree (content);
  insertMathCharacter (0xf55c);
}

static void
russianFunction (xmlNode * node)
{
  static const char *names[] = {
    "sin", "cos", "tg", "ctg", "arcsin", "arccos", "arctg", "arcctg",
    "log", "lg", "ln", "max", "min", "exp", "lim", NULL
  };
  xmlChar *content = xmlNodeGetContent (node);
  int k;

  if (content == NULL)
    return;
  for (k = 0; names[k] != NULL; k++)
    if (xmlStrEqual (content, (const xmlChar *) names[k]))
      {
	insertMathCharacter (0xf510 + k);
	break;
      }
  if (nextMathElement (node->next) != NULL &&
      isMathElement (nextMathElement (node->next), "mi"))
    insertMathCharacter (0xf51f);
  xmlFree (content);
}

static int
isRussianFunctionNode (xmlNode *node)
{
  static const char *names[] = {
    "sin", "cos", "tg", "ctg", "arcsin", "arccos", "arctg", "arcctg",
    "log", "lg", "ln", "max", "min", "exp", "lim", NULL
  };
  xmlChar *content;
  int result = 0;
  int k;

  if (!isMathElement (node, "mi") && !isMathElement (node, "mtext") &&
      !isMathElement (node, "mo"))
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL)
    return 0;
  for (k = 0; names[k] != NULL; k++)
    if (xmlStrEqual (content, (const xmlChar *) names[k]))
      {
	result = 1;
	break;
      }
  xmlFree (content);
  return result;
}

static void
insertAlphabetRestartForFollowingArgument (xmlNode *node)
{
  xmlNode *argument = nextMathElement (node->next);
  xmlChar *content;

  if (argument == NULL || !isMathElement (argument, "mi"))
    return;
  content = xmlNodeGetContent (argument);
  if (content != NULL && content[0] < 0x80 &&
      ((content[0] >= 'A' && content[0] <= 'Z') ||
       (content[0] >= 'a' && content[0] <= 'z')))
    insertMathCharacter (0xf51f);
  if (content != NULL)
    xmlFree (content);
}

static int
insertExplicitLatinToken (xmlNode *node)
{
  static const unsigned char letterDots[] = {
    0x01, 0x03, 0x09, 0x19, 0x11, 0x0b, 0x1b, 0x13, 0x0a,
    0x1a, 0x05, 0x07, 0x0d, 0x1d, 0x15, 0x0f, 0x1f, 0x17,
    0x0e, 0x1e, 0x25, 0x27, 0x3a, 0x2d, 0x3d, 0x35
  };
  xmlChar *content = xmlNodeGetContent (node);
  int k;
  int upper;

  if (content == NULL || content[0] == 0)
    goto failure;
  for (k = 0; content[k] != 0; k++)
    if (!((content[k] >= 'A' && content[k] <= 'Z') ||
	  (content[k] >= 'a' && content[k] <= 'z')))
      goto failure;
  upper = content[0] >= 'A' && content[0] <= 'Z';
  insertMathCharacter (upper ? 0x2828 : 0x2820);
  for (k = 0; content[k] != 0; k++)
    {
      int letter = content[k] >= 'A' && content[k] <= 'Z' ?
	content[k] - 'A' : content[k] - 'a';
      insertMathCharacter (0x2800 + letterDots[letter]);
    }
  xmlFree (content);
  return 1;

failure:
  if (content != NULL)
    xmlFree (content);
  return 0;
}

static void
russianFunctionArgument (xmlNode * node)
{
  insertExplicitLatinToken (node);
}

static void
russianFunctionStructure (xmlNode * node)
{
  xmlNode *function = nextMathElement (node->children);
  xmlNode *script = function == NULL ? NULL : nextMathElement (function->next);
  widechar indicator = isMathElement (node, "msub") ? 0xf320 : 0xf321;

  if (!isRussianFunctionNode (function) || script == NULL)
    return;
  russianFunction (function);
  insertMathCharacter (indicator);
  if (isIntegerMathToken (script))
    insertLoweredInteger (script);
  else
    {
      transcribe_math (script, 1);
      insertMathCharacter (0xf402);
      insertMathCharacter (indicator);
    }
  insertAlphabetRestartForFollowingArgument (node);
}

static void
russianGeometryPrefix (xmlNode * node)
{
  xmlChar *content = xmlNodeGetContent (node);

  if (content == NULL)
    return;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x88\xa0"))
    insertMathCharacter (0xf558);
  else if (xmlStrEqual (content, (const xmlChar *) "\xe2\x96\xb3"))
    insertMathCharacter (0xf559);
  else if (xmlStrEqual (content, (const xmlChar *) "\xe2\x8c\x92"))
    insertMathCharacter (0xf557);
  insertMathCharacter (0xf51f);
  xmlFree (content);
}

static void
russianGeometryRelation (xmlNode * node)
{
  xmlChar *content = xmlNodeGetContent (node);

  if (content == NULL)
    return;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x88\xa5"))
    insertMathCharacter (0xf5ad);
  else if (xmlStrEqual (content, (const xmlChar *) "\xe2\x8a\xa5") ||
           xmlStrEqual (content, (const xmlChar *) "\xe2\x9f\x82"))
    insertMathCharacter (0xf5ae);
  else if (xmlStrEqual (content, (const xmlChar *) "\xe2\x88\xbc") ||
           xmlStrEqual (content, (const xmlChar *) "~"))
    {
      insertMathCharacter (' ');
      insertMathCharacter (0xf5af);
    }
  xmlFree (content);
}

static void
russianSetOperator (xmlNode * node)
{
  static const char *symbols[] = {
    "\xe2\x88\x88", "\xe2\x88\x89", "\xe2\x8a\x82", "\xe2\x8a\x84",
    "\xe2\x88\xa9", "\xe2\x88\xaa", "\xe2\x88\x96", "\xe2\x88\x85",
    "\xe2\x87\x92", "\xe2\x87\x90", "\xe2\x87\x94", "\xe2\x88\x80",
    "\xe2\x88\x83", NULL
  };
  xmlChar *content = xmlNodeGetContent (node);
  int k;

  if (content == NULL)
    return;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x81\xa2"))
    {
      insertMathCharacter (0xf5c0);
      xmlFree (content);
      return;
    }
  for (k = 0; symbols[k] != NULL; k++)
    if (xmlStrEqual (content, (const xmlChar *) symbols[k]))
      {
	if (k == 0 || k == 2 || (k >= 4 && k <= 6) || (k >= 8 && k <= 10))
	  insertMathCharacter (' ');
	insertMathCharacter (0xf5a0 + k);
	if (k == 0 || k == 2)
	  insertMathCharacter (' ');
	break;
      }
  xmlFree (content);
}

static widechar
russianMarkCharacter (const xmlChar *content)
{
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x80\xbe") ||
      xmlStrEqual (content, (const xmlChar *) "\xc2\xaf") ||
      xmlStrEqual (content, (const xmlChar *) "_") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x80\x95")) return 0xf540;
  if (xmlStrEqual (content, (const xmlChar *) "'") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x80\xb2")) return 0xf541;
  if (xmlStrEqual (content, (const xmlChar *) "*") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x88\x97")) return 0xf542;
  if (xmlStrEqual (content, (const xmlChar *) "\xc3\x97")) return 0xf543;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x97\x8b")) return 0xf544;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x96\xa1")) return 0xf545;
  if (xmlStrEqual (content, (const xmlChar *) "~")) return 0xf546;
  if (xmlStrEqual (content, (const xmlChar *) "^")) return 0xf547;
  if (xmlStrEqual (content, (const xmlChar *) "\xcb\x87")) return 0xf548;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x86\x92") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x9f\xb6")) return 0xf54b;
  if (xmlStrEqual (content, (const xmlChar *) "\xe2\x86\x90") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x9f\xb5")) return 0xf54c;
  if (xmlStrEqual (content, (const xmlChar *) "+")) return 0xf54d;
  if (xmlStrEqual (content, (const xmlChar *) "-") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x88\x92")) return 0xf54e;
  return 0;
}

static int
isCompoundMarkBase (xmlNode *node)
{
  return isMathElement (node, "mrow") || isMathElement (node, "mfrac") ||
    isMathElement (node, "msqrt") || isMathElement (node, "mroot") ||
    isMathElement (node, "mtable");
}

/* Section 2.7 distinguishes exact over/under marks from right-hand marks.
 * Vector arrows and bars have compact one- and multi-letter forms. */
static void
russianMark (xmlNode * node)
{
  xmlNode *base = nextMathElement (node->children);
  xmlNode *mark = base == NULL ? NULL : nextMathElement (base->next);
  xmlChar *content;
  xmlChar *baseContent;
  widechar markCharacter;
  int baseLength;
  int repeatCount = 1;
  int exactAbove = isMathElement (node, "mover");
  int exactBelow = isMathElement (node, "munder");
  int rightAbove = isMathElement (node, "msup");

  if (base == NULL || mark == NULL)
    return;
  content = xmlNodeGetContent (mark);
  baseContent = xmlNodeGetContent (base);
  if (content == NULL)
    goto cleanup;
  baseLength = baseContent == NULL ? 0 : xmlUTF8Strlen (baseContent);

  if (exactAbove && (xmlStrEqual (content, (const xmlChar *) "\xe2\x86\x92") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x9f\xb6")))
    {
      if (!insertExplicitLatinToken (base))
	transcribe_math (base, 1);
      insertMathCharacter (baseLength > 1 ? 0xf551 : 0xf550);
      goto cleanup;
    }
  if (exactAbove && (xmlStrEqual (content, (const xmlChar *) "\xc2\xaf") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x80\xbe") ||
      xmlStrEqual (content, (const xmlChar *) "_")))
    {
      if (!insertExplicitLatinToken (base))
	transcribe_math (base, 1);
      insertMathCharacter (baseLength > 1 ? 0xf553 : 0xf552);
      goto cleanup;
    }
  if (exactAbove && (xmlStrEqual (content, (const xmlChar *) "\xe2\x8c\x92") ||
      xmlStrEqual (content, (const xmlChar *) "\xe2\x8f\x9c")))
    {
      insertMathCharacter (0xf557);
      if (!insertExplicitLatinToken (base))
	transcribe_math (base, 1);
      goto cleanup;
    }

  markCharacter = russianMarkCharacter (content);
  if (markCharacter == 0 && content[0] != 0 && content[0] < 0x80 &&
      content[1] == content[0] &&
      (content[2] == 0 || (content[2] == content[0] && content[3] == 0)))
    {
      xmlChar oneMark[2];
      oneMark[0] = content[0];
      oneMark[1] = 0;
      markCharacter = russianMarkCharacter (oneMark);
      repeatCount = content[2] == 0 ? 2 : 3;
    }
  if (markCharacter == 0)
    goto cleanup;
  if (isCompoundMarkBase (base))
    insertMathCharacter (0xf570);
  transcribe_math (base, 1);
  if (isCompoundMarkBase (base))
    insertMathCharacter (0xf571);

  if (exactBelow)
    insertMathCharacter (0xf533);
  else if (exactAbove && markCharacter != 0xf540 && markCharacter != 0xf546 &&
           markCharacter != 0xf547 && markCharacter != 0xf548 &&
           markCharacter != 0xf54b && markCharacter != 0xf54c)
    insertMathCharacter (0xf532);
  else if (rightAbove && markCharacter != 0xf541 && markCharacter != 0xf542 &&
           markCharacter != 0xf543 && markCharacter != 0xf54d &&
           markCharacter != 0xf54e)
    insertMathCharacter (0xf530);
  else if (!exactAbove && !exactBelow && !rightAbove)
    insertMathCharacter (0xf531);
  while (repeatCount-- > 0)
    insertMathCharacter (markCharacter);

cleanup:
  if (content != NULL)
    xmlFree (content);
  if (baseContent != NULL)
    xmlFree (baseContent);
}

static void
russianContinuedNumber (xmlNode * node)
{
  insertUnnumberedInteger (node);
}

static void
insertMathTableMarker (widechar marker)
{
  insertMathCharacter (0x1b);
  insertMathCharacter (marker);
}

static void
transcribeStackRowContents (xmlNode * row)
{
  xmlNode *child;

  if (row == NULL)
    return;
  if (isMathElement (row, "msrow") || isMathElement (row, "mtr") ||
      isMathElement (row, "mtd"))
    {
      for (child = row->children; child != NULL; child = child->next)
        if (child->type == XML_ELEMENT_NODE)
          transcribe_math (child, 1);
      return;
    }
  if (isMathElement (row, "msline"))
    {
      insertMathCharacter ('=');
      return;
    }
  transcribe_math (row, 1);
}

static void
finishStackRow (void)
{
  insertMathTableMarker ('c');
  insertMathTableMarker ('r');
}

static void
transcribeStackRows (xmlNode * first)
{
  xmlNode *row;
  xmlNode *nested;

  for (row = first; row != NULL; row = row->next)
    {
      if (row->type != XML_ELEMENT_NODE)
        continue;
      if (isMathElement (row, "msgroup") || isMathElement (row, "mstack"))
        {
          for (nested = row->children; nested != NULL; nested = nested->next)
            if (nested->type == XML_ELEMENT_NODE)
              {
                transcribeStackRowContents (nested);
                finishStackRow ();
              }
        }
      else
        {
          transcribeStackRowContents (row);
          finishStackRow ();
        }
    }
}

static StyleType *
startRussianMathTable (xmlNode * node)
{
  StyleType *matrixStyle = lookup_style ((xmlChar *) "matrix");

  mathTrans ();
  if (matrixStyle != NULL)
    start_style (matrixStyle, node);
  return matrixStyle;
}

static void
finishRussianMathTable (StyleType * matrixStyle)
{
  mathTrans ();
  if (matrixStyle != NULL)
    end_style ();
}

static void
russianMathStack (xmlNode * node)
{
  StyleType *matrixStyle = startRussianMathTable (node);

  transcribeStackRows (node->children);
  finishRussianMathTable (matrixStyle);
}

static xmlNode *
nextStackExpression (xmlNode * node)
{
  return nextMathElement (node == NULL ? NULL : node->next);
}

static void
russianLongDivision (xmlNode * node)
{
  xmlNode *divisor = firstMathChild (node);
  xmlNode *result = nextStackExpression (divisor);
  xmlNode *firstStack = nextStackExpression (result);
  xmlNode *dividend = firstStack;
  xmlNode *remaining;
  StyleType *matrixStyle;

  if (isMathElement (firstStack, "mstack"))
    dividend = firstMathChild (firstStack);
  if (divisor == NULL || result == NULL || dividend == NULL)
    return;

  matrixStyle = startRussianMathTable (node);
  transcribeStackRowContents (dividend);
  insertMathCharacter (' ');
  insertMathCharacter (0x2215);
  insertMathCharacter (' ');
  transcribe_math (divisor, 1);
  insertMathCharacter (' ');
  insertMathCharacter ('=');
  insertMathCharacter (' ');
  transcribe_math (result, 1);
  finishStackRow ();

  if (isMathElement (firstStack, "mstack"))
    remaining = nextStackExpression (dividend);
  else
    remaining = nextStackExpression (firstStack);
  transcribeStackRows (remaining);
  finishRussianMathTable (matrixStyle);
}

/* Current ru-math.uti uses lowered digits without a number sign or a closing
 * sign for an integer denominator, root index, exponent or subscript. */
static void
russianIntegerStructure (xmlNode * node)
{
  xmlNode *first = nextMathElement (node->children);
  xmlNode *second = first == NULL ? NULL : nextMathElement (first->next);

  if (first == NULL ||
      (!isIntegerMathToken (second) && !isInfinityMathToken (second)))
    return;
  if (isMathElement (node, "mfrac"))
    {
      transcribe_math (first, 1);
      insertLoweredInteger (second);
    }
  else if (isMathElement (node, "mroot"))
    {
      insertMathCharacter (0xf32a);
      insertLoweredInteger (second);
      insertMathCharacter (0xf402);
      insertMathCharacter (0xf321);
      transcribe_math (first, 1);
      insertMathCharacter (0xf32b);
    }
  else if (isMathElement (node, "msub") || isMathElement (node, "msup"))
    {
      transcribe_math (first, 1);
      insertMathCharacter (isMathElement (node, "msub") ? 0xf320 : 0xf321);
      if (isIntegerMathToken (second))
	insertLoweredInteger (second);
      else
	transcribe_math (second, 1);
    }
}

static int
nodeIsDigitOrPunctuation (xmlNode * node)
{
  xmlChar *content;
  int k;
  int result = 0;

  if (node == NULL)
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL || content[0] == 0)
    goto cleanup;
  if (isMathElement (node, "mn"))
    {
      result = 1;
      for (k = 0; content[k] != 0; k++)
	if (content[k] < '0' || content[k] > '9')
	  {
	    result = 0;
	    break;
	  }
    }
  else if (isMathElement (node, "mo") && content[1] == 0 &&
           strchr (".,;:!?()[]{}", content[0]) != NULL)
    result = 1;

cleanup:
  if (content != NULL)
    xmlFree (content);
  return result;
}

static int
nodeEndsInRussianVowel (xmlNode * node)
{
  static const char *vowels[] = {
    "\xd0\xb0", "\xd0\xb5", "\xd1\x91", "\xd0\xb8", "\xd0\xbe",
    "\xd1\x83", "\xd1\x8b", "\xd1\x8d", "\xd1\x8e", "\xd1\x8f",
    "\xd0\x90", "\xd0\x95", "\xd0\x81", "\xd0\x98", "\xd0\x9e",
    "\xd0\xa3", "\xd0\xab", "\xd0\xad", "\xd0\xae", "\xd0\xaf",
    NULL
  };
  xmlChar *content;
  const xmlChar *last;
  int length;
  int k;
  int result = 0;

  if (node == NULL)
    return 0;
  content = xmlNodeGetContent (node);
  if (content == NULL)
    return 0;
  length = xmlUTF8Strlen (content);
  last = length > 0 ? xmlUTF8Strpos (content, length - 1) : NULL;
  for (k = 0; last != NULL && vowels[k] != NULL; k++)
    if (xmlStrEqual (last, (const xmlChar *) vowels[k]))
      {
	result = 1;
	break;
      }
  xmlFree (content);
  return result;
}

static xmlNode *
firstMathChild (xmlNode * node)
{
  return node == NULL ? NULL : nextMathElement (node->children);
}

static xmlNode *
lastMathChild (xmlNode * node)
{
  xmlNode *child = node == NULL ? NULL : node->last;
  while (child != NULL && child->type != XML_ELEMENT_NODE)
    child = child->prev;
  return child;
}

static int
bracketPairNeedsRecognition (int square, xmlNode *container,
                             xmlNode *firstInside, xmlNode *lastInside)
{
  xmlNode *previous = container == NULL ? NULL :
    previousMathElement (container->prev);
  xmlNode *next = container == NULL ? NULL : nextMathElement (container->next);

  if (square)
    {
      int openingAmbiguous = previous != NULL &&
        !isMathElement (previous, "mspace") &&
        !nodeEndsInRussianVowel (previous) &&
        !nodeIsDigitOrPunctuation (previous) &&
        !nodeIsDigitOrPunctuation (firstInside);
      int closingAmbiguous = lastInside != NULL &&
        !nodeEndsInRussianVowel (lastInside) &&
        !nodeIsDigitOrPunctuation (lastInside) &&
        !nodeIsDigitOrPunctuation (next);
      return openingAmbiguous || closingAmbiguous;
    }
  return (!nodeIsDigitOrPunctuation (previous) &&
          !nodeIsDigitOrPunctuation (firstInside)) ||
    (!nodeIsDigitOrPunctuation (lastInside) &&
     !nodeIsDigitOrPunctuation (next));
}

static int
isBracketContent (const xmlChar *content, const char *ascii,
                  const char *unicode)
{
  return content != NULL &&
    (xmlStrEqual (content, (const xmlChar *) ascii) ||
     (unicode != NULL && xmlStrEqual (content, (const xmlChar *) unicode)));
}

static void
russianRecognizedBracket (xmlNode * node)
{
  xmlNode *container = node == NULL ? NULL : node->parent;
  xmlNode *first = firstMathChild (container);
  xmlNode *last = lastMathChild (container);
  xmlNode *firstInside = first == NULL ? NULL : nextMathElement (first->next);
  xmlNode *lastInside = last == NULL ? NULL : previousMathElement (last->prev);
  xmlChar *content = xmlNodeGetContent (node);
  xmlChar *firstContent = first == NULL ? NULL : xmlNodeGetContent (first);
  xmlChar *lastContent = last == NULL ? NULL : xmlNodeGetContent (last);
  int square = isBracketContent (firstContent, "[", NULL) &&
    isBracketContent (lastContent, "]", NULL);
  int angle = (isBracketContent (firstContent, "<", "\xe2\x9f\xa8") &&
               isBracketContent (lastContent, ">", "\xe2\x9f\xa9"));
  int recognized = (square || angle) &&
    bracketPairNeedsRecognition (square, container, firstInside, lastInside);

  if (isBracketContent (content, "[", NULL))
    insertMathCharacter (recognized ? 0xf508 : '[');
  else if (isBracketContent (content, "]", NULL))
    insertMathCharacter (recognized ? 0xf509 : ']');
  else if (isBracketContent (content, "<", "\xe2\x9f\xa8"))
    insertMathCharacter (recognized ? 0xf50a : 0x27e8);
  else if (isBracketContent (content, ">", "\xe2\x9f\xa9"))
    insertMathCharacter (recognized ? 0xf50b : 0x27e9);
  if (content != NULL)
    xmlFree (content);
  if (firstContent != NULL)
    xmlFree (firstContent);
  if (lastContent != NULL)
    xmlFree (lastContent);
}

static void
russianFenced (xmlNode * node)
{
  xmlChar *open = xmlGetProp (node, (const xmlChar *) "open");
  xmlChar *close = xmlGetProp (node, (const xmlChar *) "close");
  xmlChar *separators = xmlGetProp (node, (const xmlChar *) "separators");
  xmlNode *first = firstMathChild (node);
  xmlNode *last = lastMathChild (node);
  xmlNode *child;
  int square;
  int angle;
  int recognized;
  int childCount = 0;

  if (open == NULL)
    open = xmlStrdup ((const xmlChar *) "(");
  if (close == NULL)
    close = xmlStrdup ((const xmlChar *) ")");
  square = xmlStrEqual (open, (const xmlChar *) "[") &&
    xmlStrEqual (close, (const xmlChar *) "]");
  angle = isBracketContent (open, "<", "\xe2\x9f\xa8") &&
    isBracketContent (close, ">", "\xe2\x9f\xa9");
  recognized = (square || angle) &&
    bracketPairNeedsRecognition (square, node, first, last);

  if (square)
    insertMathCharacter (recognized ? 0xf508 : '[');
  else if (angle)
    insertMathCharacter (recognized ? 0xf50a : 0x27e8);
  else
    insert_utf8 (open);

  for (child = node->children; child != NULL; child = child->next)
    if (child->type == XML_ELEMENT_NODE)
      {
        if (childCount++ > 0)
          insertMathCharacter (separators != NULL && separators[0] != 0 ?
                               separators[0] : ',');
        transcribe_math (child, 1);
      }

  if (square)
    insertMathCharacter (recognized ? 0xf509 : ']');
  else if (angle)
    insertMathCharacter (recognized ? 0xf50b : 0x27e9);
  else
    insert_utf8 (close);
  xmlFree (open);
  xmlFree (close);
  if (separators != NULL)
    xmlFree (separators);
}

static void
transcribeScriptGroup (xmlNode * first, xmlNode * stop, int offset,
		       widechar indicator)
{
  xmlNode *script;
  int scriptCount = 0;
  int allInteger = 1;
  int haveScript = 0;
  int originalOffset = offset;

  for (script = first; script != stop; script = script->next)
    {
      if (script->type != XML_ELEMENT_NODE)
	continue;
      if (offset > 0)
	{
	  offset--;
	  continue;
	}
      offset = 1;
      if (!isMathElement (script, "none") &&
	  !isMathElement (script, "mprescripts"))
	{
	  scriptCount++;
	  if (!isIntegerMathToken (script))
	    allInteger = 0;
	}
    }

  if (scriptCount > 0 && allInteger)
    {
      insertMathCharacter (indicator);
      offset = originalOffset;
      for (script = first; script != stop; script = script->next)
	{
	  if (script->type != XML_ELEMENT_NODE)
	    continue;
	  if (offset > 0)
	    {
	      offset--;
	      continue;
	    }
	  offset = 1;
	  if (!isMathElement (script, "none") &&
	      !isMathElement (script, "mprescripts"))
	    insertLoweredInteger (script);
	}
      return;
    }

  offset = originalOffset;

  for (script = first; script != stop; script = script->next)
    {
      if (script->type != XML_ELEMENT_NODE)
	continue;
      if (offset > 0)
	{
	  offset--;
	  continue;
	}
      offset = 1;
      if (!isMathElement (script, "none") &&
	  !isMathElement (script, "mprescripts"))
	{
	  if (!haveScript)
	    insertMathCharacter (indicator);
	  transcribe_math (script, 1);
	  haveScript = 1;
	}
    }

  if (haveScript)
    {
      insertMathCharacter (0xf402);
      insertMathCharacter (indicator);
    }
}

/* MathML stores prescripts after the base and postscripts. Russian linear
 * mathematical Braille requires all left indices before the base. Indices at
 * the same level share one indicator and one terminator. */
static void
russianMultiscripts (xmlNode * node)
{
  xmlNode *base = node->children;
  xmlNode *prescripts = NULL;
  xmlNode *child;

  while (base != NULL && base->type != XML_ELEMENT_NODE)
    base = base->next;
  if (base == NULL)
    return;

  for (child = base->next; child != NULL; child = child->next)
    if (isMathElement (child, "mprescripts"))
      {
	prescripts = child;
	break;
      }

  if (prescripts != NULL)
    {
      transcribeScriptGroup (prescripts->next, NULL, 0, 0xf320);
      transcribeScriptGroup (prescripts->next, NULL, 1, 0xf321);
    }

  transcribe_math (base, 1);
  transcribeScriptGroup (base->next, prescripts, 0, 0xf320);
  transcribeScriptGroup (base->next, prescripts, 1, 0xf321);
}

static void
mathCreateBrlNode ()
{
  xmlNode *curBrlNode;
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  xmlSetProp (newNode, (xmlChar *) "modifiers", (xmlChar *) "notext");
  curBrlNode = xmlAddNextSibling (curLink, newNode);
  link_brl_node (curBrlNode);
  curLink = curBrlNode;
}
