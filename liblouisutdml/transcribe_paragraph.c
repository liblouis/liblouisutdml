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
#include <string.h>
#include "louisutdml.h"

extern void widecharcpy (widechar * to, const widechar * from, int length);
extern void widestrcpy (widechar * to, const widechar * from);
extern void unsignedcharcpy (char *to, const char *from, int length);
extern void save_translated_buffer (void);
extern void restore_translated_buffer (void);
extern void contents_save_state (void);
extern void contents_restore_state (void);

static int dont_split = 0;
static int dont_split_status = 0;
static int keep_with_next = 0;
static int keep_with_previous = 0;
static int keep_with_previous_pos = 0;
static int keep_with_previous_status = 0;
static int orphan_control = 0;
static int orphan_control_pos = 0;
static int orphan_control_status = 0;
static int widow_control = 0;
static int widow_control_pos = 0;
static int widow_control_status = 0;
static int saved_text_length;
static int saved_sync_text_length;
static int saved_translated_length;
static int saved_outbuf1_len_so_far;
static int saved_outbuf2_len_so_far;
static int saved_running_head_length;
static int saved_footer_length;
static int saved_braille_page_number;
static int saved_style_left_margin;
static int saved_style_right_margin;
static int saved_style_first_line_indent;
static StyleFormat saved_style_format;
static BrlPageNumFormat saved_brl_page_num_format;
static BrlPageNumFormat saved_cur_brl_page_num_format;
static int saved_lines_on_page;
static int saved_line_spacing;
static int saved_blank_lines;
static int saved_fill_pages;
static int saved_positions_array[2 * BUFSIZE];
static widechar saved_text_buffer[2 * BUFSIZE];
static widechar saved_sync_text_buffer[2 * BUFSIZE];
static widechar saved_translated_buffer[2 * BUFSIZE];
static widechar saved_outbuf1[2 * BUFSIZE];
static widechar saved_outbuf2[2 * BUFSIZE];
static widechar saved_page_separator_number_first[MAXNUMLEN];
static widechar saved_page_separator_number_last[MAXNUMLEN];
static widechar saved_print_page_number_first[MAXNUMLEN];
static widechar saved_print_page_number_last[MAXNUMLEN];
static widechar saved_print_page_number[MAXNUMLEN];
static widechar saved_braille_page_string[MAXNUMLEN];
static widechar saved_running_head[MAXNAMELEN / 2];
static widechar saved_footer[MAXNAMELEN / 2];
static unsigned char saved_typeform[2 * BUFSIZE];

static void
saveState (void)
{
  saved_text_length = ud->text_length;
  saved_sync_text_length = ud->sync_text_length;
  saved_translated_length = ud->translated_length;
  saved_outbuf1_len_so_far = ud->outbuf1_len_so_far;
  saved_outbuf2_len_so_far = ud->outbuf2_len_so_far;
  saved_running_head_length = ud->running_head_length;
  saved_footer_length = ud->footer_length;
  saved_style_left_margin = ud->style_left_margin;
  saved_style_right_margin = ud->style_right_margin;
  saved_style_first_line_indent = ud->style_first_line_indent;
  saved_style_format = ud->style_format;
  saved_braille_page_number = ud->braille_page_number;
  saved_brl_page_num_format = ud->brl_page_num_format;
  saved_cur_brl_page_num_format = ud->cur_brl_page_num_format;
  saved_lines_on_page = ud->lines_on_page;
  saved_line_spacing = ud->line_spacing;
  saved_blank_lines = ud->blank_lines;
  saved_fill_pages = ud->fill_pages;
  memcpy (saved_positions_array, ud->positions_array,
	  saved_translated_length * sizeof (int));
  widecharcpy (saved_text_buffer, ud->text_buffer, saved_text_length);
  widecharcpy (saved_sync_text_buffer, ud->sync_text_buffer,
	       saved_sync_text_length);
  widecharcpy (saved_translated_buffer, ud->translated_buffer,
	       saved_translated_length);
  widecharcpy (saved_outbuf1, ud->outbuf1, saved_outbuf1_len_so_far);
  widecharcpy (saved_outbuf2, ud->outbuf2, saved_outbuf2_len_so_far);
  widecharcpy (saved_running_head, ud->running_head,
	       saved_running_head_length);
  widecharcpy (saved_footer, ud->footer, saved_footer_length);
  memcpy(saved_typeform, ud->typeform, saved_text_length * sizeof(formtype));

  widestrcpy (saved_page_separator_number_first,
	      ud->page_separator_number_first);
  widestrcpy (saved_page_separator_number_last,
	      ud->page_separator_number_last);
  widestrcpy (saved_print_page_number_first, ud->print_page_number_first);
  widestrcpy (saved_print_page_number_last, ud->print_page_number_last);
  widestrcpy (saved_print_page_number, ud->print_page_number);
  widestrcpy (saved_braille_page_string, ud->braille_page_string);
  save_translated_buffer ();
  contents_save_state ();
}

static void
restoreState (void)
{
  ud->text_length = saved_text_length;
  ud->sync_text_length = saved_sync_text_length;
  ud->translated_length = saved_translated_length;
  ud->outbuf1_len_so_far = saved_outbuf1_len_so_far;
  ud->outbuf2_len_so_far = saved_outbuf2_len_so_far;
  ud->running_head_length = saved_running_head_length;
  ud->footer_length = saved_footer_length;
  ud->style_left_margin = saved_style_left_margin;
  ud->style_right_margin = saved_style_right_margin;
  ud->style_first_line_indent = saved_style_first_line_indent;
  ud->style_format = saved_style_format;
  ud->braille_page_number = saved_braille_page_number;
  ud->brl_page_num_format = saved_brl_page_num_format;
  ud->cur_brl_page_num_format = saved_cur_brl_page_num_format;
  ud->lines_on_page = saved_lines_on_page;
  ud->line_spacing = saved_line_spacing;
  ud->blank_lines = saved_blank_lines;
  ud->fill_pages = saved_fill_pages;
  memcpy (ud->positions_array, saved_positions_array,
	  saved_translated_length * sizeof (int));
  widecharcpy (ud->text_buffer, saved_text_buffer, saved_text_length);
  widecharcpy (ud->sync_text_buffer, saved_sync_text_buffer,
	       saved_sync_text_length);
  widecharcpy (ud->translated_buffer, saved_translated_buffer,
	       saved_translated_length);
  widecharcpy (ud->outbuf1, saved_outbuf1, saved_outbuf1_len_so_far);
  widecharcpy (ud->outbuf2, saved_outbuf2, saved_outbuf2_len_so_far);
  widecharcpy (ud->running_head, saved_running_head,
	       saved_running_head_length);
  widecharcpy (ud->footer, saved_footer, saved_footer_length);
  memcpy(ud->typeform, saved_typeform, saved_text_length * sizeof(formtype));

  widestrcpy (ud->page_separator_number_first,
	      saved_page_separator_number_first);
  widestrcpy (ud->page_separator_number_last,
	      saved_page_separator_number_last);
  widestrcpy (ud->print_page_number_first, saved_print_page_number_first);
  widestrcpy (ud->print_page_number_last, saved_print_page_number_last);
  widestrcpy (ud->print_page_number, saved_print_page_number);
  widestrcpy (ud->braille_page_string, saved_braille_page_string);
  restore_translated_buffer ();
  contents_restore_state ();
}

int
transcribe_paragraph (xmlNode * node, int action)
{
  xmlNode *saved_child;
  int saved_branchCount = 0;
  int state_saved = 0;
  StyleType *style_this;
  int dont_split_this = 0;
  int keep_with_next_this = 0;
  int keep_with_previous_this = 0;
  int orphan_control_this = 0;
  int widow_control_this = 0;
  StyleType *style;
  int haveMacro = 0;
  xmlNode *child;
  int branchCount = 0;
  int i;
  logMessage(LOU_LOG_DEBUG, "Begin transcribe_paragraph");
  if (node == NULL)
    return 0;
  if (ud->top == 0)
    action = 1;
  if (action != 0)
    push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case no:
      if (ud->format_for == utd)
	break;
      if (ud->text_length > 0 && ud->text_length < MAX_LENGTH &&
	  ud->text_buffer[ud->text_length - 1] > 32)
	ud->text_buffer[ud->text_length++] = 32;
      break;
    case skip:
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case markhead:
      ud->head_node = node;
      pop_sem_stack ();
      break;
    case configtweak:
      do_configstring (node);
      ud->main_braille_table = ud->contracted_table_name;
      if (!lou_getTable (ud->main_braille_table))
	{
	  logMessage (LOU_LOG_ERROR, "Cannot open main table %s", ud->main_braille_table);
	  return 0;
	}
      if (node->children == NULL)
	return 1;
      break;
    case htmllink:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case htmltarget:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 1);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case pagebreak:
      do_pagebreak (node);
      return 1;
    case attrtotext:
      do_attrtotext (node);
      if (node->children == NULL)
	return 1;
      break;
    case blankline:
      do_blankline ();
      if (node->children == NULL)
	return 1;
      break;
    case linespacing:
      do_linespacing (node);
      if (node->children == NULL)
	return 1;
      break;
    case softreturn:
      do_softreturn ();
      if (node->children == NULL)
	return 1;
      break;
    case newpage:
      do_newpage ();
      if (node->children == NULL)
	return 1;
      break;
    case righthandpage:
      do_righthandpage ();
      if (node->children == NULL)
	return 1;
      break;
    case code:
      transcribe_computerCode (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case math:
      transcribe_math (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case graphic:
      transcribe_graphic (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utddispimg:
    case utdinlnimg:
      do_utdxxxximg (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case chemistry:
      transcribe_chemistry (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case music:
      transcribe_music (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case changetable:
      change_table (node);
            //if (node->children == NULL)			////?? removed, as change_table() cycles through all children anyway, so if changetable has child nodes (ie. <changetable><xyz>Text</xyz><changetable>) then produces duplicate outputs.
			return 1;
      break;
	case noteref:							////
		if(ud->endnotes)
			start_endnote(node);
		else
		{
			if(action!=0)
				pop_sem_stack();
			return 1;
		}
		break;
    case pagenum:
      do_pagenum ();
      break;
    case footer:
        keep_with_next = 1;
        break;
    default:
      break;
    }
  if (is_macro (node))
    {
      haveMacro = 1;
      logMessage(LOU_LOG_DEBUG, "Node has macro");
      start_macro (node);
    }
  else if ((style = is_style (node)) != NULL)
    {
      if (node->children == NULL)
	{
	  if (action != 0)
	    pop_sem_stack ();
	  return 0;
	}
      logMessage(LOU_LOG_DEBUG, "Node has style");
      start_style (style, node);
    }
  child = node->children;
  while (child)
    {
      insert_code (node, branchCount);
      branchCount++;
      dont_split_this = 0;
      orphan_control_this = 0;
      keep_with_previous_this = 0;
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  if (ud->format_for == utd)
	    {
	    }
	  else
	    {
	      style_this = is_style (child);
	      if (style_this && ud->braille_pages)
		{
		  if (keep_with_next_this && ud->lines_length > 0)
		    keep_with_previous_this = 1;
		  keep_with_next_this = 0;
		  if (!dont_split)
		    {
		      if (ud->lines_on_page > 0 &&
			  !((keep_with_previous || keep_with_previous_this) &&
			    !ud->outbuf3_enabled))
			{
			  if (style_this->dont_split
			      || style_this->keep_with_next)
			    dont_split_this = 1;
			  else if (style_this->orphan_control > 1)
			    orphan_control_this = style_this->orphan_control;
			  else if (style_this->widow_control > 1)
				widow_control_this = style_this->widow_control;

			}
		      keep_with_next_this = style_this->keep_with_next;
		    }
		  if (dont_split_this || orphan_control_this || widow_control_this)
		    {
		      if (!ud->outbuf3_enabled)
			{
			  saved_child = child;
			  saved_branchCount = branchCount - 1;
			  saveState ();
			  state_saved = 1;
			  ud->outbuf3_enabled = 1;
			  ud->lines_length = 0;
			}
		    }
		  if (dont_split_this)
		    dont_split = dont_split_this;
		  if (keep_with_next_this)
		    keep_with_next = keep_with_next_this;
		  if (keep_with_previous_this)
		    {
		      keep_with_previous = keep_with_previous_this;
		      keep_with_previous_pos = ud->lines_length;
		    }
		  if (orphan_control_this)
		    {
		      orphan_control = orphan_control_this;
		      orphan_control_pos = ud->lines_length;
		    }
		  if (widow_control_this)
		  {
			widow_control = widow_control_this;
			widow_control_pos = ud->lines_length;
		  }
		}
	    }
	  if (strcmp (child->name, "brl") != 0)
	    transcribe_paragraph (child, 1);
	  break;
	case XML_TEXT_NODE:
	  /*Is there already a <brl> node? */
	  if (!(ud->format_for == utd && child->next != NULL
		&& strcmp ((char *) child->next->name, "brl") == 0))
	    insert_text (child);
	  /*Is there now a <brl>node? */
	  if (ud->format_for == utd && child->next != NULL
	      && strcmp ((char *) child->next->name, "brl") == 0)
	    child = child->next;	/*skip <brl> node */
	  break;
	case XML_CDATA_SECTION_NODE:
	  transcribe_cdataSection (child);
	  break;
	default:
	  break;
	}
      if (child != NULL)
	child = child->next;
      if (ud->format_for == utd)
	{
	}
      else
	{
	  if (ud->outbuf3_enabled)
	    {
	      if (dont_split_status >= 0 &&
		  keep_with_previous_status >= 0
		  && orphan_control_status >= 0
		  && widow_control_status >=0)
		{
		  if (keep_with_previous)
		    {
		      if (keep_with_previous_this)
			keep_with_previous_status = 1;
		      else if (keep_with_previous_pos < ud->lines_length)
			keep_with_previous_status = 1;
		      else
			keep_with_previous_status = 0;
		      if (keep_with_previous_pos < ud->lines_length &&
			  ud->lines_pagenum[keep_with_previous_pos] >
			  ud->lines_pagenum[keep_with_previous_pos - 1] &&
			  !ud->lines_newpage[keep_with_previous_pos])
			keep_with_previous_status = -1;
		    }
		  if (dont_split_this)
		    {
		      dont_split_status = 1;
		      for (i = 1; i < ud->lines_length; i++)
			{
			  if (ud->lines_pagenum[i] > ud->lines_pagenum[i - 1])
			    {
			      if (!ud->lines_newpage[i])
				dont_split_status = -1;
			      break;
			    }
			}
		    }
		  if (orphan_control)
		    {
		      if (orphan_control_this)
			orphan_control_status = 1;
		      else
			orphan_control_status = 0;
		      i = 1;
		      while (i < orphan_control
			     && orphan_control_pos + i < ud->lines_length)
			{
			  if (ud->lines_pagenum[orphan_control_pos + i] >
			      ud->lines_pagenum[orphan_control_pos + i - 1])
			    {
			      if (!ud->lines_newpage[i])
				orphan_control_status = -1;
			      break;
			    }
			  i++;
			}
		      if (i == orphan_control)
			orphan_control_status = 1;
		    }
			
		  if (widow_control)
		    {
		      if (widow_control_this)
				widow_control_status = 1;
		      else
				widow_control_status = 0;
		      i = ud->lines_length-widow_control_pos-1;
		      while (i > ud->lines_length-widow_control && i > 0)
			  {
				if (ud->lines_pagenum[widow_control_pos + i] >
			      ud->lines_pagenum[widow_control_pos + i - 1])
			    {
			      if (!ud->lines_newpage[i])
					widow_control_status = -1;
			      break;
			    }
				i--;
			  }
		      if (i == ud->lines_length-widow_control)
				widow_control_status = 1;
		    }
		}
	      if (dont_split_status < 0 ||
		  keep_with_previous_status < 0 || orphan_control_status < 0 ||
		  widow_control_status < 0)
		{
		  if (state_saved)
		    {
		      dont_split = 0;
		      dont_split_status = 0;
		      keep_with_next = 0;
		      keep_with_previous = 0;
		      keep_with_previous_status = 0;
		      orphan_control = 0;
		      orphan_control_status = 0;
			  widow_control = 0;
			  widow_control_status = 0;	
		      restoreState ();
		      child = saved_child;
		      branchCount = saved_branchCount;
		      state_saved = 0;
		      ud->outbuf3_len_so_far = 0;
		      ud->outbuf3_enabled = 0;
		      do_newpage ();
		    }
		  else
		    break;
		}
	      if (dont_split_status > 0)
		{
		  dont_split = 0;
		  dont_split_status = 0;
		}
	      if (keep_with_previous_status > 0)
		{
		  keep_with_previous = 0;
		  keep_with_previous_status = 0;
		}
	      if (orphan_control_status > 0)
		{
		  orphan_control = 0;
		  orphan_control_status = 0;
		}
		if(widow_control_status > 0)
		{
			widow_control = 0;
			widow_control_status = 0;
		}
	    if ((!dont_split && !keep_with_previous &&
		   !keep_with_next && !orphan_control 
		   &&!widow_control) ||(!child && state_saved))
		{
		  write_buffer (3, 0);
		  ud->outbuf3_enabled = 0;
		  state_saved = 0;
		}
	    }
	  if (dont_split_this)
	    dont_split = 0;
	  if (keep_with_next_this)
	    keep_with_next = 0;
	  if (keep_with_previous_this)
	    keep_with_previous = 0;
	  if (orphan_control_this)
	    orphan_control = 0;
	  if(widow_control_this)
		widow_control = 0;
	}
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  if (haveMacro)
    end_macro ();
  else if (style)
    end_style ();
  else
    switch (ud->stack[ud->top])
      {
      case runninghead:
	insert_translation (ud->main_braille_table);
	set_runninghead_string (ud->translated_buffer, ud->translated_length);
	ud->translated_length = 0;
	break;
      case footer:
	insert_translation (ud->main_braille_table);
	set_footer_string (ud->translated_buffer, ud->translated_length);
	ud->translated_length = 0;
	break;
	case noteref:
		if(ud->endnotes)
			finish_endnote(node);
		break;
      default:
	break;
      }
  if (action != 0)
    pop_sem_stack ();
  else
    {
      insert_translation (ud->main_braille_table);
      write_paragraph (para, NULL);
    }
  logMessage(LOU_LOG_DEBUG, "Finished transcribe_paragraph");
  return 1;
}
