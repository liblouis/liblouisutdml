/* libloluisutdml Braille Transcription Library

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

   Maintained by John J. Boyer john.boyer@jjb-software.com
   */

/* Semantic actions */
typedef enum
{
  /* General */
  no = 0,
  skip,
  generic,
  cdata,
  htmllink,
  htmltarget,
  changetable,
  /* Endnote position */
  noteref,
  reverse,
  configfile,
  configstring,
  configtweak,
  /* reserved styles */
  document,
  para,
  heading1,
  heading2,
  heading3,
  heading4,
  heading5,
  heading6,
  heading7,
  heading8,
  heading9,
  heading10,
  contentsheader,
  contents1,
  contents2,
  contents3,
  contents4,
  contents5,
  contents6,
  contents7,
  contents8,
  contents9,
  contents10,
  /* Endnotes */
  note,
  notesheader,
  notesdescription,
  /* translation */
  notranslate,
  compbrl,
  uncontracted,
  contracted,
  /* General text */
  pagenum,
  /* Use next 2 for anything. */
  genpurp1,
  genpurp2,
  pagebreak,
  attrtotext,
  runninghead,
  footer,
  italicx,
  boldx,
  underlinex,
  linespacing,
  blankline,
  softreturn,
  newpage,
  righthandpage,
  code,
  music,
  /* MathML */
  math,
  mi,
  mn,
  mo,
  mtext,
  mspace,
  ms,
  mglyph,
  mrow,
  mfrac,
  msqrt,
  mroot,
  mstyle,
  merror,
  mpadded,
  mphantom,
  mfenced,
  menclose,
  msub,
  msup,
  msubsup,
  munder,
  mover,
  munderover,
  mmultiscripts,
  none,
  semantics,
  mprescripts,
  mtable,
  mtr,
  mtd,
  maligngroup,
  malignmark,
  mlabeledtr,
  maction,
  /* Other technical notations */
  chemistry,
  graphic,
  /* Needed by UTD */
  markhead,
  utdmeta,
  utdbrl,
  utdnewpage,
  utdnewline,
  utdbrlonly,
  utddispimg,
  utdinlnimg,
  /* End marker */
  end_all
}
sem_act;
