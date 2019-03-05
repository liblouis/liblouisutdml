/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   Abilitiessoft, Inc. www.abilitiessoft.org
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
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"

typedef struct EndnoteStruct EndnoteStruct;
typedef enum free_codes free_codes;

struct EndnoteStruct
 {
   EndnoteStruct* next;
   EndnoteStruct* prev;
   char* id;
   int id_length;
   widechar* character;
   int character_length;
   widechar* page_num;
   int page_num_length;
   widechar* line_num;
   int line_num_length;
   widechar* endnote;
   int endnote_length;
   int has_endnote_pos;
   int free_code;
 };
 
static EndnoteStruct workingEndnote;
static EndnoteStruct *firstEndnote;
static EndnoteStruct *lastEndnote;
static widechar* notes_header;
static int notes_header_length;
static widechar* notes_description;
static int notes_description_length;
static int old_translated_length;

static void free_endnotes_from(EndnoteStruct *start);

//initialisation to make sure everything is set up correctly
int initialize_endnotes (void)
{
  static int initialized=0;
  if(!initialized)
  {
	firstEndnote = NULL;
	lastEndnote = NULL;
	notes_header = NULL;
	notes_description = NULL;
	notes_header_length = 0;
	notes_description_length = 0;
	ud->endnote_stage=1;
	initialized=1;
  }
  return 1;
}

// finds the endnote with id 'id' and returns a pointer to that endnote.
static EndnoteStruct* find_endnote_from_id(const char* id)
{
	EndnoteStruct* current;
		
	for(current = firstEndnote;current!=NULL;current=current->next)
	{
		if(strcmp(current->id,id)==0)
			return current;
	}
	return NULL;
}

// returns the length of the current braille page number string
static int braille_page_num_length()
{
	int braille_string_length=0;
	while(ud->braille_page_string[braille_string_length])
		braille_string_length++;
	return braille_string_length;
}

// roughly copied from hyphenatex in transcriber.c, taking out any code that
// isn't needed in this situation and made changes as below:
// instead of lastBlank, this takes in wordStart, which should be lastBlank+1.
// added wordLength argument, as already calculated where needed.
// no need for hyphenAdded so removed.
// breakAt returns the relative position from wordStart (breakAt=k), rather than
// the absolute position (breakAt=wordStart+k).
static int hyphenatex (int wordStart, int wordLength,int lineEnd, int *breakAt)
{
#define MIN_SYLLABLE_LENGTH 2
#define MIN_WORD_LENGTH 5
#define MIN_NEXT_LINE 12
#define minimum(a,b) ((a<b)?a:b)

  int k;
  char hyphens[MAXNAMELEN];
     
  if (ud->hyphenate != 1 && ud->hyphenate != 2)
    return 0;
	
  if ((ud->translated_length - wordStart) < MIN_NEXT_LINE)
    return 0;

  if (wordLength < MIN_WORD_LENGTH || wordLength > ud->cells_per_line)
    return 0;
    
  for (k = minimum(lineEnd - wordStart, wordLength - MIN_SYLLABLE_LENGTH);
       k > MIN_SYLLABLE_LENGTH;
       k--)
    {
	  if(ud->translated_buffer[wordStart + k - 1] == *ud->lit_hyphen)
	    {
	      *breakAt = k;
	      return 1;
	    }
    }
    
  for (k = 0; k <= wordLength; k++)
    hyphens[k] = '0';
  
  if (!lou_hyphenate (ud->main_braille_table,
			   &ud->translated_buffer[wordStart], wordLength,
			   hyphens, 1))
    return 0;
  
  for (k = minimum(lineEnd - wordStart, wordLength - MIN_SYLLABLE_LENGTH) - 1;
       k > MIN_SYLLABLE_LENGTH;
       k--)
    if (hyphens[k] == '1')
      {
	*breakAt = k;
	return 1;
      }
  
  return 0;
}

// starting at the current line (the line of the start of the current paragraph),
// this returns the line number and page number (in 'pagenum' and 'linenum' respectively),
// from the number of characters that need to be written ('num_chars').
// could be improved/replaced as most likely not perfect.
static void get_page_line_num(int num_chars,int* pagenum,int* linenum)
{
	int first_line = 1;
	int line_num = ud->lines_on_page+ud->blank_lines+1;
	int page_num = ud->braille_page_number;
	int chars_done = 0;
	int available_cells = ud->cells_per_line;
	available_cells -= ud->style_first_line_indent;
	if(line_num==1)
		available_cells -= braille_page_num_length();
	available_cells -= ud->style_left_margin;
	available_cells -= ud->style_right_margin;
	while(chars_done < num_chars)
	{
		int word_length;

		if(num_chars - chars_done < available_cells)
			break;
		else
		{
			int last_word_begin;
			int breakat;

			for(last_word_begin=available_cells;last_word_begin>0;last_word_begin--)
				if(ud->translated_buffer[chars_done+last_word_begin] == ' ')
					break;
					
			for(word_length = last_word_begin+1;chars_done+word_length<num_chars;word_length++)
				if(ud->translated_buffer[chars_done+word_length] == ' ')
					break;
			word_length-=last_word_begin;
			
			line_num++;
			available_cells = ud->cells_per_line;
			available_cells -= ud->style_left_margin;
			available_cells -= ud->style_right_margin;
			if(last_word_begin==0)
				available_cells -= (word_length+1);
			else if(ud->hyphenate)
			{
				if(hyphenatex(chars_done+last_word_begin+1,word_length,chars_done+available_cells,&breakat))
					available_cells -= (word_length-breakat+1);
				else
					available_cells -= (word_length+1);
			}
			else
				available_cells -= (word_length+1);

			chars_done += last_word_begin+word_length + 1;
			if(line_num > ud->lines_per_page)
			{
				if(ud->style_format == centered)
					available_cells -= 2*braille_page_num_length();
				else
					available_cells -= braille_page_num_length();
				page_num++;
				line_num=1;
			}
		}
	}
	*linenum = line_num;
	*pagenum = page_num;
}

// gets the current translation length for use in finish_endnote
 int start_endnote(void)
 {	
	insert_translation(ud->main_braille_table);
	old_translated_length = ud->translated_length;
	return 1;
 }
 
// uses the change in length of the translation buffer to
// calculate the content of 'node', which it uses for the endnote
// character, and the attribute of 'node' as its id.
int finish_endnote(xmlNode* node)
{
	xmlNode *child;
	int node_translated_length;
	int k;
	int branchCount=0;
	EndnoteStruct* endnotePtr;
	char buffer[MAXNUMLEN];
	int found_endnote=0;
	int lines_added;
	int line_num;
	int page_num;
	
	endnotePtr = find_endnote_from_id(get_attr_value(node));
	if(endnotePtr == NULL)
	{
		endnotePtr = &workingEndnote;
		endnotePtr->id = get_attr_value(node);
		endnotePtr->id_length = strlen(endnotePtr->id);
	}
	else if(endnotePtr != NULL && endnotePtr->has_endnote_pos!=0)
	{
		logMessage(LOU_LOG_DEBUG,"finish_endnote:Endnote with id %s already exists",get_attr_value(node));
		return 0;
	}
	else found_endnote=1;
	
	insert_translation (ud->main_braille_table);
	
	node_translated_length = ud->translated_length - old_translated_length;
	
	if(node_translated_length == 0) return 0;		// no endnote character
	
	if(ud->translated_buffer[ud->translated_length-1]==32)
		ud->translated_length--;
	
	endnotePtr->character_length = node_translated_length;
	endnotePtr->character = malloc(endnotePtr->character_length * CHARSIZE);
	memcpy(endnotePtr->character,&ud->translated_buffer[old_translated_length],endnotePtr->character_length*CHARSIZE);
	if(ud->translated_buffer[old_translated_length-1]==32)
	{
		memmove(&ud->translated_buffer[old_translated_length+2],&ud->translated_buffer[old_translated_length],endnotePtr->character_length*CHARSIZE);
		ud->translated_length+=2;
		ud->translated_buffer[old_translated_length]=ud->translated_buffer[old_translated_length+1]='9';
	}
	else if(ud->translated_buffer[old_translated_length]==32)
	{
		memmove(&ud->translated_buffer[old_translated_length+3],&ud->translated_buffer[old_translated_length+1],(endnotePtr->character_length-1)*CHARSIZE);
		ud->translated_buffer[old_translated_length+1]=ud->translated_buffer[old_translated_length+2]='9';
		ud->translated_length+=2;
	}
	if(!(endnotePtr->free_code & 1)) endnotePtr->free_code += 1;
	
	get_page_line_num(ud->translated_length,&page_num,&line_num);
	
	sprintf(buffer, "%d", line_num);
	for(k=0;k<strlen(buffer);k++)
		ud->text_buffer[k]=buffer[k];
	ud->text_length = strlen(buffer);
	old_translated_length = ud->translated_length;
	insert_translation(ud->main_braille_table);
	if(ud->translated_buffer[old_translated_length]==32)
	{
		endnotePtr->line_num_length = ud->translated_length-old_translated_length-1;
		endnotePtr->line_num = malloc(endnotePtr->line_num_length * CHARSIZE);
		memcpy(endnotePtr->line_num,&ud->translated_buffer[old_translated_length+1],endnotePtr->line_num_length*CHARSIZE);
	}
	else if(ud->translated_buffer[old_translated_length-1]==32)
	{
		endnotePtr->line_num_length = ud->translated_length-old_translated_length;
		endnotePtr->line_num = malloc(endnotePtr->line_num_length * CHARSIZE);
		memcpy(endnotePtr->line_num,&ud->translated_buffer[old_translated_length],endnotePtr->line_num_length*CHARSIZE);
	}
	ud->translated_length = old_translated_length;
	if(!(endnotePtr->free_code & 8)) endnotePtr->free_code += 8;
	
	
	sprintf(buffer, "%d", page_num);
	for(k=0;k<strlen(buffer);k++)
		ud->text_buffer[k]=buffer[k];
	ud->text_length = strlen(buffer);
	old_translated_length = ud->translated_length;
	insert_translation(ud->main_braille_table);
	if(ud->translated_buffer[old_translated_length]==32)
	{
		endnotePtr->page_num_length = ud->translated_length-old_translated_length-1;
		endnotePtr->page_num = malloc(endnotePtr->line_num_length*CHARSIZE);
		memcpy(endnotePtr->page_num,&ud->translated_buffer[old_translated_length+1],endnotePtr->page_num_length*CHARSIZE);
	}
	else if(ud->translated_buffer[old_translated_length-1]==32)
	{
		endnotePtr->page_num_length = ud->translated_length-old_translated_length;
		endnotePtr->line_num = malloc(endnotePtr->page_num_length * CHARSIZE);
		memcpy(endnotePtr->line_num,&ud->translated_buffer[old_translated_length],endnotePtr->page_num_length*CHARSIZE);
	}
	ud->translated_length = old_translated_length;
	if(!(endnotePtr->free_code & 4)) endnotePtr->free_code += 4;

	if(found_endnote)
	{
		if(endnotePtr != lastEndnote)
		{
			if(endnotePtr == firstEndnote)
			{
				firstEndnote = endnotePtr->next;
				(endnotePtr->next)->prev = NULL;
				endnotePtr->prev = lastEndnote;
				endnotePtr->next = NULL;
				lastEndnote->next = endnotePtr;
				lastEndnote = endnotePtr;
			}
			else
			{
				(endnotePtr->prev)->next = endnotePtr->next;
				(endnotePtr->next)->prev = endnotePtr->prev;
				endnotePtr->prev = lastEndnote;
				endnotePtr->next = NULL;
				lastEndnote->next = endnotePtr;
				lastEndnote = endnotePtr;
			}
		}
		endnotePtr->has_endnote_pos=1;
		return 1;
	}
	
	endnotePtr->has_endnote_pos=1;
	endnotePtr->next = NULL;
	endnotePtr->prev = lastEndnote;
	endnotePtr = malloc (sizeof(workingEndnote));
	memcpy (endnotePtr, &workingEndnote, sizeof(workingEndnote));
	if (lastEndnote != NULL)
		lastEndnote->next = endnotePtr;
	lastEndnote = endnotePtr;
	if (firstEndnote == NULL)
		firstEndnote = endnotePtr;
	return 0;
}
 
// called when a note cannot find a noteref with the same id.
// assumes that the noteref will come after it.
// creates a basic endnote that contains the endnote description and id from 'node'.
// when create_endnote is called (from a noteref), it checks all the
// endnotes created by this function, and if it finds one with the same id,
// adds the rest of the information.
int create_endnote_shell(xmlNode* node)
{
	int k;
	EndnoteStruct* endnotePtr;
	
	workingEndnote.id = get_attr_value(node);
	workingEndnote.id_length = strlen(workingEndnote.id);
	
	workingEndnote.free_code = 0;
	
	workingEndnote.endnote_length=ud->translated_length;
	workingEndnote.endnote = malloc(workingEndnote.endnote_length*CHARSIZE);
	for(k=0;k<workingEndnote.endnote_length;k++)
		workingEndnote.endnote[k] = ud->translated_buffer[k];
	if(!(workingEndnote.free_code & 2))
		workingEndnote.free_code += 2;
		
	workingEndnote.has_endnote_pos=0;
	workingEndnote.next = NULL;
	workingEndnote.prev = lastEndnote;
	endnotePtr = malloc (sizeof(workingEndnote));
	memcpy (endnotePtr, &workingEndnote, sizeof(workingEndnote));
	if (lastEndnote != NULL)
		lastEndnote->next = endnotePtr;
	lastEndnote = endnotePtr;
	if (firstEndnote == NULL)
		firstEndnote = endnotePtr;
	return 1;
}

// called by the note action.
// tries to find a corresponding noteref with the id from the attribute of 'node'.
// if it finds it, then it adds the content of 'node' as the description of this endnote.
// if it doesn't find it, then it creates a new basic endnote (see create_endnote_shell), 
// containing just the id and description, hoping to link up to a noteref later on.
// note: when the endnotes are being displayed, any endnotes that are created as basic ones
// from here, but do not link up to a noteref later on, will not be displayed.
int link_endnote(xmlNode* node)
{
	EndnoteStruct* linked = find_endnote_from_id(get_attr_value(node));
		
	if(linked==NULL)
		create_endnote_shell(node);
	else if(linked!=NULL && linked->has_endnote_pos==1)
	{
		int k;
		linked->endnote_length=ud->translated_length;
		linked->endnote = malloc(linked->endnote_length * CHARSIZE);
		
		for(k=0;k<linked->endnote_length;k++)
			linked->endnote[k] = ud->translated_buffer[k];
			
		if(!(linked->free_code & 2)) linked->free_code += 2;
	}
	else
	{
		logMessage(LOU_LOG_DEBUG,"link_endnote:Endnote with id %s already exists",get_attr_value(node));
		return 0;
	}
	
	return 1;
}

// set the notes header as what has been translated.
// if one is already set, then this overwrites it.
void set_notes_header()
{
	if(notes_header==NULL)
	{
		notes_header = malloc(ud->translated_length * CHARSIZE);
		memcpy(notes_header,ud->translated_buffer,ud->translated_length*CHARSIZE);
		notes_header_length = ud->translated_length;
	}
	else
	{
		free(notes_header);
		notes_header=NULL;
		set_notes_header();
	}
}

// set the notes description as what has been translated.
// if one is already set, then this overwrites it.
void set_notes_description()
{
	if(notes_description==NULL)
	{
		notes_description = malloc(ud->translated_length * CHARSIZE);
		memcpy(notes_description,ud->translated_buffer,ud->translated_length*CHARSIZE);
		notes_description_length = ud->translated_length;
	}
	else
	{
		free(notes_description);
		notes_description=NULL;
		set_notes_description();
	}
}

// called at the end of the translating process.
// this adds the endnote header and description using their respective styles,
// then steps through all the endnotes in order, adding in the endnote character,
// its page and line number, and its note.
int make_endnotes(void)
{
	EndnoteStruct* current_endnote = NULL;
	
	ud->endnote_stage = 2;
	if(!ud->endnotes) return 1;
		
	if(firstEndnote != NULL)
	{
		int k;
		StyleType* style;	
		int has_newline=0;
		do_newpage();
		if(ud->running_head_length>0)
		{
			do_blankline();
			has_newline=1;
		}
		insert_translation(ud->main_braille_table);
		if(notes_header_length > 0)
		{
			style = action_to_style(notesheader);
			start_style(style,NULL);
			memcpy(&ud->translated_buffer[ud->translated_length],notes_header,notes_header_length*CHARSIZE);
			ud->translated_length+=notes_header_length;
			end_style(style);
			do_blankline();
			has_newline=1;
			free(notes_header);
		}
		if(notes_description_length)
		{
			style = action_to_style(notesdescription);
			start_style(style,NULL);
			memcpy(&ud->translated_buffer[ud->translated_length],notes_description,notes_description_length*CHARSIZE);
			ud->translated_length+=notes_description_length;
			end_style(style);
			do_blankline();
			has_newline=1;
			free(notes_description);
		}
		if(!has_newline)
			do_blankline();
		current_endnote = firstEndnote;
		while (current_endnote != NULL)
		{
			if(!current_endnote->has_endnote_pos)
			{
				current_endnote = current_endnote->next;
				continue;
			}
			style = action_to_style(note);
			start_style(style,NULL);
			memcpy(&ud->translated_buffer[ud->translated_length],current_endnote->character,current_endnote->character_length*CHARSIZE);
			ud->translated_length += current_endnote->character_length;
			if(ud->braille_pages && current_endnote->page_num_length!=0)
			{
				ud->translated_buffer[ud->translated_length++] = 160;
				ud->translated_buffer[ud->translated_length++] = 'p';
				memcpy(&ud->translated_buffer[ud->translated_length],current_endnote->page_num,current_endnote->page_num_length*CHARSIZE);
				if(current_endnote->line_num_length!=0)
				{
					ud->translated_length += current_endnote->page_num_length;
					memcpy(&ud->translated_buffer[ud->translated_length],current_endnote->line_num,current_endnote->line_num_length*CHARSIZE);
					ud->translated_length += current_endnote->line_num_length;
				}
			}
			ud->translated_buffer[ud->translated_length++] = ' ';
			memcpy(&ud->translated_buffer[ud->translated_length],current_endnote->endnote,current_endnote->endnote_length*CHARSIZE);
			ud->translated_length += current_endnote->endnote_length;
			end_style(style);
			ud->in_sync=0;
			current_endnote = current_endnote->next;
		}
		do_newpage ();
		write_buffer(1,0);
		free_endnotes_from (firstEndnote);
		firstEndnote = NULL;
    }
  return 1;
}

// frees the memory of all resources belonging to 'endnote'
static void free_endnote(EndnoteStruct* endnote)
{
	if(endnote->free_code&1) free(endnote->character);
	if(endnote->free_code&2) free(endnote->endnote);
	if(endnote->free_code&4) free(endnote->page_num);
	if(endnote->free_code&8) free(endnote->line_num);
	free(endnote);
}

// frees the memory of all the endnotes that appear after 'start'
static void free_endnotes_from(EndnoteStruct *start)
 {
   EndnoteStruct *current;
   EndnoteStruct *next;
   if (start == NULL) return;
   current = start;
   while (current != NULL) 
	{
		next = current->next;
		free_endnote(current);
		current = next;
	}
 }
 
