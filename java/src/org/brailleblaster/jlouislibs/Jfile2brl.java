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

