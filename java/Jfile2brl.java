package org.brailleblaster.jllouislibs;

//import org.brailleblaster.jlouislibs.*;

public class Jfile2brl
{
public static void main (String[] args)
{
Jliblouisutdml bindings = new Jliblouisutdml ();
bindings.lbu_translateFile ("default.cfg", args[0], args[1], "logfile", 
null, 0);
}
}

