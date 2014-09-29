@echo off
pushd "%CD%" 
cd %~dp0
erase *.obj
erase brailleblasterlib\*.dll
erase liblouisutdml.dll
erase make_makefile.exe
erase Makefile.gen
popd
@echo on
