all: Makefile.gen
    nmake /nologo /f Makefile.gen
Makefile.gen: ..\liblouisutdml\Makefile.am ..\configure.ac make_makefile.exe
    make_makefile
make_makefile.exe: make_makefile.c
    cl /nologo /Ox make_makefile.c
