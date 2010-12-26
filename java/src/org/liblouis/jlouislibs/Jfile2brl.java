/* BrailleBlaster Braille Transcription Application
  *
  * Copyright (C) 2010, 2012
  * ViewPlus Technologies, Inc. www.viewplus.com
  * and
  * Abilitiessoft, Inc. www.abilitiessoft.com
  * All rights reserved
  *
  * This file may contain code borrowed from files produced by various
  * Java development teams. These are gratefully acknowledged.
  *
  * This file is free software; you can redistribute it and/or modify it
  * under the terms of the Apache 2.0 License, as given at
  * http://www.apache.org/licenses/
  *
  * This file is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
  * See the Apache 2.0 License for more details.
  *
  * You should have received a copy of the Apache 2.0 License along with
  * this program; see the file COPYING.
  * If not, see
  * http://www.apache.org/licenses/
  *
  * Maintained by John J. Boyer john.boyer@abilitiessoft.com
*/

package org.brailleblaster.jlouislibs;

import org.brailleblaster.jlouislibs.Jliblouisutdml;

public class Jfile2brl
{
public static void main (String[] args)
{
Jliblouisutdml bindings = new Jliblouisutdml ();
bindings.lbu_translateFile ("default.cfg", args[0], args[1], 
"logfile", 
"debug yes\n", 0);
bindings.lbu_free ();
}
}

